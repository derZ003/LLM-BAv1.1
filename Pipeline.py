import os
import re
import sys

from pathlib import Path

from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.prompts import PromptTemplate

load_dotenv()
MORPHEUS_API_KEY = os.environ["MORPHEUS_API_KEY"]

llm = ChatOpenAI(
    model= "google/gemma-4-31B-it",
    base_url= "https://morpheus.cit.tum.de/api/v1",
    api_key= MORPHEUS_API_KEY
)
sum_gen_prompt = PromptTemplate(
    input_variables=["func_name", "libc_code"],
    template = """#Task: You are a senior software engineer and need to write a summary of the follwoing C library function {func_name}
    for symbolic testing, the summary must conform to the API guidelines.
    Your answer must not include any other text or explanation, only the C code block.
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
    execution into several paths. Decide branches with `__is_certain` / `__is_sat`
    and merge results with `_ITE_VAR_` instead.
    - Constraints (`cnstr_t`) are opaque handles, not booleans. Never combine or
    test them with `!`, `&&`, `||`, `==` or `if (c)`. Use the constructors below,
    and query them with `__is_certain` / `__is_sat`.
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
    | `symbolic __sym_var(size_t bits)` | fresh unconstrained symbolic value |
    | `symbolic __sym_var_named(char *name, size_t bits)` | same, with a given name |
    | `symbolic __sym_var_array(char *name, size_t index, size_t bits)` | fresh value for cell `name[index]` |
    | `int __is_symbolic(symbolic v)` | 1 if `v` is symbolic, else 0 |
    | `long __concretize(symbolic v)` | one concrete value `v` may take (adds nothing to the path condition) |
    | `long __maximize(symbolic v)` | largest value `v` may take |
    | `long __minimize(symbolic v)` | smallest value `v` may take |

    ### 2. Path condition and solver
    | Function | Meaning |
    |---|---|
    | `int __is_certain(cnstr_t c)` | 1 if the path condition implies `c` (true on every path) |
    | `int __is_sat(cnstr_t c)` | 1 if `c` can hold together with the path condition |
    | `void __assume(cnstr_t c)` | adds `c` to the path condition (drops the executions where `c` is false) |
    | `void __assert(cnstr_t c)` | fails if the path condition does not imply `c` |
    | `void __push_pc(void)` | saves the current path condition |
    | `void __pop_pc(void)` | restores the last saved path condition |
    | `void __report_error(const char *file, unsigned int line, const char *msg)` | reports an error, does not return |

    ### 3. Constraint constructors (all return `cnstr_t`)
    - Logic: `_NOT_(c)`, `_AND_(c1, c2)`, `_OR_(c1, c2)`
    - Signed comparison: `_EQ_(a, b)`, `_NEQ_(a, b)`, `_LT_`, `_LE_`, `_GT_`, `_GE_`
    - Unsigned comparison: `_ULT_`, `_ULE_`, `_UGT_`, `_UGE_`
    - If-then-else over constraints: `_ITE_(cond, c1, c2)`
    - If-then-else over values: `_ITE_VAR_(cond, v1, v2)` returns the symbolic
    value `cond ? v1 : v2`

    ### 4. Memory
    | Function | Meaning |
    |---|---|
    | `void *__mem_alloc(size_t nbytes)` | allocates a heap block (a symbolic size is maximized) |
    | `void __mem_free(void *p)` | frees a block returned by `__mem_alloc` |
    | `size_t __n_allocd(void *p)` | size of a block returned by `__mem_alloc` |
    | `void __allocd(void *p, size_t n)` | fails unless bytes `p .. p+n` are readable and writable |
    | `void __cond_write(void *p, symbolic v, cnstr_t pc)` | writes `pc ? v : old` to `*p`; a symbolic `p` writes to every address it may denote |

    ### 5. Symbolic lists (for memory segments of symbolic length)
    `list_t __lst_mk(void)` (empty), `list_t __lst_cons(symbolic v, list_t l)`,
    `symbolic __lst_hd(list_t l)`, `list_t __lst_tl(list_t l)`,
    `cnstr_t __lst_empty(list_t l)`, `size_t __lst_len(list_t l)`,
    `list_t __lst_nbytes(char c, size_t n)`, `list_t __lst_zeros(size_t n)`

    ### Usage patterns
    **Case split (exact).** Checks `cond` without forking and merges both branches
    into one path:
    ```c
    int r;
    if (__is_certain(cond))            {{ r = A; }}
    else if (__is_certain(_NOT_(cond))) {{ r = B; }}
    else {{
    __push_pc(); __assume(cond);        r = A; int a = r; __pop_pc();
    __push_pc(); __assume(_NOT_(cond)); r = B; int b = r; __pop_pc();
    r = _ITE_VAR_(cond, a, b);
    }}
    ```

    **Recursion over a string.** Recurse on `s + 1` until
    `__is_certain(_EQ_(*s, '\0'))`. The inputs are bounded, so this terminates.

    **How primitives affect the approximation**
    - Exact: every case is kept (case split + `_ITE_VAR_`).
    - Under-approximation: `__assume` keeps only some executions. Examples: assume
    one branch only, or concretize with `v = __maximize(x); __assume(_EQ_(x, v));`.
    - Over-approximation: return a fresh `__sym_var(bits)`, possibly constrained by
    `__assume` to a range that contains every real result.
    
    #Library function code:
    ```c
    {libc_code}
    ```
    """
)
glibc_func_prompt = PromptTemplate(
    input_variables=["func_name"],
    template = """#Task: fetch the source code of the glibc function {func_name} from the local glibc repository and return it as a C code block.
    you must not include any other text or explanation, only the C code block."""
)

def extract_c_code(text: str) -> str:
    code = re.findall(r"```(?:c|C)?[ \t]*\n(.*?)```", text, re.S)
    if not code:
        raise ValueError("no C code found in LLM response")
    return "\n".join(code).strip() + "\n"


if __name__ == "__main__":
    #func_name = sys.argv[1]
    func_name = "printf"
    chain_glib = glibc_func_prompt | llm
    func_code_response = chain_glib.invoke({"func_name": func_name})
    print(func_code_response.content)

    chain_gensum = sum_gen_prompt | llm
    response = chain_gensum.invoke({"func_name": func_name, "libc_code": extract_c_code(func_code_response.content)})
    print(extract_c_code(response.content))
