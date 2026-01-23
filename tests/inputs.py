import __keys
import time
import esp32

k = __keys.Keys()
k.clearAll()

kPrev = 0

key_grp = [
    [5,5,6,6],
    [4,4,4,5],
    [2,3,3,3],
    [1,1,2,2],
    [0,0,0,1]
]
print("\033[H\033[2J Inputs test")
while True:
    print("\033[H")
    print("\033[K","systick - ",time.ticks_us())
    print("\033[K","Keys:",k.raw(), k.getAllDown(),"\033[0m")
    for row in range(5):
        line = ''
        for key in range(4):
            _key = (key + (5-row-1)*4)
            line += "\033[" + str(31 + key_grp[row][key]) + "m"
            line += '[#]' if k == _key else '[ ]'
        print("\033[K",line)
    print("\033[0m")
    print("\033[K","Turns",k.raw_turns())
    print("\033[K","Home",k.raw_home())

    while True:
        if(not (k.raw() + k.raw_home() + k.raw_turns()) == kPrev):
            kPrev = k.raw() + k.raw_home() + k.raw_turns()
            break
        time.sleep(0.1)
    