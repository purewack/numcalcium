import math
import sigma_delta
import time
import esp32
import nat_sine

u = esp32.ULP()
u.run_embedded()

b_size = 1024
buffer_a = bytearray(b_size)
cur_buf = 0
need_refil = False
avail = 1000*1000*(b_size/32000)

def buffer_callback(active_buffer):
    global cur_buf, need_refil
    cur_buf = active_buffer
    need_refil = True
    print("b",cur_buf)

sigma_delta.init(buffer_callback,buffer_a,[40])  # Start playback
nat_sine.buffer(buffer_a)
print("ready")

s = time.time()
while True:
    if(need_refil):
        t = time.ticks_us()
        need_refil = False
        half = 0 if cur_buf else b_size>>1
        nat_sine.process(half)
        dt = time.ticks_us() - t
        
        if(time.time() - s > 10): break
#    time.sleep(0)

sigma_delta.deinit()

