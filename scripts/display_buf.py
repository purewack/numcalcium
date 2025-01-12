import terminal
import framebuf

terminal.console(False)

fbuf = framebuf.FrameBuffer(bytearray(64 * 64 * 2), 64, 64, framebuf.RGB565)

fbuf.fill(0)
fbuf.rect(0,0,32,32,terminal.YELLOW,True)
fbuf.rect(32,0,32,32,0x07e0,True)
fbuf.rect(0,32,32,32,0x001f,True)
fbuf.rect(32,32,32,32,0xf800,True)
fbuf.text('MicroPython!', 0, 0, 0xffff)
fbuf.hline(0, 9, 96, 0xffff)

terminal.cursor(0,0)
terminal.buffer(64,64,fbuf)

gc.collect()
terminal.console(True)
#input()
