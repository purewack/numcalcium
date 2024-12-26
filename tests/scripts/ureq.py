import urequests
import network
import time
import machine

led = machine.Pin(34, machine.Pin.OUT)
led.value(0)
time.sleep(2)
ssid = "EE-36qfga"
psk = "aqua-fix-optic"
nic = network.WLAN(network.STA_IF)
nic.active(True)
led.value(1)

print('[net] connecting to network:', ssid)
while True:
    # try:
        nic.connect(ssid, psk)
        ticks = 0
        while not nic.isconnected():
            led.value(0)
            time.sleep(0.1)
            led.value(1)
            print("[net] .")
            ticks += 1
            if(ticks > 500):
                print("[net] waiting too long, system reset")
                import machine
                machine.reset()
        break
    # except Exception as e:
    #     print("[net] connection error, retry:",e)
    #     time.sleep(1)


print('[net] network connected:', nic.ifconfig())


while True:
    r = urequests.get("https://google.com")
    print(r.raw.read(128))
    led.value(0)
    time.sleep(1)
    led.value(1)
