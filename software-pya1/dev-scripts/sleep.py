import machine
import esp32
import time

u = esp32.ULP()
u.run_embedded()
u.write(u.VAR_WAKE,1)

u.rtc_init(16)
u.rtc_init(0)
s = machine.Pin(0,machine.Pin.IN,hold=True)
p = machine.Pin(16,machine.Pin.OPEN_DRAIN,value=0,hold=True)

esp32.wake_on_ulp(True)
esp32.wake_on_touch(False)
esp32.gpio_deep_sleep_hold(True)

machine.deepsleep()
