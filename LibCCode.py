import re
from functools import lru_cache
from pathlib import Path

MUSL_DIR = Path(__file__).resolve().parent / "musl"
UCLIBC_DIR = Path(__file__).resolve().parent / "uClibc"

@lru_cache(maxsize=None)
def get_musl_files():
    files = {}
    for path in MUSL_DIR.rglob("*.c"):
        files.setdefault(path.name, []).append(path)
    return files
@lru_cache(maxsize=None)
def get_uclibc_files():
    files = {}
    for path in UCLIBC_DIR.rglob("*.c"):
        files.setdefault(path.name, []).append(path)
    return files

def remove_comments(code):
    code = re.sub(r"//.*", "", code)
    code = re.sub(r"/\*.*?\*/", "", code, flags=re.DOTALL)
    return code

def find_musl_file(func_name):
    candidates = get_musl_files().get(f"{func_name}.c")
    if not candidates:
        raise ValueError(f"{func_name}.c not found in {MUSL_DIR}.")
    return min(candidates, key=lambda p: ("arch" in p.parts, len(p.parts), str(p)))
def find_uclibc_file(func_name):
    candidates = get_uclibc_files().get(f"{func_name}.c")
    if not candidates:
        raise ValueError(f"{func_name}.c not found in {UCLIBC_DIR}.")
    return min(candidates, key=lambda p: ("sysdeps" in p.parts, len(p.parts), str(p)))

def get_musl_code(func_name):
    func_code = remove_comments(find_musl_file(func_name).read_text(errors="replace"))
    print(f"Fetched musl-code for {func_name}: \n" + func_code)
    return func_code
def get_uclibc_code(func_name):
    func_code = remove_comments(find_uclibc_file(func_name).read_text(errors="replace"))
    print(f"Fetched uClibc-code for {func_name}: \n" + func_code)
    return func_code

def get_musl_codepath(func_name):
    return str(find_musl_file(func_name))
def get_uclibc_codepath(func_name):
    return str(find_uclibc_file(func_name))
