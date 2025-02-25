import board
import network
import requests
import time

def cb(b):
    pass

buf = bytearray(4000)
dac = board.DAC(buf,cb)

n = network.WLAN()
n.active(True)
with open("local.psk.txt","r") as f:
    ssid = f.readline().rstrip('\n')
    psk = f.readline().rstrip('\n')
    print("using wifi",ssid,psk)
n.connect(ssid,psk)
while not n.isconnected():
    board.statusLed(10,10,10)
    time.sleep(0.1)
    board.statusLed(0,0,0)
    time.sleep(0.1)

lcd = board.LCD()
lcd.clear()
lcd.print("Connected\n\r")
lcd.print(str(n.ifconfig()))

for i in range(10):
    r = requests.get("http://example.com/")
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

dac.deinit()
