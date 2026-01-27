import time
import bmp
import framebuf
import numcalc


lcd = numcalc.LCD()
lcd.clear()

with open("test4.bmp","rb") as f:
    img = bmp.parse(f,scale=2)
    print(img,img['width'], img['height'], img['bpp'])
    

# Open the BMP file
t = time.ticks_ms()
print("opening")
with open("test4.bmp", "rb") as f:
    width, height = 8, 8  # Match your display resolution
    scale = 10
    # Create a FrameBuffer using a bytearray in RGB565 format
    buf = bytearray(width*scale * height*scale * 2)
    fb = framebuf.FrameBuffer(buf, width*scale, height*scale, framebuf.RGB565)
    
    print("converting",t)
    # Load the BMP image into the framebuffer
    bmp.parse(f, buf, scale=scale)

    lcd.buffer(fb,0,0,width*scale,height*scale)

print("done, took ms:",(time.ticks_ms()-t))    

t = time.ticks_ms()
print("opening")
with open("test.bmp", "rb") as f:
    width, height = 64, 64  # Match your display resolution
    scale = 1
    # Create a FrameBuffer using a bytearray in RGB565 format
    buf = bytearray(width*scale * height*scale * 2)
    fb = framebuf.FrameBuffer(buf, width*scale, height*scale, framebuf.RGB565)
    
    print("converting",t)
    # Load the BMP image into the framebuffer
    bmp.parse(f, buf, scale=scale)

    lcd.buffer(fb,128,0,width*scale,height*scale)

print("done, took ms:",(time.ticks_ms()-t))    

t = time.ticks_ms()
print("opening")
with open("test4.bmp", "rb") as f:
   
    print("converting",t)
    # Load the BMP image into the framebuffer
    img = bmp.parse(f,scale=6)
    fb = framebuf.FrameBuffer(img['buffer'], img['width'], img['height'], framebuf.RGB565)
    
    lcd.buffer(fb,0,64,img['width'],img['height'])

print("done, took ms:",(time.ticks_ms()-t))    
