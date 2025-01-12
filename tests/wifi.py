import network
import time

ssid = "ssid"
psk = "psk"

nic = network.WLAN(network.STA_IF)
if(not nic.active()):
    nic.active(True)

if not nic.isconnected():
    print('[net] connecting to network:', ssid)
    while True:
        nic.connect(ssid, psk)
        ticks = 0
        while not nic.isconnected():
            time.sleep(0.5)
            print("[net] .")
        break


ip = nic.ifconfig()[0] + ''
subnet = nic.ifconfig()[1] + ''

print(ip,subnet)
