import board
import time
lcd = board.LCD()
lcd.clear()
lcd.print("Goind to sleep, 3s")
time.sleep(3)

board.enterLowPowerSleep()