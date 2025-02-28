import time
import bmp
import framebuf
import board
from microbmp import MicroBMP

# Define display size (modify as needed)
width, height = 64, 64  # Match your display resolution

lcd = board.LCD()
lcd.clear()

# Open the BMP file
t = time.ticks_ms()
print("opening")
with open("test.bmp", "rb") as f:
    # Create a FrameBuffer using a bytearray in RGB565 format
    buf = bytearray(width * height * 2)
    fb = framebuf.FrameBuffer(buf, width, height, framebuf.RGB565)
    
    print("converting",t)
    # Load the BMP image into the framebuffer
    bmp.parse(f, buf)

    lcd.buffer(fb,0,0,width,height)

print("done, took ms:",(time.ticks_ms()-t))    

import sys
sys.exit()

t = time.ticks_ms()
scale = 1
FRAMEBUF_WIDTH = 64 * scale
FRAMEBUF_HEIGHT = 64 * scale
fbuf = framebuf.FrameBuffer(bytearray(FRAMEBUF_WIDTH * FRAMEBUF_HEIGHT * 2), FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT, framebuf.RGB565)
img = MicroBMP().load("test.bmp")
x=0
y=0
print(img.palette)
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
        
        for sy in range(scale):
            for sx in range(scale):
                fbuf.pixel((x*scale)+sx,(y*scale)+sy,irgb)

lcd.buffer(fbuf, 0,0, FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT)

print("done, took ms:",(time.ticks_ms()-t))    
