import machine
home = machine.Pin(0, machine.Pin.IN)

import esp32
_u = esp32.ULP()

def shouldBack(value = None):
    if value == None:
        return not home.value()
    else:
        _u.write(_u.VAR_BOK,value)

def wasBackRequested():
    if _u.read(_u.VAR_BOK):
        _u.write(_u.VAR_BOK,0)
        return True
    return False

def turns(invert=False):
    t = 0 - _u.read(_u.VAR_TURNS)
    _u.write(_u.VAR_TURNS, 0)
    return t
