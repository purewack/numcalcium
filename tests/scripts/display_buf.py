import terminal
import framebuf

terminal.console(False)

fbuf = framebuf.FrameBuffer(bytearray(64 * 64 * 2), 64, 64, framebuf.RGB565)

fbuf.fill(0)
fbuf.text('MicroPython!', 0, 0, 0xffff)
fbuf.hline(0, 9, 96, 0xffff)

terminal.cursor(0,0)
terminal.buffer(64,64,fbuf)

gc.collect()
terminal.console(True)
#input()
