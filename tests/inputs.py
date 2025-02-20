import keys
import nav
import time
import esp32

k = keys.Keys()
k.clearAll()

u = esp32.ULP()
u.pause()
u.run_embedded()
u.write(u.VAR_SYSTEM_SLEEPING,0)
u.set_wakeup_period(1000)
u.resume()

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
    print("\033[K","Keys:",k.getRaw(),"\033[0m")
    for row in range(5):
        line = ''
        for key in range(4):
            _key = 1 << (key + (5-row-1)*4)
            line += "\033[" + str(31 + key_grp[row][key]) + "m"
            line += '[#]' if k == _key else '[ ]'
        print("\033[K",line)
    print("\033[0m")
    print("\033[K","Turns",nav.turns())
    print("\033[K","Home",nav.shouldBack())
    time.sleep(0.1)
