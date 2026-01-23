import board
import framebuf
# import bmp

terminal = board.LCD()
terminal.background(terminal.BLACK)
terminal.color(terminal.BLACK)
terminal.clear()
terminal.options(rgbSwap=False,scale=1,invert=False)

terminal.fill(48,48,16,16,terminal.GREEN)
for i in range(128):
    terminal.plot(i,i,terminal.GREEN)

for i in range(128):
    terminal.plot(64,i,terminal.BLUE)

for i in range(128):
    terminal.plot(128-i,i,terminal.RED)


terminal.cursor(0,0)
terminal.color(terminal.WHITE)
terminal.background(terminal.BLACK)
terminal.print("hello HELLO\n123 !#@\n\rYO deleted\b\b\ro")

terminal.color(terminal.YELLOW)
terminal.background(0x03c8)
terminal.print("colored text and background")

fbuf = framebuf.FrameBuffer(bytearray(64 * 32 * 2), 64, 32, framebuf.RGB565)
fbuf.rect(0,0,64,16,terminal.framebufColor(0x03c8),True)
fbuf.text('Buffer!', 0, 0, 0xffff)

fbuf.rect(0,16,64,16,terminal.framebufColor(terminal.YELLOW),True)
fbuf.text('Yellow', 0, 16, 0)

terminal.buffer(fbuf,320//2,0,64,32)

# with open("test.bmp","rb") as _f:
# #    img = bmp.parse(_f)
#     img = terminal.parseBMP(_f)
#     terminal.bitmap(320//2,64,img)
