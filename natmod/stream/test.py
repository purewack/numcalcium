import stream_nat
import os

with open("data.txt", "rb") as f:
    stream_nat.use(f)
