import re
#extract C code from LLM response
def extract_c_code(text: str) -> str:
    code = re.findall(r"```(?:c|C)?[ \t]*\n(.*?)```", text, re.S)
    if not code:
        raise ValueError("no C code found in LLM response")
    return keep_last_definitions("\n".join(code).strip() + "\n")

#split C code into top-level chunks (function definitions, declarations), skipping strings and comments
def split_top_level(code: str) -> list[str]:
    chunks, depth, start, i = [], 0, 0, 0
    while i < len(code):
        if code[i] == "#" and depth == 0 and (i == 0 or code[i - 1] == "\n"):
            end = code.find("\n", i) % (len(code) + 1)
            chunks += [code[start:i], code[i:end + 1]]
            start, i = end + 1, end
        elif code.startswith("//", i):
            i = code.find("\n", i) % (len(code) + 1)
        elif code.startswith("/*", i):
            i = code.find("*/", i + 2) % (len(code) + 1) + 1
        elif code[i] in "\"'":
            end = re.compile(rf"(?<!\\)(?:\\\\)*{code[i]}").search(code, i + 1)
            i = end.end() - 1 if end else len(code)
        elif code[i] == "{":
            depth += 1
        elif code[i] == "}":
            depth -= 1
            if depth == 0:
                chunks.append(code[start:i + 1])
                start = i + 1
        elif code[i] == ";" and depth == 0:
            chunks.append(code[start:i + 1])
            start = i + 1
        i += 1
    if code[start:].strip():
        chunks.append(code[start:])
    return chunks

#LLMs sometimes append a corrected second version -> keep only the last definition of each function
def keep_last_definitions(code: str) -> str:
    chunks = split_top_level(code)
    names = []
    for chunk in chunks:
        header = chunk.split("{", 1)[0]
        m = re.search(r"(\w+)\s*\([^;]*\)\s*$", header)
        names.append(m.group(1) if chunk.rstrip().endswith("}") and m else None)
    defined = [n for n in names if n]
    if len(defined) == len(set(defined)):
        return code
    kept = [c for i, (c, n) in enumerate(zip(chunks, names))
            if c.lstrip().startswith("#")
            or (n is None and c.strip() not in [x.strip() for x in chunks[:i]]) or (n and n not in names[i + 1:])]
    return "".join(kept).strip() + "\n"
