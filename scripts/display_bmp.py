import machine
import board
import os
import framebuf
from microbmp import MicroBMP

print("starting")

board.init()

terminal = board.Terminal()

print("mounting")

sd = board.SD()
os.mount(sd,'/sd')

path = "/sd/hellomiki.bmp"
FRAMEBUF_WIDTH = 64
FRAMEBUF_HEIGHT = 64

fbuf = framebuf.FrameBuffer(bytearray(FRAMEBUF_WIDTH * FRAMEBUF_HEIGHT * 2), FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT, framebuf.RGB565)

print("printing")

try:
    img = MicroBMP().load(path)
    x=0
    y=0
    for y in range(64):
        for x in range(64):
            px = x + y*64
            p = img.parray[px]
            c = img.palette[p]
            r = c[0] 
            g = c[1] 
            b = c[2] 
            rgb565 = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
            irgb = ((rgb565 & 0xff)<<8) | (rgb565>>8)
            fbuf.pixel(x,y,irgb)
    terminal.buffer(fbuf, 0,0, FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT)

except Exception as e:
    print("\n\nError:", e)

