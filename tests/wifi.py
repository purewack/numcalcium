import network
import time
import board

board.init()
t = board.Terminal()

ssid = "EE-36qfga"
psk = "aqua-fix-optic"

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
            t.print("[net] .")
        break


ip = nic.ifconfig()[0] + ''
subnet = nic.ifconfig()[1] + ''

t.print(ip)
t.print(subnet)
