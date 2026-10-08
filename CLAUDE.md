# BA: LLM-generated symbolic summaries for libc functions

Bachelor's thesis (TUM). Pipeline: fetch a libc function's source → LLM writes a symbolic summary (C, using SummBoundVerify's symbolic reflection API) → SummBoundVerify (`summbv`) generates and runs a validation harness → if the result isn't accepted, the LLM revises the summary (max 3 times) → results sorted into `cur_Files/`.

## Files
| File | Role |
|------|------|
| `Pipeline.py` | Entry point. `insert_parameters()` sets `func_name` and `approx_mode` (currently hardcoded). Runs summbv, revision loop, sorting. |
| `SummaryGeneration.py` | Prompts + LLM calls for generating and revising summaries (`gen_symbolic_summary`, `gen_revision_summary`). |
| `ConcreteCode.py` | Builds header-free `concrete.c` deterministically from uClibc via pycparser (`get_concrete_code`). `gen_concrete_code` is the older LLM-based variant. |
| `LibCCode.py` | Finds and fetches function source from `uClibc/` (current) or `musl/` (legacy). |
| `Helpers.py` | `extract_c_code`: pulls the C code block out of an LLM response. |
| `lib.c` | malloc/free shims passed to summbv via `--lib`. |
| `uClibc_stubs/` | Stub headers used when preprocessing uClibc. |
| `SBV-usage.txt` | Notes on summbv flags and config options. |

## Results
- Approximation modes: `exact`, `over`, `under`. `exact` results are accepted in every mode.
- Output goes to `cur_Files/{exact,over,under,bug}/<func>-tests/` (concrete, summary, harness, binary, result JSON).
- Test timeout: 120 s (`TEST_TIMEOUT`).

## LLM
- Morpheus API (TUM), OpenAI-compatible, via `langchain_openai`. Model: `google/gemma-4-31B-it`.
- The key is `MORPHEUS_API_KEY` in `.env`. Never print or read `.env`.

## Rules
- Always use the venv at `/home/flo/_Uni/BAv1.1/sbv-env`: run `/home/flo/_Uni/BAv1.1/sbv-env/bin/python` and `/home/flo/_Uni/BAv1.1/sbv-env/bin/pip`, never the system `python`/`pip`. `summbv` is also in that venv's `bin/`.
- Never modify anything inside `SummBoundVerify/` on your own.
- Don't modify vendored libc sources (`uClibc/`, `musl/`).
- `BAv1.0` (sibling folder) is the old version. Work in `BAv1.1`.

## Run
```bash
/home/flo/_Uni/BAv1.1/sbv-env/bin/python Pipeline.py
```
