import keys
import nav
import time

keys.clearAll()
while True:
    if(keys.isAnyDown()):
        print(keys.getFirstDown())
    if(nav.shouldBack()):
        print("home hold")
        print(nav.getCursorSteps())
    time.sleep(0.4)
