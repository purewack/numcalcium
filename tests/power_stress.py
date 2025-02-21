import board
import network
import requests
import time
import sigma_delta

lcd = board.LCD()

n = network.WLAN()
n.active(True)
with open("local.psk.txt","r") as f:
    ssid = f.readline().rstrip('\n')
    psk = f.readline().rstrip('\n')
n.connect(ssid,psk)

lcd.print("Connected\n\r")
lcd.print(str(n.ifconfig()))

def cb(b):
    pass

buf = bytearray(4000)

sigma_delta.init(buf,cb)

for i in range(10):
    r = requests.get("https://example.com/")
    t = (r.text)
    print(t)
    lcd.print(t[0:250])
    r.close()
    for l in range(20):
        board.led(l,250,250,250)
        time.sleep(0.1)
    for l in range(20):
        board.led(l,0,0,0)
        time.sleep(0.1)
