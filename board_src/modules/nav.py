import machine
home = machine.Pin(0, machine.Pin.IN)

import esp32
_u = esp32.ULP()
_u.run_embedded()

def shouldBack():
    return not home.value()

def getCursorSteps():
    return _u.read(_u.VAR_TURNS)
