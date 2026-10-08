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
    - If {func_name} returns void, its only observable effect is memory (see Rule 3).
    Do not define a `{func_name}_w` wrapper.
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
    execution into several paths. Use the case split (Rule 1) instead.
    - Constraints (`cnstr_t`) are opaque handles, not booleans. Never combine or
    test them with `!`, `&&`, `||`, `==` or `if (c)`. Use the constructors below,
    and query them with `is_certain` / `is_sat`.
    - Sizes of symbolic variables are given in bits (char = 8, int = 32, 32-bit
    target).

    ### Types
    - `symbolic` = `void *` (a symbolic value), `cnstr_t` (constraint handle),
    `list_t` (a symbolic byte list), `size_t` = `unsigned int`, `ssize_t` = `int`.
    - Read a byte only via its char type:
    `symbolic c = (symbolic)(unsigned long)*(const unsigned char *)p;`
    Never dereference a `symbolic *` (`*(symbolic *)p` reads 4 bytes, not 1).
    - Never use a `symbolic` directly in pointer arithmetic (`p + sym` is a
    compile error). Add a symbolic offset with a cast, `p + (unsigned long)k`,
    or merge whole pointers and cast the result:
    `return (char *)_ITE_VAR_(c, (symbolic)p1, (symbolic)p2);`
    - Lengths and counters returned as `symbolic` are also `void *`: never write
    `1 + r`, `(symbolic)1 + r` or `r1 - r2`. Compute in `unsigned long` and cast back:
    `symbolic k = (symbolic)((unsigned long)r + 1);`
    - The API functions need no `#include`. The summary file is compiled together
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

    Pointer arguments point to caller arrays, not `mem_alloc` blocks; their size
    is unknown. Never call `n_allocd` or `mem_free` on them.

    ### 5. Symbolic lists (for memory segments of symbolic length)
    `list_t lst_mk(void)` (empty), `list_t lst_cons(symbolic v, list_t l)`,
    `symbolic lst_hd(list_t l)`, `list_t lst_tl(list_t l)`,
    `cnstr_t lst_empty(list_t l)`, `size_t lst_len(list_t l)`,
    `list_t lst_nbytes(char c, size_t n)`, `list_t lst_zeros(size_t n)`

    ### How primitives affect the approximation
    - Exact: every case is kept (case split + `_ITE_VAR_`).
    - Under-approximation: `assume` keeps only some executions. Examples: assume
    one branch only, or concretize with `v = maximize(x); assume(_EQ_(x, v));`.
    - Over-approximation: return a fresh `sym_var(bits)`, possibly constrained by
    `assume` to a range that contains every real result.

    ### Rule 1: Case split on every symbolic condition
    A condition `c` that depends on a symbolic value has three cases: certainly
    true, certainly false and undecided. Handle all three; this checks `c`
    without forking and merges both branches into one path:
    ```c
    symbolic r;
    if (is_certain(c))            {{ r = A; }}
    else if (is_certain(_NOT_(c))) {{ r = B; }}
    else {{
    push_pc(); assume(c);        symbolic a = A; pop_pc();
    push_pc(); assume(_NOT_(c)); symbolic b = B; pop_pc();
    r = _ITE_VAR_(c, a, b);
    }}
    ```
    A two-way `if (is_certain(c)) A else B` is wrong in an exact summary: when `c`
    is only possible, it silently executes B for inputs where A is correct.
    This applies to nested conditions too: after `assume(c1)`, a second
    condition `c2` is usually still undecided and needs its own case split.
    `assume(c)` aborts the test if `c` is impossible on the current path. Only
    `assume` a condition in the undecided branch of its own case split; never
    `assume` two conditions in a row without checking the second one.

    ### Rule 2: Recursion
    Model loops as recursion. Each step has an end condition `end`:
    - String terminator: `end = _EQ_(c, 0)`, with `c` the current byte.
    - Length bound: `end = _EQ_(n, 0)`. A size parameter (`n`, `len`, `count`, ...)
    is usually symbolic, so `is_certain(end)` alone never becomes true.
    - Scan without length or terminator (e.g. rawmemchr: the library assumes the
    byte occurs): `end = _EQ_(c, ch)`. Do not invent a length and do not stop
    at `'\\0'`.
    A function may have several end conditions (e.g. `n == 0` and `'\\0'`). Nest
    them in the library's order: test the second (e.g. `found` in memchr) only
    inside the assume block of the first (`n != 0`).
    Only a condition that stops the library's loop is an end condition; others
    (e.g. strncpy's `'\\0'`, which only stops advancing src) are ordinary case
    splits inside the step. Both branches of such a split still recurse: a branch
    whose result stops changing (e.g. strncpy/stpncpy after `'\\0'`: the returned
    position stays the same) still has to do the remaining writes until an end
    condition holds.

    Write EVERY recursive helper with this skeleton. `assume(_NOT_(end))` appears
    exactly once, directly after the base case, and wraps the whole rest of the step:
    ```c
    static symbolic rec(char *d, const char *s, size_t n, cnstr_t guard) {{
        cnstr_t end = _EQ_(n, 0);
        if (is_certain(end)) {{ return END; }}
        push_pc(); assume(_NOT_(end));        /* from here on n != 0 on every path */
        cnstr_t g = _AND_(guard, _NOT_(end));
        /* read the byte, case splits (Rule 1), guarded writes (Rule 3),
           recursive calls with s + 1 and n - 1 */
        symbolic r = ...;
        pop_pc();
        return _ITE_VAR_(end, END, r);
    }}
    ```
    - All reads of the current byte and all calls with `n - 1` (recursion or
    another helper) belong inside this block. Never add separate
    `assume(_NOT_(end))` calls to single branches instead. `n - 1` on a path where
    `n == 0` is possible wraps to 4294967295 and never terminates.
    - The skeleton needs no separate `is_certain(_NOT_(end))` case: if `end` is
    certainly false, the `assume` changes nothing and `_ITE_VAR_` yields `r`.
    - If the library writes before it tests the end condition (e.g.
    `while ((*d = *s) != 0)` in strcpy/strlcpy also copies the terminator), do this
    write with the incoming guard BEFORE the base case, so it happens in the end
    step too: `cond_write(d, c, guard); if (is_certain(end)) {{ return END; }}`
    - The undecided end case occurs at EVERY recursion depth (`n - k` and the bytes
    are symbolic), so every helper that returns a value ends with
    `return _ITE_VAR_(end, END, r);`. A `void` helper simply ends with `pop_pc();`;
    its guarded writes cover the end case (Rule 3).
    - On each path a recursion must reach a base case whose condition becomes
    *certain* via `assume` on that path. A base case that is only tested with
    `is_certain`, without such an `assume`, does not terminate.

    ### Rule 3: Writes
    `push_pc`/`assume`/`pop_pc` only change the path condition; memory writes
    made in between stay visible on all paths. Output buffers are compared byte
    by byte with the library's: every byte the library writes must be written
    with the same value under the same condition, and no other byte may change.
    - A plain `*p = v;` or `cond_write(p, v, TRUE)` is only correct when the write
    happens on every path, i.e. its condition is certain.
    - A write that depends on undecided conditions must be guarded:
    `cond_write(p, v, guard)`, where `guard` is the conjunction (`_AND_`) of all
    undecided conditions that lead to this write.
    - In a recursion, pass the guard as a parameter (start with `TRUE` only when
    the call is not inside an `assume` block) and extend it at each step:
    `g = _AND_(guard, _NOT_(end))`.
    - Every case split inside a step also extends the guard: writes in the
    `assume(c)` branch use `_AND_(g, c)`, writes in the `assume(_NOT_(c))` branch
    use `_AND_(g, _NOT_(c))` (e.g. strncpy: `*s == 0` decides whether s advances).
    - This also holds outside recursion, e.g. in {func_name} itself: a call made
    inside `push_pc(); assume(c); ... pop_pc();` must receive `c` (conjoined with the
    current guard) as its guard, never `TRUE`. Example:
    `push_pc(); assume(_NOT_(n_zero)); r = rec(d, s, n - 1, _NOT_(n_zero)); pop_pc();`
    - Model pointer updates literally: a pointer the library does not advance
    (e.g. strlcpy: `if (n) ++dst;`) stays the SAME pointer in the recursion; later
    writes overwrite that byte. Never replace a buffer pointer by `(char *)0`.
    A library-local dummy buffer (strlcpy with n == 0: `dst = dummy`) means "no
    visible writes": pass the original pointer with guard `FALSE`.
    Example, filling `n` bytes with 0 (Rule 2 skeleton as a `void` helper):
    ```c
    static void fill(unsigned char *p, size_t n, cnstr_t guard) {{
        cnstr_t end = _EQ_(n, 0);
        if (is_certain(end)) {{ return; }}
        push_pc(); assume(_NOT_(end));
        cnstr_t g = _AND_(guard, _NOT_(end));
        cond_write(p, 0, g);
        fill(p + 1, n - 1, g);
        pop_pc();
    }}
    ```

    ### Rule 4: Return value
    Return exactly what the library returns: check its `return` statement and
    which buffer the returned pointer points into. The recursion advances its
    pointers, so the pointer reached at the base case is NOT the original argument.
    - Unchanged parameter (e.g. `dest`/`s1` in strcpy, strcat, memcpy, memset):
    the recursive helper only does the work (returns `void`), and {func_name}
    returns the saved original parameter.
    - Value that depends on where the recursion stops (e.g. strlen, strchr):
    merge the results with `_ITE_VAR_` (Rule 2).
    - Writes AND such a value (e.g. stpcpy, stpncpy, mempcpy): ONE recursion that
    writes with the guard and returns the merged result. Never walk a buffer a
    second time just for the result.
    - If the library builds the result from another pointer (e.g. stpncpy:
    `return s1 + (s2 - p);` points into s1, not s2), let the recursion return
    the number of steps `k` and build the result in {func_name} with the
    library's own expression (`s1 + k`).
    - Comparison functions (strcmp, strcasecmp, memcmp, ...): return the library's
    exact expression (e.g. the byte difference `c1 - c2`), not -1/0/1. Also at
    the terminator the result is computed from the bytes (`*s1 == 0` -> `0 - c2`).

    ### Self-check
    Before answering, check EVERY helper and EVERY branch:
    - each recursive helper follows the Rule 2 skeleton: ONE `assume(_NOT_(end))`
    block directly after the base case, every byte read and every call with
    `n - 1` inside it, and (if it returns a value) `return _ITE_VAR_(end, END, r);`
    at the end;
    - each write uses the guard including the current branch condition;
    - every `TRUE` passed as a guard: it is only allowed outside of every
    `push_pc(); assume(...); ... pop_pc();` block. Inside such a block (also in
    {func_name}) pass the assumed condition instead, e.g. `_NOT_(n_zero)`.

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
    - "memory" ("mem_s" -> "mem_s_0", ...) is the content of buffer `s` after the
    call. If it differs from what the library writes for these inputs, a write is
    missing, has the wrong value, or is not guarded (e.g. a byte is written although
    `n` is 0 -> use `cond_write` with the guard of the path).
    - For pointer return types, "ret" is an address. If the two counterexamples
    have "ret" values that differ by a small offset, the summary returns a
    pointer advanced by the recursion instead of the pointer the library
    returns -> compare with the library's `return` statement.
    Trace the summary on these concrete inputs, find the branch that produces the
    wrong result, and fix that branch. Usually it is a missing case split.
    - If the test results start with "The summary does not compile", there are no
    counterexamples: fix only the reported compiler errors and keep the logic.
    The shown source line identifies the faulty statement; the line numbers refer
    to the generated test file, not to the summary.

"""
def gen_symbolic_summary(approx_mode: str, func_name:str, libc_code:str) -> str: 
    print("Summary generation running...")
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
    print("Revision generation running...")
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
