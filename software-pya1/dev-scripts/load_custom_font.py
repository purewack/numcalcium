import numcalc
import font_gohu13inv

lcd = numcalc.LCD()

try:
    lcd.scale(2)
    lcd.print("Hello yo")
    lcd.cursor(0,1)
    lcd.font(font_gohu13inv.data)
    lcd.print("Hello")
    print("Font loaded successfully!")
except Exception as e:
    print(f"Error loading font: {e}")
