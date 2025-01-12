import machine

s = machine.Pin(16, machine.Pin.OPEN_DRAIN)

machine.deepsleep()
