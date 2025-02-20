import board
import os

sd = board.SD()
os.mount(sd,'/sd')
items = os.listdir('/sd')

lcd = board.LCD()
lcd.color(lcd.WHITE)
lcd.background(lcd.BLACK)

lcd.clear()
lcd.cursor(0,0)
for i in items:
    lcd.print(i)
    lcd.print("\n\r")
