import framebuf
import time
import random 

import _board
_board.init()
import board

terminal = board.LCD()

xx = 320
yy = 170
fbuf  = framebuf.FrameBuffer(bytearray(xx * yy * 2), xx, yy, framebuf.RGB565)
fbuf2 = framebuf.FrameBuffer(bytearray(xx * yy * 2), xx, yy, framebuf.RGB565)

bufs = (fbuf,fbuf2)
current = 0

tick = 0
dt = 1
ps = 1
while True:
    ds = time.ticks_ms()
    ps = time.ticks_ms()
    buf = bufs[1] if current else bufs[0]
    buf.fill(0)
    buf.rect(0,0,xx,yy,random.randint(0,0xffff),True)
    txt = f'{dt} {1/(dt/1000)} fps'
    buf.text(txt, 0, 0, 0xffff)
    txt = f'{tick}'
    buf.text(txt, 0, 8, 0xffff)
    buf.text('Numcalcium', 0, 16, 0xffff)
    buf.hline(0, 7, 96, 0xffff)
    print("draw try",buf)
    terminal.buffer(buf,0,0,xx,yy)
    print("draw end")
    current = 1 if current == 0 else 0
    tick += 1
    print("took ms:",dt)
    ps = time.ticks_ms() - ps
    vt = (1/30) - (ps/1000)
    time.sleep(vt)

    dt = time.ticks_ms() - ds
