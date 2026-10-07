import re
#extract C code from LLM response
def extract_c_code(text: str) -> str:
    code = re.findall(r"```(?:c|C)?[ \t]*\n(.*?)```", text, re.S)
    if not code:
        raise ValueError("no C code found in LLM response")
    return "\n".join(code).strip() + "\n"
