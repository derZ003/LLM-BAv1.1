import re
import sys
import ctypes
import tarfile
from functools import lru_cache
from pathlib import Path

GLIBC_DIR = Path(__file__).resolve().parent / "glibc"

@lru_cache(maxsize=None)
def get_glib_files():
    files = {}
    for path in GLIBC_DIR.rglob("*.c"):
        files.setdefault(path.name, []).append(path)
    return files

def remove_comments(code):
    code = re.sub(r"//.*", "", code)
    code = re.sub(r"/\*.*?\*/", "", code, flags=re.DOTALL)
    return code

def find_glib_file(func_name):
    candidates = get_glib_files().get(f"{func_name}.c")
    if not candidates:
        raise ValueError(f"{func_name}.c not found in {GLIBC_DIR}.")
    return min(candidates, key=lambda p: ("sysdeps" in p.parts, len(p.parts), str(p)))

def get_glib_code(func_name):
    return remove_comments(find_glib_file(func_name).read_text(errors="replace"))

if __name__ == "__main__":
    print(get_glib_code(sys.argv[1]))
