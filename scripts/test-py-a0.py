import terminal
import machine
import neopixel
import board

n = neopixel.NeoPixel(machine.Pin(47),21)

n[0]  = (50,0,0)
n[19] = (0,50,0)
n[20] = (0,0,50)
n.write()

sd = board.SD()
vfs.mount(sd,"/sd")
with open("/sd/hello.txt") as file:
    a = file.read()

print("file",a)
terminal.print(a)
