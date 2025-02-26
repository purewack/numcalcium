import machine
import random
import time
import board

t = board.LCD()

board.statusLed(0,0,0)

for i in range(20):
    board.statusLed(random.randint(0,40),random.randint(0,40),random.randint(0,40))
    time.sleep(0.2)
    t.cursor(0,0)
    t.print("hello ")
    t.print(str(random.randint(0,255)))

board.statusLed(0,0,0)
a = 42
