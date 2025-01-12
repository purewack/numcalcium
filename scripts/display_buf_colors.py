import terminal
import framebuf

terminal.console(False)

fbuf = framebuf.FrameBuffer(bytearray(64 * 64 * 2), 64, 64, framebuf.RGB565)

def int_to_rgb565(value): 
    # Clamp the value to be within the range 0-255 
    value = max(0, min(value, 255)) # Calculate the RGB components 
    red = (value >> 3) & 0x1F 
    green = (value >> 2) & 0x3F 
    blue = (value >> 3) & 0x1F # Combine the RGB components into a single 16-bit value 
    rgb565 = (red << 11) | (green << 5) | blue 
    return rgb565

fbuf.fill(0)
for i in range(256):
    fbuf.rect(i,0,1,8,int_to_rgb565(i),True)

terminal.cursor(0,0)
terminal.buffer(64,64,fbuf)

gc.collect()
terminal.console(True)
#input()
