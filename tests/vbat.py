import numcalc
import time
import os

lcd = numcalc.LCD()
os.dupterm(lcd)
k = numcalc.Keys()

def dischargeCycle():
    for i in range(20):
        time.sleep(0.01)
        numcalc.led(i,200,200,200)
    for i in range(20):
        time.sleep(0.01)
        numcalc.led(i,0,0,0)

discharing = False

bat = numcalc.Battery()

while True:
    if(k.isAnyDown()):
        k.clearAll()
        discharing = not discharing

    if discharing:
        dischargeCycle()

    lcd.clear() 
    print(bat.isAC(),bat.present())
    print(bat.voltage())
    print("-----")
    time.sleep(0.1)