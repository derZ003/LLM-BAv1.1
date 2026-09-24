import os

from langchain import PromptTemplate, LLMChain
from dotenv import load_dotenv
import SummBoundVerify

load_dotenv()
api_key = os.environ["MORPHEUS_API_KEY"]


