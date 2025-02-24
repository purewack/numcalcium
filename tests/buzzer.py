import machine
import time

melody = [69,73,76,81,76,73]

def mToF(n):
    return int(pow(2,(n-69)/12)*440)


p = machine.Pin(machine.Pin.board.BUZZER)
spk = machine.PWM(p)
v = 1

while True:
    for m in melody:
        n = mToF(m)
        spk.freq(n)
        print(m,n)
        time.sleep(0.075)
    v = (v+1) % 24
    v += 1
    spk.duty(v * 8)
    print("d",v)
