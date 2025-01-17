#import esp32
#import time
#
#u = esp32.ULP()
#u.run_embedded()
#
#time.sleep(0.3)
#
#print(u.read(u.VAR_BSCAN))
#
#if(not u.read(u.VAR_BSCAN) == 1):
#	print("normal boot")
#	import keypad
#else:
#	print("stop boot")

import board
import machine

board.init()
terminal = board.Terminal()
terminal.clear()
terminal.cursor(0,0)
terminal.print("Reset: " + str(machine.reset_cause()))
terminal.print("\n\r" + str(machine.PWRON_RESET))
terminal.print("\n\r" + str(machine.HARD_RESET))
terminal.print("\n\r" + str(machine.WDT_RESET))
terminal.print("\n\r" + str(machine.DEEPSLEEP_RESET))
terminal.print("\n\r" + str(machine.SOFT_RESET))
