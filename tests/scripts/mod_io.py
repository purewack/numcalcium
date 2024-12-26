import hwio
import time

while True:
    d = hwio.downKeys()
    hwio.downKeys(d)
    print(bin(d))
    time.sleep(0.1)
