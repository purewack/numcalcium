import sys
sys.path.append('/remote/modules')
sys.path.append('/remote/modules-build')

import numcalc
import time

k = numcalc.Keys()
while True:
    print(k.getAllDown())
    time.sleep(1)