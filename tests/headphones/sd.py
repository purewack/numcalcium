import math
import time
import esp32
import machine
import board
import os

u = esp32.ULP()
u.pause()
u.run_embedded()
u.resume()

lcd = board.LCD()
lcd.clear()
lcd.print("audio stream test")

keys = board.keys()

b_size = 512
buffer_a = bytearray(b_size)
cur_buf = 0
need_refil = False
avail = 1000*1000*(b_size/32000)

def buffer_callback(active_buffer):
    global cur_buf, need_refil
    cur_buf = not active_buffer
    need_refil = True
    #print("b",cur_buf)

sdm = board.DAC(buffer_a, buffer_callback, [machine.Pin.board.A_OUT_L,machine.Pin.board.A_OUT_R])  # Start playback
print("ready")

s = time.time()
sd = board.SD()
os.mount(sd,'/sd')
with open("/sd/stereo.raw","rb") as f:

    while True:
        if(not need_refil): 
            time.sleep(0.001)
            continue

        t = time.ticks_us()
        need_refil = False
        half = 0 if cur_buf else b_size>>1
       
        chunk = f.read(b_size>>1)

        if not chunk:
            # End of file
            print("end of file")
            break
        
        buffer_a[half:half + (b_size>>1)] = chunk

        dt = time.ticks_us() - t
        print(f"{dt}/{avail}")
    
sdm.deinit()

