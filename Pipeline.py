import json
import re
import subprocess
import sys

from pathlib import Path

from pycparser import c_ast, c_generator, c_parser

from LibCCode import get_uclibc_code
from ConcreteCode import gen_concrete_code, get_concrete_code, PARSE_PRELUDE
from SummaryGeneration import gen_symbolic_summary, gen_revision_summary

BASE_PATH = "~/_Uni/BAv1.1"
CUR_DIR = Path(BASE_PATH).expanduser() / "cur_Files"
TEST_TIMEOUT = 120
TIMEOUT_MSG = (f"The symbolic execution of the summary did not terminate within {TEST_TIMEOUT} seconds")
COMPILE_ERROR_MSG = "The summary does not compile. Compiler errors:\n"
RUNTIME_ERROR_MSG = "The symbolic execution of the summary aborted with an error:\n"
RUNTIME_ERROR_HINTS = {
    "UnsatConstraintError": "An `assume(c)` was called with a condition `c` that is impossible on the current path. "
                            "Only assume a condition in the undecided branch of its own case split.",
    "MemoryPermissionsError": "The summary accessed memory outside the input buffers (e.g. read past a possibly reached end).",
    "DuplicateSymbolicVariableError": "`sym_var_named` was called twice with the same name; names must be unique.",
}
#accepted result types per approximation mode, exact also accepted for over- and under-approximation
ACCEPTED_RESULTS = {
    "exact": {"exact"},
    "over": {"over-approximation", "exact"},
    "under": {"under-approximation", "exact"},
}
#extra arguments to ensure SBV terminates
SBV_EXTRA_ARGS = {
    "rawmemchr": ["--defaultvalues", "{2:0}"],
}

#Checks every result inside .json is correct/acceptable -> result-types: "exact", "under-approximation", "over-approximation" or "bug"
def results_accepted(test_results: str, accepted_results: set[str]) -> bool:
    if test_results == TIMEOUT_MSG or test_results.startswith((COMPILE_ERROR_MSG, RUNTIME_ERROR_MSG)):
        return False
    tests = json.loads(test_results)
    return len(tests) > 0 and all(t["result"] in accepted_results for t in tests.values())
#categrorizes test results
def result_category(test_results: str | None) -> str:
    if test_results is None:
        return "bug"
    for category, accepted_results in ACCEPTED_RESULTS.items():
        if results_accepted(test_results, accepted_results):
            return category
    return "bug"
#sorts and moves cur_Files
def move_cur_Files(func_name: str, test_results: str | None) -> None:
    target_dir = CUR_DIR / result_category(test_results) / f"{func_name}-tests"
    target_dir.mkdir(parents=True, exist_ok=True)
    file_names = ["cur_concrete.c", "cur_summary.c", f"{func_name}_validation.c",
                f"{func_name}_validation.test", f"{func_name}_validation.test_result.json"]
    for name in file_names:
        path = CUR_DIR / name
        if path.exists():
            path.replace(target_dir / name)
    return

def write_concrete_cur_Files(concrete_code:str) -> None:
    concrete_path = CUR_DIR / "cur_concrete.c"
    concrete_path.write_text(concrete_code)
    return

def write_summary_cur_Files(summary:str) -> None:
    summ_path = CUR_DIR / "cur_summary.c"
    #TRUE/FALSE constraint constants promised to the LLM in SYMBOLIC_API_RULES
    header = "#ifndef TRUE\n#define TRUE 1\n#endif\n#ifndef FALSE\n#define FALSE 0\n#endif\n"
    summ_path.write_text(header + summary)
    return

#SBV can't test void functions -> wrap them in int functions, compared via --memory
def void_wrappers(concrete_code: str, func_name: str) -> tuple[str, str] | None:
    ast = c_parser.CParser().parse(PARSE_PRELUDE + concrete_code)
    fdecl = next(n.decl.type for n in ast.ext
                if isinstance(n, c_ast.FuncDef) and n.decl.name == f"concrete_{func_name}")
    ret = fdecl.type
    if not (isinstance(ret, c_ast.TypeDecl) and ret.type.names == ["void"]):
        return None
    params = [p for p in fdecl.args.params if not isinstance(p, c_ast.Typename)] if fdecl.args else []
    gen = c_generator.CGenerator()
    param_str = ", ".join(gen.visit(p) for p in params) or "void"
    arg_str = ", ".join(p.name for p in params)
    wrap = lambda name: f"\nint {name}_w({param_str})\n{{\n  {name}({arg_str});\n  return 0;\n}}\n"
    return wrap(f"concrete_{func_name}"), wrap(func_name)

#returns the gcc errors (with source line) if the validation test does not compile, else None
def gen_test(concrete_path:Path, summ_path:Path, test_path:Path, func_name:str, wrapped:bool) -> str | None:
    print("Generating validation test...")
    test_path.with_suffix(".test").unlink(missing_ok=True)
    suffix = "_w" if wrapped else ""
    testgen = subprocess.run(
        [str(Path(sys.executable).parent / "summbv"),
        "-func", str(concrete_path), "--funcname", f"concrete_{func_name}{suffix}",
        "-summ", str(summ_path), "--summname", f"{func_name}{suffix}",
        "-o", str(test_path), "--compile", "x86", "--lib", str(Path(__file__).parent / "lib.c"),
        "--maxvalue", "5", "-memory", *SBV_EXTRA_ARGS.get(func_name, [])],
        capture_output=True,
        text=True
    )
    print("stdout: \n" + testgen.stdout)
    print("stderr: \n" + testgen.stderr)
    if testgen.returncode == 0 and test_path.with_suffix(".test").exists():
        return None
    lines = (testgen.stdout + testgen.stderr).splitlines()
    errors = []
    for i, line in enumerate(lines):
        if ": error:" in line:
            errors += [line.split("/")[-1]] + lines[i + 1:i + 3]
    return COMPILE_ERROR_MSG + "\n".join(dict.fromkeys(errors) if errors else lines[-5:])

def run_test(func_name: str) -> str | None:
    print("Running validation test...")
    test_results_path = CUR_DIR / f"{func_name}_validation.test_result.json"
    #remove old results
    test_results_path.unlink(missing_ok=True)
    #executing Testfile
    try:
        testrun = subprocess.run(
            [str(Path(sys.executable).parent / "summbv"), "-run", "--binary", 
            str(CUR_DIR / f"{func_name}_validation.test"), "--results", 
            str(CUR_DIR), "-ascii", "-timeout", str(TEST_TIMEOUT)],
            capture_output=True,
            text=True,
            timeout=TEST_TIMEOUT + 30
        )
        timed_out = "TimeoutError" in testrun.stdout
        output = testrun.stdout + testrun.stderr
    except subprocess.TimeoutExpired:
        timed_out = True
        output = ""
    if test_results_path.exists():
        return test_results_path.read_text()
    if timed_out:
        print("Testing Timeout.")
        return TIMEOUT_MSG
    #runtime errors of SBV (e.g. UnsatConstraintError) are passed to the revision like compile errors
    errors = re.findall(r"summboundverify\.exceptions\.exceptions\.(\w+): (.*)", output)
    if errors:
        name, msg = errors[-1]
        print(f"Testing aborted: {name}")
        return RUNTIME_ERROR_MSG + f"{name}: {msg}\n" + RUNTIME_ERROR_HINTS.get(name, "")
    return None

def insert_parameters() -> tuple[str, str, set[str]]:
    #func_name = sys.argv[1]
    #approx_mode = sys.argv[2]
    func_name = "strlcpy"
    approx_mode = "exact"
    accepted_results = ACCEPTED_RESULTS[approx_mode]
    print(f"Generating symbolic summary for {func_name} with {approx_mode}-approximation...")
    return func_name, approx_mode, accepted_results

def pipeline(func_name: str, approx_mode: str, accepted_results: set[str]) -> None:
    func_code = get_uclibc_code(func_name)

    #generate concrete.c (header-free) for sbv call
    concrete_code = get_concrete_code(func_name)

    #generating symbolic summary with LLM
    summary = gen_symbolic_summary(approx_mode, func_name, func_code)

    wrappers = void_wrappers(concrete_code, func_name)
    concrete_wrap, summ_wrap = wrappers or ("", "")

    #write concrete and summary to cur_Files folder
    write_concrete_cur_Files(concrete_code + concrete_wrap)
    write_summary_cur_Files(summary + summ_wrap)

    #generating + compiling Testfile with SummBoundVerify -> cur_Files/<func>_validation.c / .test
    test_path = CUR_DIR / f"{func_name}_validation.c"
    concrete_path = CUR_DIR / "cur_concrete.c"
    summ_path = CUR_DIR / "cur_summary.c"

    test_results = gen_test(concrete_path, summ_path, test_path, func_name, wrappers is not None) or run_test(func_name)

    #skip regeneration of symbolic summary if the result is accepted for this approximation mode
    if test_results is not None:
        print("Test results:\n" + test_results)
        i = 0
        while not results_accepted(test_results, accepted_results) and i < 3:
            print("result not accepted, regenerating symbolic summary...")
            summary = gen_revision_summary(approx_mode, func_name, func_code, summary, test_results)
            write_summary_cur_Files(summary + summ_wrap)
            #retesting revised summary
            test_results = gen_test(concrete_path, summ_path, test_path, func_name, wrappers is not None) or run_test(func_name)
            if test_results is None:
                print("No test results exist after revision, stopping.")
                break
            print("Updated test results:\n" + test_results)
            i += 1
    else: 
        print("No test results found, skipping symbolic summary regeneration.")
    #return summary and counterexamples
    write_summary_cur_Files(summary + summ_wrap)
    print("Final symbolic summary:\n" + summary)
    if test_results is not None:
        print("Final test results:\n" + test_results)
    else:
        print("No test results found and test failed, try again ;)")
    move_cur_Files(func_name, test_results)
    print("Finished")
    return

if __name__ == "__main__":
    func_name, approx_mode, accepted_results = insert_parameters()
    pipeline(func_name, approx_mode, accepted_results)

