import numcalc

numcalc.__sleep.exitLowPowerSleep()
numcalc.clearLeds()
numcalc.statusLed(0,2,0)
numcalc.tone()
numcalc.Keys().clearAll()