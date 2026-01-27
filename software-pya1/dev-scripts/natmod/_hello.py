import machine
import random
import time
import numcalc

t = numcalc.LCD()

numcalc.statusLed(0,0,0)

for i in range(20):
    numcalc.statusLed(random.randint(0,40),random.randint(0,40),random.randint(0,40))
    time.sleep(0.2)
    t.cursor(0,0)
    t.print("hello ")
    t.print(str(random.randint(0,255)))

numcalc.statusLed(0,0,0)
a = 42
