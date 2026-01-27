import numcalc
import os

sd = numcalc.SD()
os.mount(sd,'/sd')
items = os.listdir('/sd')

lcd = numcalc.LCD()
lcd.color(lcd.WHITE)
lcd.background(lcd.BLACK)

lcd.clear()
lcd.cursor(0,0)
for i in items:
    lcd.print(i)
    lcd.print("\n\r")
