import _board
import board
import esp32

_board.init()
board.clearLights()

u = esp32.ULP()
u.run_embedded()
u.set_wakeup_period(1000)
