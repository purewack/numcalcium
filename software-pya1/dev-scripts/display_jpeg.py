import machine
import board
import terminal
import os
import framebuf
from JPEGdecoder import jpeg


sd = board.SD()
os.mount(sd, "/sd")

# Path to the PNG file on the SD card
png_path = "/sd/hellomiki128.jpeg"

# Framebuffer dimensions (adjust to match your display)
FRAMEBUF_WIDTH = 128
FRAMEBUF_HEIGHT = 128
framebuf_data = bytearray(FRAMEBUF_WIDTH * FRAMEBUF_HEIGHT * 2)  # Assuming RGB565 format
fbuf = framebuf.FrameBuffer(framebuf_data, FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT, framebuf.RGB565)

def plot(x,y,color):
    terminal.color(color)
    terminal.cursor(x,y)
    terminal.plot()

try:
    # Initialize the PNG decoder
    decoder = jpeg(png_path, callback=plot, quality=1).render(0,0)

#    terminal.buffer(FRAMEBUF_WIDTH,FRAMEBUF_HEIGHT,fbuf)

except Exception as e:
    print("Error decoding or displaying PNG:", e)

finally:
    # Unmount the SD card
    os.umount("/sd")
    sd.deinit()

