import os
import re
import subprocess
from pathlib import Path

from dotenv import load_dotenv
from langchain_openai import ChatOpenAI
from langchain_core.prompts import PromptTemplate
from pycparser import c_ast, c_generator, c_parser

from LibCCode import *
from Helpers import extract_c_code
from LibCCode import get_uclibc_codepath

#using morpheus LLM API for code generation
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
#Prompt for generating concrete testfile for SummboundVerify
concrete_gen_prompt = PromptTemplate(
    input_variables=["func_name", "libc_code"],
    template="""#Task: You are a senior software engineer. Rewrite the following uclibc function {func_name}
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
#generate concrete code using LLM
def gen_concrete_code(func_name:str, libc_code:str) -> str:
    chain_genconcrete = concrete_gen_prompt | llm
    response = chain_genconcrete.invoke({"func_name": func_name, "libc_code": libc_code})
    concrete_code = extract_c_code(response.content)
    concrete_code = re.sub(r"^\s*#\s*include.*\n", "", concrete_code, flags=re.M)
    print("Generated concrete code:\n" + concrete_code)
    return concrete_code

#using preprocessed uClibc code to generate concrete code
STUB_DIR = Path(__file__).resolve().parent / "uClibc_stubs"
LIB_FUNCS = {"malloc", "free"}
PARSE_PRELUDE = "typedef unsigned long size_t;\ntypedef int wchar_t;\ntypedef unsigned int __uwchar_t;\n"
N_PRELUDE = len(c_parser.CParser().parse(PARSE_PRELUDE).ext)

class CallCollector(c_ast.NodeVisitor):
    def __init__(self):
        self.defined = set()
        self.called = set()
    def visit_FuncDef(self, node):
        self.defined.add(node.decl.name)
        self.generic_visit(node)
    def visit_FuncCall(self, node):
        if isinstance(node.name, c_ast.ID):
            self.called.add(node.name.name)
        self.generic_visit(node)

class Renamer(c_ast.NodeVisitor):
    def __init__(self, names):
        self.names = names
    def visit_Decl(self, node):
        if node.name in self.names:
            node.name = "concrete_" + node.name
            t = node.type
            while not isinstance(t, c_ast.TypeDecl):
                t = t.type
            t.declname = node.name
        self.generic_visit(node)
    def visit_ID(self, node):
        if node.name in self.names:
            node.name = "concrete_" + node.name

def preprocess_uclibc(func_name: str) -> str:
    try:
        code = subprocess.run(
            ["gcc", "-E", "-P", "-undef", "-nostdinc", "-I", str(STUB_DIR / "include"),
            "-include", str(STUB_DIR / "uclibc_stub.h"), get_uclibc_codepath(func_name)],
            capture_output=True, text=True, check=True
        ).stdout
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Preprocessing failed for {func_name}: {e.stderr}") from e

    #strip typedefs and string_uchar_t
    code = code.replace("typedef unsigned char __string_uchar_t;\n", "")
    code = code.replace("__string_uchar_t", "unsigned char").strip() + "\n"
    return code

def collect_uclibc_functions(func_name: str, defined: set | None = None) -> tuple[list, set]:
    defined = set() if defined is None else defined
    if func_name in defined:
        return [], defined
    ast = c_parser.CParser().parse(PARSE_PRELUDE + preprocess_uclibc(func_name))
    collector = CallCollector()
    collector.visit(ast)
    defined |= collector.defined | {func_name}
    nodes = []
    for callee in sorted(collector.called - defined - LIB_FUNCS):
        nodes += collect_uclibc_functions(callee, defined)[0]
    return nodes + ast.ext[N_PRELUDE:], defined

def get_concrete_code(func_name: str) -> str:
    nodes, defined = collect_uclibc_functions(func_name)
    if not any(isinstance(n, c_ast.FuncDef) and n.decl.name == func_name for n in nodes):
        raise RuntimeError(f"{func_name} not defined after preprocessing {get_uclibc_codepath(func_name)}")
    ast = c_ast.FileAST(nodes)
    Renamer(defined).visit(ast)
    gen = c_generator.CGenerator()
    concrete_code = "".join(dict.fromkeys(gen.visit(c_ast.FileAST([n])) for n in ast.ext))
    print("Generated concrete code:\n" + concrete_code)
    return concrete_code
