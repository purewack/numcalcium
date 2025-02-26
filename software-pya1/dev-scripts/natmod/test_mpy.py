import mpy
import gc

with open("hello.mpy","rb") as f:
    mpy.from_stream(f)
