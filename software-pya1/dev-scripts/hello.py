import machine
import random
import time
import board

board.init()
t = board.Terminal()

import neopixel
n = neopixel.NeoPixel(machine.Pin(47),1)

n[0]  = (0,0,0)
n.write()

for i in range(20):
    n[0] = (random.randint(0,40),random.randint(0,40),random.randint(0,40))
    n.write()
    time.sleep(0.1)
    t.cursor(0,0)
    t.print("hello ")
    t.print(str(random.randint(0,255)))

n[0] = (0,0,0)
n.write()
