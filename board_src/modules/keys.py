import esp32

_u = esp32.ULP()
_u.run_embedded()


KEY_SHIFT = 0

KEY_DOT  = 2
KEY_0    = 1
KEY_1    = 4
KEY_2    = 5
KEY_3    = 6
KEY_4    = 8
KEY_5    = 9
KEY_6    = 10
KEY_7    = 12
KEY_8    = 13
KEY_9    = 14

KEY_EQ   = 4
KEY_PLUS = 7

def resetDown(key):
    r = _u.read(_u.VAR_BDOWN)
    r &= ~ (1 << key)
    _u.write(_u.VAR_BDOWN, r)

def resetUp(key):
    r = _u.read(_u.VAR_BUP)
    r &= ~ (1 << key)
    _u.write(_u.VAR_BUP, r)

def isAnyDown():
    return _u.read(_u.VAR_BDOWN) > 0

def isAnyUp():
    return _u.read(_u.VAR_BUP) > 0

def getFirstDown():
    k = _u.read(_u.VAR_BDOWN)
    for i in range(20):
        if((1<<i) & k):
            resetDown(i)
            return i
    
    return None

def getFirstUp():
    k = _u.read(_u.VAR_BUP)
    for i in range(20):
        if((1<<i) & k):
            resetUp(i)
            return i
    return None

def isDown(key, reset=True):
    r = _u.read(_u.VAR_BDOWN) & (1<<key)
    if(reset): resetDown(key)
    return r

def isUp(key, reset=True):
    r = _u.read(_u.VAR_BUP) & (1<<key)
    if(reset): resetUp(key)
    return r

def getAll():
    return _u.read(_u.VAR_BSCAN)

def clearAll():
    _u.write(_u.VAR_BDOWN,0)
    _u.write(_u.VAR_BUP,0)
