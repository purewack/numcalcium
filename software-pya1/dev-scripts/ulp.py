import esp32

u = esp32.ULP()
u.run_embedded()
u.rtc_output(33)

import time
while True:
    time.sleep(0.2)
    print(u.read(0))
