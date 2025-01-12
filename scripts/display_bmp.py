import machine
import board
import terminal
import os
import framebuf
from microbmp import MicroBMP

sd = board.SD()
os.mount(sd,'/sd')

path = "/sd/hellomiki.bmp"
FRAMEBUF_WIDTH = 64
FRAMEBUF_HEIGHT = 64

fbuf = framebuf.FrameBuffer(bytearray(FRAMEBUF_WIDTH * FRAMEBUF_HEIGHT * 2), FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT, framebuf.RGB565)

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
    terminal.options(rgbSwap=True)
    terminal.buffer(FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT,fbuf)

except Exception as e:
    print("\n\nError:", e)

