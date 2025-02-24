import math
import sigma_delta
import time
import esp32

u = esp32.ULP()
u.run_embedded()

# Create buffers
f = 440
sf = 32000
b_size = 8192
ms_avail = 1000 * 1000 * (b_size/sf)

phase = 0
buffer_a = bytearray(b_size)
buffer_b = bytearray(b_size)
cur_buf = 0
need_refil = False

def buffer_callback(active_buffer):
    global cur_buf, need_refil
    cur_buf = active_buffer
    need_refil = True
#    print("b",cur_buf)

def refill(f,buf):
    global b_size,sf,phase
    for i in range(b_size): 
        buf[i] = int(127 * 0.5 * math.sin(2 * math.pi * (f / sf) * phase))
        phase += 1

sigma_delta.set_buffers(buffer_a, buffer_b)  # Set the buffers
sigma_delta.set_callback(buffer_callback)  # Register the callback
sigma_delta.begin()  # Start playback
print("ready")

while True:
    if(need_refil):
        need_refil = False
        buf = buffer_b if cur_buf else buffer_a
        refill(f,buf)
    time.sleep(0.005)
    d = u.read(u.VAR_BDOWN)
    if(d):
        print("key",d)
        u.write(u.VAR_BDOWN,0)
        for a in range(20):
            if (1<<a) & d:
                f = 220 + math.pow(2,a/12) * 220
                print("freq",f,a)
