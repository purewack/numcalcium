import machine
import neopixel
import time

colors = [(10,0,0),(0,10,0),(0,0,10),(10,10,10),(127,127,127),(0,0,0)]
leds = neopixel.NeoPixel(machine.Pin.board.LEDS,21)

for i in range(21):
    for c in colors:
        leds[i] = c
        leds.write()
        time.sleep(0.125)

time.sleep(1)

for i in range(21):
    leds[i] = (127,127,127)
    leds.write()
    time.sleep(0.075)

time.sleep(2)

for i in range(21):
    leds[i] = (0,0,0)
leds.write()
