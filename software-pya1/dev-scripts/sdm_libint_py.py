import math
import sigma_delta
import time
import esp32

u = esp32.ULP()
u.run_embedded()

b_size = 512
buffer_a = bytearray(b_size)
buffer_b = bytearray(b_size)
cur_buf = 0
need_refil = False

def buffer_callback(active_buffer):
    global cur_buf, need_refil
    cur_buf = active_buffer
    need_refil = True
#    print("b",cur_buf)

sigma_delta.set_buffers(buffer_a, buffer_b)  # Set the buffers
sigma_delta.set_callback(buffer_callback)  # Register the callback
sigma_delta.begin()  # Start playback
print("ready")

LUT_COUNT = 256
phi = 0
bias = 0
acc = 900
gain = 255
sint = []
for i in range(LUT_COUNT):
    sint.append(int(32766 * math.sin(2 * 3.14159265359 * (i/LUT_COUNT))))

def set_osc_freq(f_big, srate):
  global acc
  a = LUT_COUNT*256*f_big;
  acc = a / srate / 10;


while True:
    if(need_refil):
        t = time.ticks_ms()
        need_refil = False
        buf = buffer_b if cur_buf else buffer_a
        
        for i in range(b_size):

            phi_dt = phi & 0xFF;
            phi_l = phi >> 8;
            out_spl = sint[phi_l & 0xff];
            buf[i] = int(out_spl/256/2)

            phi += acc;
        
        dt = time.ticks_ms() - t
        print("time: ",dt)

#    time.sleep(0.005)
   
