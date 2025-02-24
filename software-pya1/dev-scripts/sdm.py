import math
import sigma_delta
import time
import esp32

u = esp32.ULP()
u.run_embedded()

# Create buffers
a = 0.05
f = 440
sf = 32000
b_size = 256
buffer_a = bytearray(b_size)
buffer_b = bytearray(b_size)

for i in range(b_size): 
    buffer_a[i] = int(127 * a * math.sin(2 * math.pi * (i/b_size)))
    buffer_b[i] = buffer_a[i]

def buffer_callback(active_buffer):
    pass

sigma_delta.set_buffers(buffer_a, buffer_b)  # Set the buffers
sigma_delta.set_callback(buffer_callback)  # Register the callback
sigma_delta.begin()  # Start playback
