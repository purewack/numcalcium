import _board
import keys
import machine
import pins

_board.init()
k = keys.Keys()

def LCD():
    return _board.Terminal()

def SD():
    return _board.SD()

def keys():
    return k
