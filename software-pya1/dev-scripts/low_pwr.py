import numcalc
import time
lcd = numcalc.LCD()
lcd.clear()
lcd.print("Goind to sleep, 3s")
time.sleep(3)

numcalc.enterLowPowerSleep()