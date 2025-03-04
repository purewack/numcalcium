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

def drawBuf(buf,dt,tick):
    buf.fill(0)
    buf.rect(0,0,xx,yy,random.randint(0,0xffff),True)
    txt = f'{dt} '
    buf.text(txt, 0, 0, 0xffff)
    txt = f'{1/(dt/1000)} fps'
    buf.text(txt, 0, 8, 0xffff)
    txt = f'{tick}'
    buf.text(txt, 0, 16, 0xffff)
    buf.text('Numcalcium', 0, 32, 0xffff)
    buf.hline(0, 40, 96, 0xffff)

bufs = (fbuf,fbuf2)
current = 0

tick = 0
dt = 1
ps = 1
while True:
    print("pre dt")
    ds = time.ticks_ms()
    ps = time.ticks_ms()
    print("post dt")
    buf = bufs[1] if current else bufs[0]
    drawBuf(buf,dt,tick)
    print("====================")
    us = time.ticks_us()
    terminal.fill(0,0,50,50,0x58b2)
    terminal.buffer(buf,0,0,xx,yy)
    usdt = time.ticks_us() - us
    print("-------","took ms:",dt,usdt)
    current = 1 if current == 0 else 0
    tick += 1
    ps = time.ticks_ms() - ps
    vt = (1/30) - (ps/1000)
    print("pre sleep")
    # time.sleep(vt)
    print("post sleep")

    dt = time.ticks_ms() - ds
