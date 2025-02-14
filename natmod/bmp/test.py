import time
import bmp
import framebuf
import board

# Define display size (modify as needed)
width, height = 64, 64  # Match your display resolution

lcd = board.LCD()

# Open the BMP file
print("opening")
with open("test.bmp", "rb") as f:
    # Create a FrameBuffer using a bytearray in RGB565 format
    buf = bytearray(width * height * 2)
    fb = framebuf.FrameBuffer(buf, width, height, framebuf.RGB565)
    
    print("converting",time.time())
    # Load the BMP image into the framebuffer
    bmp.load_rgb565(f, buf)
    print("done",time.time())    

    lcd.buffer(fb,0,0,width,height)
    # Now you can use `fb` to draw on a display
    # Example: If using an SPI TFT driver, you can call:
    # display.blit_buffer(buf, 0, 0, width, height)

