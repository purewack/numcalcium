import neopixel
import machine
import time

n = neopixel.NeoPixel(machine.Pin(47),21)
n[0]  = (10,10,10)
n.write()

print("hello")

time.sleep(2)
n[0]  = (0,0,0)
n.write()
