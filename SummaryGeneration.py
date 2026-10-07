import os

from pathlib import Path

from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.prompts import PromptTemplate

from Helpers import extract_c_code

load_dotenv()
MORPHEUS_API_KEY = os.environ["MORPHEUS_API_KEY"]
BASE_PATH = "~/_Uni/BAv1.1"
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
    symbolic r;
    if (is_certain(cond))            {{ r = A; }}
    else if (is_certain(_NOT_(cond))) {{ r = B; }}
    else {{
    push_pc(); assume(cond);        symbolic a = A; pop_pc();
    push_pc(); assume(_NOT_(cond)); symbolic b = B; pop_pc();
    r = _ITE_VAR_(cond, a, b);
    }}
    ```

    **Unconditional write.** Use a plain assignment `*p = v;` or
    `cond_write(p, v, TRUE)`. Use `cond_write` with another constraint only when
    the write depends on a symbolic condition.

    **How primitives affect the approximation**
    - Exact: every case is kept (case split + `_ITE_VAR_`).
    - Under-approximation: `assume` keeps only some executions. Examples: assume
    one branch only, or concretize with `v = maximize(x); assume(_EQ_(x, v));`.
    - Over-approximation: return a fresh `sym_var(bits)`, possibly constrained by
    `assume` to a range that contains every real result.

    **Every symbolic condition has three cases.** For each condition `c` that
    depends on a symbolic value, handle: certainly true (`is_certain(c)`),
    certainly false (`is_certain(_NOT_(c))`), and undecided (case split).
    A two-way `if (is_certain(c)) A else B` is wrong in an exact summary: when `c`
    is only possible, it silently executes B for inputs where A is correct.
    This applies to nested conditions too: after `assume(c1)`, a second
    condition `c2` is usually still undecided and needs its own case split.

    **Recursion over a string.** At each byte, the end-of-string condition
    `_EQ_(*s, '\\0')` is a symbolic condition like any other. Use the case split:
    return the terminating result if `*s` is `'\\0'`, recurse on `s + 1` otherwise,
    and merge both with `_ITE_VAR_`. Only recurse inside
    `assume(_NOT_(_EQ_(*s, '\\0')))`, never past a possible terminator.
    The inputs are bounded, so this terminates.

    **Return value.** The summary must return exactly what the library code
    returns; check its `return` statement. The recursion advances its pointer
    arguments, so the pointer reached at the base case is NOT the original
    argument. If the library returns an unchanged parameter (e.g. `dest`/`s1`
    in strcpy, strcat, memcpy, memset), let the recursive helper only do the
    work (return `void`) and return the saved original parameter from
    {func_name} itself. Only merge results with `_ITE_VAR_` when the return
    value really depends on where the recursion stops (e.g. strlen, strchr).

    **Recursion bounded by a length.** A size parameter (`n`, `len`, `count`, ...)
    is usually symbolic. `is_certain(_EQ_(n, 0))` is then never true, so a
    recursion that only stops on it never terminates. Treat `_EQ_(n, 0)` like
    any other symbolic condition and case split on it at every step:
    ```c
    cnstr_t n_zero = _EQ_(n, 0);
    if (is_certain(n_zero)) {{ return END; }}
    if (is_certain(_NOT_(n_zero))) {{ return STEP(s, n); }}
    push_pc(); assume(_NOT_(n_zero)); symbolic r = STEP(s, n); pop_pc();
    return _ITE_VAR_(n_zero, END, r);
    ```
    Only recurse with `n - 1` inside `assume(_NOT_(n_zero))`. Then `n` shrinks
    on every path and becomes certainly 0 after at most the input bound.
    Never read `s[i]` past a possibly reached bound (`n` or `'\\0'`).

    **Termination check.** Before answering, check every recursion and loop:
    on each path it must reach a base case whose condition becomes *certain*
    (via `assume` on that path). A base case that is only ever tested with
    `is_certain`, without an `assume` that makes it certain, does not terminate.

"""

#Prompt for generating exact symbolic summaries
EXACT_SUM_GEN_STRING = """#Task: You are a senior software engineer and need to write an exact summary of the follwoing C library function {func_name}
    for symbolic testing, the summary must conform to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""
#Prompt for generating over-approximate symbolic summaries
OVER_SUM_GEN_STRING="""#Task: You are a senior software engineer and need to write an over-approximate summary of the follwoing C library function {func_name}
    for symbolic testing, the summary must conform to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""
#Prompt for generating under-approximate symbolic summaries
UNDER_SUM_GEN_STRING = """#Task: You are a senior software engineer and need to write an under-approximate summary of the follwoing C library function {func_name}
    for symbolic testing, the summary must conform to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""

#Prompt for revision of symbolic summaries based on SummBoundVerify counterexamples
EXACT_REV_GEN_STRING = """#Task: You are a senior software engineer. The following symbolic summary of the C library function {func_name}
    was validated against the library code and produced counterexamples: inputs on which the summary and the
    library function behave differently. Revise the summary so that it fixes these counterexamples and conforms to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""
#Prompt for revision of symbolic summaries based on SummBoundVerify counterexamples
OVER_REV_GEN_STRING = """#Task: You are a senior software engineer. The following over-approximate symbolic summary of the C library function {func_name}
    was validated against the library code and produced counterexamples: inputs on which the library function shows a behavior
    that the summary does not cover. An over-approximation must include every behavior of the library function; additional behaviors are allowed.
    Revise the summary so that it covers these counterexamples, stays an over-approximation and conforms to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""
#Prompt for revision of symbolic summaries based on SummBoundVerify counterexamples
UNDER_REV_GEN_STRING = """#Task: You are a senior software engineer. The following under-approximate symbolic summary of the C library function {func_name}
    was validated against the library code and produced counterexamples: inputs on which the summary shows a behavior
    that the library function does not have. An under-approximation may only include behaviors of the library function; omitting behaviors is allowed.
    Revise the summary so that it excludes these counterexamples, stays an under-approximation and conforms to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
"""

GENERAL_REV_GEN_STRING = """
    #How to read the counterexamples:
    - "over-approximation": inputs where the library function returns "ret",
    but the summary cannot return it -> the summary is missing a behavior.
    - "under-approximation": inputs where the summary returns "ret",
    but the library function never does -> the summary has a wrong behavior.
    - "Not in model": the byte is irrelevant for this counterexample.
    - For pointer return types, "ret" is an address. If the two counterexamples
    have "ret" values that differ by a small offset, the summary returns a
    pointer advanced by the recursion instead of the pointer the library
    returns -> compare with the library's `return` statement.
    Trace the summary on these concrete inputs, find the branch that produces the
    wrong result, and fix that branch. Usually it is a missing case split.

"""
def gen_symbolic_summary(approx_mode: str, func_name:str, libc_code:str) -> str: 
    sum_prompt = {
            (approx_mode == "exact"): EXACT_SUM_GEN_STRING,
            (approx_mode == "over"): OVER_SUM_GEN_STRING,
            (approx_mode == "under"): UNDER_SUM_GEN_STRING,
        }[True]
    gen_prompt = PromptTemplate(
        input_variables=["func_name", "libc_code"],
        template=sum_prompt + SUMMARY_OUTPUT_RULES + SYMBOLIC_API_RULES + """
        #Library function code:
        ```c
        {libc_code}
        ```
        """
        )
    chain_gensum = gen_prompt | llm
    response = chain_gensum.invoke({"func_name": func_name, "libc_code": libc_code})
    summary = extract_c_code(response.content)
    print("Generated symbolic summary:\n" + summary)
    return summary

def gen_revision_summary(approx_mode: str, func_name:str, libc_code:str, summary:str, counterexamples:str) -> str:
    rev_prompt = {
            (approx_mode == "exact"): EXACT_REV_GEN_STRING,
            (approx_mode == "over"): OVER_REV_GEN_STRING,
            (approx_mode == "under"): UNDER_REV_GEN_STRING,
        }[True]
    gen_prompt = PromptTemplate(
        input_variables=["func_name", "libc_code", "summary", "counterexamples"],
        template=rev_prompt + GENERAL_REV_GEN_STRING + SUMMARY_OUTPUT_RULES + SYMBOLIC_API_RULES + """
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
    chain_revision = gen_prompt | llm
    response = chain_revision.invoke({"func_name": func_name, "libc_code": libc_code, "summary": summary, "counterexamples": counterexamples})
    summary = extract_c_code(response.content)
    print("Generated revised-symbolic summary:\n" + summary)
    return summary
