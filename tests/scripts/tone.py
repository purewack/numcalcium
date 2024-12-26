import machine
import time

melody = [69,73,76,81,76,73]

def mToF(n):
    return int(pow(2,(n-69)/12)*440)


p = machine.Pin(21)
spk = machine.PWM(p)
spk.duty(12)

while True:
    for m in melody:
        n = mToF(m)
        spk.freq(n)
        print(m,n)
        time.sleep(0.07)
