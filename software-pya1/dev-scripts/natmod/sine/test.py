import math
import time
import esp32
import machine
import board
import nat_sine
import gc

u = esp32.ULP()
u.pause()
u.run_embedded()
u.resume()

lcd = board.LCD()
lcd.clear()
lcd.print("audio test")

keys = board.keys()

b_size = 1024
buffer_a = bytearray(b_size)
cur_buf = 0
need_refil = False
avail = 1000*1000*(b_size/32000)

def buffer_callback(active_buffer):
    global cur_buf, need_refil
    cur_buf = not active_buffer
    need_refil = True
    #print("b",cur_buf)

sdm = board.DAC(buffer_a, buffer_callback, [6,7])  # Start playback
sdm = board.DAC(buffer_a, buffer_callback)  # Start playback
gc.collect()
nat_sine.buffer(buffer_a)
print("ready")

scale = [69,71,72,74,76,77,79,81,83,84,86,88,89,91,93]
def mToF(n):
    return int(pow(2,(n-69)/12)*440)

s = time.time()
while True:
    if(need_refil):
        t = time.ticks_us()
        need_refil = False
        half = 0 if cur_buf else b_size>>1
        nat_sine.process(half)
        dt = time.ticks_us() - t
        
        # if(time.time() - s > 10): break
    
    if(keys == keys.E):
        break
        
    if(keys.isAnyDown()):
        n = keys.getNextDown()
        nat_sine.freq(mToF(scale[n % len(scale)]))
        keys.clearAll()
	lcd.print("key ")
	lcd.print(str(n))
	lcd.print("\n\r")

sdm.deinit()

