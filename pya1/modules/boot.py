import _board
import esp32

_board.init()

u = esp32.ULP()
u.run_embedded()
u.set_wakeup_period(1000)
print("driver setup done")