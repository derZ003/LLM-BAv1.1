from genericpath import exists
import os
import re
import subprocess
import sys

from pathlib import Path

from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.prompts import PromptTemplate

from LibCCode import *

load_dotenv()
MORPHEUS_API_KEY = os.environ["MORPHEUS_API_KEY"]
BASE_PATH = "~/_Uni/BA"
CUR_DIR = Path(BASE_PATH).expanduser() / "cur_Files"

#Morpheus LLM API setup
llm = ChatOpenAI(
    model= "google/gemma-4-31B-it",
    base_url= "https://morpheus.cit.tum.de/api/v1",
    api_key= MORPHEUS_API_KEY
)
#Output rules shared by the summary prompts
SUMMARY_OUTPUT_RULES = """
    #Output rules:
    - Output exactly ONE C code block containing only the final version; no drafts or alternatives.
    - Define {func_name} exactly once. Helper functions are allowed but must be static and have different names.
"""
#Rules/functions of the symbolic reflection API, shared by the summary prompts
SYMBOLIC_API_RULES = """
    #Rules of the API:
    ## Symbolic Reflection API

    You write a symbolic summary: a C function with the same signature as the
    libc function it models. The summary is executed by a symbolic execution
    engine. Its inputs may hold symbolic values. The functions below let the code
    inspect and manipulate the symbolic state.

    ### Execution model
    - Ordinary C arithmetic on symbolic values is allowed. It yields a symbolic
    expression (e.g. `n = len + 1`).
    - A C `if`/`while` whose condition depends on a symbolic value forks the
    execution into several paths. Decide branches with `is_certain` / `is_sat`
    and merge results with `_ITE_VAR_` instead.
    - Constraints (`cnstr_t`) are opaque handles, not booleans. Never combine or
    test them with `!`, `&&`, `||`, `==` or `if (c)`. Use the constructors below,
    and query them with `is_certain` / `is_sat`.
    - Sizes of symbolic variables are given in bits (char = 8, int = 32, 32-bit
    target).

    ### Types
    `symbolic` (a symbolic value), `cnstr_t` (constraint handle), `list_t` (a
    symbolic byte list), `size_t` = `unsigned int`, `ssize_t` = `int`.
    The API functions need no `#include`. The summary file is compiled together
    with the API.

    ### 1. Symbolic values
    | Function | Meaning |
    |---|---|
    | `symbolic sym_var(size_t bits)` | fresh unconstrained symbolic value |
    | `symbolic sym_var_named(char *name, size_t bits)` | same, with a given name |
    | `symbolic sym_var_array(char *name, size_t index, size_t bits)` | fresh value for cell `name[index]` |
    | `int is_symbolic(symbolic v)` | 1 if `v` is symbolic, else 0 |
    | `long concretize(symbolic v)` | one concrete value `v` may take (adds nothing to the path condition) |
    | `long maximize(symbolic v)` | largest value `v` may take |
    | `long minimize(symbolic v)` | smallest value `v` may take |

    ### 2. Path condition and solver
    | Function | Meaning |
    |---|---|
    | `int is_certain(cnstr_t c)` | 1 if the path condition implies `c` (true on every path) |
    | `int is_sat(cnstr_t c)` | 1 if `c` can hold together with the path condition |
    | `void assume(cnstr_t c)` | adds `c` to the path condition (drops the executions where `c` is false) |
    | `void assert(cnstr_t c)` | fails if the path condition does not imply `c` |
    | `void push_pc(void)` | saves the current path condition |
    | `void pop_pc(void)` | restores the last saved path condition |
    | `void report_error(const char *file, unsigned int line, const char *msg)` | reports an error, does not return |

    ### 3. Constraint constructors (all return `cnstr_t`)
    - Logic: `_NOT_(c)`, `_AND_(c1, c2)`, `_OR_(c1, c2)`
    - Signed comparison: `_EQ_(a, b)`, `_NEQ_(a, b)`, `_LT_`, `_LE_`, `_GT_`, `_GE_`
    - Unsigned comparison: `_ULT_`, `_ULE_`, `_UGT_`, `_UGE_`
    - If-then-else over constraints: `_ITE_(cond, c1, c2)`
    - If-then-else over values: `_ITE_VAR_(cond, v1, v2)` returns the symbolic
    value `cond ? v1 : v2`
    - Constants: `TRUE` (always holds), `FALSE` (never holds). These are the only
    predefined constraints; do not invent others.

    ### 4. Memory
    | Function | Meaning |
    |---|---|
    | `void *mem_alloc(size_t nbytes)` | allocates a heap block (a symbolic size is maximized) |
    | `void mem_free(void *p)` | frees a block returned by `mem_alloc` |
    | `size_t n_allocd(void *p)` | size of a block returned by `mem_alloc` |
    | `void allocd(void *p, size_t n)` | fails unless bytes `p .. p+n` are readable and writable |
    | `void cond_write(void *p, symbolic v, cnstr_t pc)` | writes `pc ? v : old` to `*p`; a symbolic `p` writes to every address it may denote |

    ### 5. Symbolic lists (for memory segments of symbolic length)
    `list_t lst_mk(void)` (empty), `list_t lst_cons(symbolic v, list_t l)`,
    `symbolic lst_hd(list_t l)`, `list_t lst_tl(list_t l)`,
    `cnstr_t lst_empty(list_t l)`, `size_t lst_len(list_t l)`,
    `list_t lst_nbytes(char c, size_t n)`, `list_t lst_zeros(size_t n)`

    ### Usage patterns
    **Case split (exact).** Checks `cond` without forking and merges both branches
    into one path:
    ```c
    int r;
    if (is_certain(cond))            {{ r = A; }}
    else if (is_certain(_NOT_(cond))) {{ r = B; }}
    else {{
    push_pc(); assume(cond);        r = A; int a = r; pop_pc();
    push_pc(); assume(_NOT_(cond)); r = B; int b = r; pop_pc();
    r = _ITE_VAR_(cond, a, b);
    }}
    ```

    **Recursion over a string.** Recurse on `s + 1` until
    `is_certain(_EQ_(*s, '\0'))`. The inputs are bounded, so this terminates.

    **Unconditional write.** Use a plain assignment `*p = v;` or
    `cond_write(p, v, TRUE)`. Use `cond_write` with another constraint only when
    the write depends on a symbolic condition.

    **How primitives affect the approximation**
    - Exact: every case is kept (case split + `_ITE_VAR_`).
    - Under-approximation: `assume` keeps only some executions. Examples: assume
    one branch only, or concretize with `v = maximize(x); assume(_EQ_(x, v));`.
    - Over-approximation: return a fresh `sym_var(bits)`, possibly constrained by
    `assume` to a range that contains every real result.
"""
#Prompt for generating symbolic summaries
exact_sum_gen_prompt = PromptTemplate(
    input_variables=["func_name", "libc_code"],
    template = """#Task: You are a senior software engineer and need to write a summary of the follwoing C library function {func_name}
    for symbolic testing, the summary must conform to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
""" + SUMMARY_OUTPUT_RULES + SYMBOLIC_API_RULES + """
    #Library function code:
    ```c
    {libc_code}
    ```
    """
)
#Prompt for generating concrete testfile for SummboundVerify
concrete_gen_prompt = PromptTemplate(
    input_variables=["func_name", "libc_code"],
    template="""#Task: You are a senior software engineer. Rewrite the following musl libc function {func_name}
    as a self-contained C file that behaves exactly the same.
    Your answer must not include any other text or explanation, only the C code block.
    #Rules:
    - Rename the function to concrete_{func_name}; keep parameter and return types otherwise identical.
    - No #include, #define, #ifdef or other preprocessor directives.
    - Do not define size_t, ssize_t or NULL (they are already provided). Do not use uintptr_t, use unsigned long.
    - No GNU extensions (__attribute__, typeof, statement expressions, inline asm, builtins).
    - Use plain byte-wise C only (no word-at-a-time/alignment tricks). The file is parsed by pycparser.
    - If the function calls other libc functions, implement them in the same file as static helpers.

    #Library function code:
    ```c
    {libc_code}
    ```
"""
)
#Prompt for revision of symbolic summaries based on SummBoundVerify counterexamples
revision_gen_prompt = PromptTemplate(
    input_variables=["func_name", "libc_code", "summary", "counterexamples"],
    template="""#Task: You are a senior software engineer. The following symbolic summary of the C library function {func_name}
    was validated against the library code and produced counterexamples: inputs on which the summary and the
    library function behave differently. Revise the summary so that it fixes these counterexamples and conforms to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
""" + SUMMARY_OUTPUT_RULES + SYMBOLIC_API_RULES + """
    #Library function code:
    ```c
    {libc_code}
    ```

    #Current symbolic summary:
    ```c
    {summary}
    ```

    #Counterexamples (SummBoundVerify test results):
    ```json
    {counterexamples}
    ```
"""
)

#extract C code from LLM response
def extract_c_code(text: str) -> str:
    code = re.findall(r"```(?:c|C)?[ \t]*\n(.*?)```", text, re.S)
    if not code:
        raise ValueError("no C code found in LLM response")
    return "\n".join(code).strip() + "\n"

def gen_symbolic_summary(func_name:str, libc_code:str) -> str: 
    chain_gensum = exact_sum_gen_prompt | llm
    response = chain_gensum.invoke({"func_name": func_name, "libc_code": libc_code})
    summary = extract_c_code(response.content)
    print("Generated symbolic summary:\n" + summary)
    return summary

def gen_revision_summary(func_name:str, libc_code:str, summary:str, counterexamples:str) -> str:
    chain_revision = revision_gen_prompt | llm
    response = chain_revision.invoke({"func_name": func_name, "libc_code": libc_code, "summary": summary, "counterexamples": counterexamples})
    summary = extract_c_code(response.content)
    print("Generated revised-symbolic summary:\n" + summary)
    return summary

def gen_concrete_code(func_name:str, libc_code:str) -> str:
    chain_genconcrete = concrete_gen_prompt | llm
    response = chain_genconcrete.invoke({"func_name": func_name, "libc_code": libc_code})
    concrete_code = extract_c_code(response.content)
    concrete_code = re.sub(r"^\s*#\s*include.*\n", "", concrete_code, flags=re.M)
    print("Generated concrete code:\n" + concrete_code)
    return concrete_code

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

def gen_test(concrete_path:Path, summ_path:Path, test_path:Path, func_name:str) -> None:
    testgen = subprocess.run(
        [str(Path(sys.executable).parent / "summbv"),
        "-func", str(concrete_path), "--funcname", f"concrete_{func_name}",
        "-summ", str(summ_path), "--summname", func_name,
        "-o", str(test_path), "--compile", "x86", "--lib", str(Path(__file__).parent / "lib.c"),
        "--maxvalue", "5"],
        capture_output=True,
        text=True
    )
    print("stdout: \n" + testgen.stdout)
    print("stderr: \n" + testgen.stderr)
    return

def run_test() -> None:
    #executing Testfile
    subprocess.run(
        [str(Path(sys.executable).parent / "summbv"), "-run", "--binary", 
        str(CUR_DIR / f"{func_name}_validation.test"), "--results", 
        str(CUR_DIR), "-ascii"],
        capture_output=True,
        text=True
    )
    return

def exact_pipeline(func_name: str) -> None:
    func_code = get_musl_code(func_name)
    #generating symbolic summary with LLM
    summary = gen_symbolic_summary(func_name, func_code)

    #generate concrete.c (header-free) for sbv call
    concrete_code = gen_concrete_code(func_name, func_code)

    #write concrete and summary to cur_Files folder
    write_concrete_cur_Files(concrete_code)
    write_summary_cur_Files(summary)

    #generating + compiling Testfile with SummBoundVerify -> cur_Files/<func>_validation.c / .test
    test_path = CUR_DIR / f"{func_name}_validation.c"
    concrete_path = CUR_DIR / "cur_concrete.c"
    summ_path = CUR_DIR / "cur_summary.c"

    gen_test(concrete_path, summ_path, test_path, func_name)
    run_test()

    test_results_path = CUR_DIR / f"{func_name}_validation.test_result.json"
    #skipp regeneration of symbolic summary if counterexamples is empty
    if test_results_path.exists():
        test_results = test_results_path.read_text()
        print("Test results:\n" + test_results)
        i = 0
        while '"counterexamples": {}' not in test_results and i < 3:
            print("counterexamples found, regenerating symbolic summary...")
            summary = gen_revision_summary(func_name, func_code, summary, test_results)
            write_summary_cur_Files(summary)
            #retesting revised summary
            gen_test(concrete_path, summ_path, test_path, func_name)
            run_test()
            test_results = test_results_path.read_text()
            print("Updated test results:\n" + test_results)
            i += 1
    else: 
        print("No test results found, skipping symbolic summary regeneration.")
    #return summary and counterexamples
    write_summary_cur_Files(summary)
    print("Final symbolic summary:\n" + summary)
    if test_results_path.exists():
        test_results = test_results_path.read_text()
        print("Final test results:\n" + test_results)
    else:
        print("No test results found and test failed, try again ;)")
    return

#TODO: add overapprox pipeline

if __name__ == "__main__":
    #fetching glibc code
    #func_name = sys.argv[1]
    func_name = "memcpy"
    exact_pipeline(func_name)


