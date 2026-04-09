import numcalc
import time

lcd = numcalc.LCD()

with open("test.bmp","rb") as _f:
    img = lcd.parseBMP(_f)
    
lcd.clear()
lcd.createCanvas()
lcd.bitmap(320//2,64,img,scale=2)
lcd.bitmap(320//2,64,img,scale=1.5)
lcd.bitmap(320//2,64,img,scale=1)
lcd.update()
# for a in range(10):
#     lcd.bitmapScaledFloat(320//2,64,img,scale=1 + a/10)
#     lcd.update()
#     time.sleep(0.1)