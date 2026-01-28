import machine
import time
import struct
import random
import mpy

def __wait_for_i2c_ready(i2c, addr, timeout_ms=1000):
    """Poll I2C device until it acknowledges (device ready)"""
    start = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), start) < timeout_ms:
        try:
            i2c.readfrom(addr, 1)  # Dummy read
            return True
        except OSError:
            time.sleep(0.005)
            pass  # Device still busy
    return False

def check(flash="24CXX",i2c=None):
    if(not flash in ["24CXX"]):
        raise Exception('Unsupported flash memory type specified')
    
    ee = i2c if i2c else machine.I2C()
    devices = ee.scan()
    if not 80 in devices:
        return -1

    header = ee.readfrom_mem(80,0,8)
    size = struct.unpack('>H', header[-2:])[0]
    check = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
    if not size or not header[0:6] == check:
        return -2

    return True

def run(flash="24CXX",i2c=None):
    if(not flash in ["24CXX"]):
        raise Exception('Unsupported flash memory type specified')
    
    ee = i2c if i2c else machine.I2C()
    time.sleep(0.01)
    header = ee.readfrom_mem(80,0,8)
    size = struct.unpack('>H', header[-2:])[0]
    print("program size",size)
    
    offset = 8
    code = bytearray()
    for i in range(1 + size//8):
        bank = offset//256
        code += ee.readfrom_mem(80 + bank,offset%256,8)
        offset += 8

    mpy.from_memory(code)

def burn(data, flash="24CXX", i2c=None):
    if(not flash in ["24CXX"]):
        raise Exception('Unsupported flash memory type specified')
    ee = i2c if i2c else machine.I2C()

    if(type(data) is str):
        with open(data,'rb') as f:
            data = f.read()
    
    size = len(data)
    if(size > 2048 - 8):
        raise Exception('data too large')
    
    key = bytearray([random.randint(93, 126) for _ in range(8)])
    ee.writeto_mem(80,0,key)
    time.sleep(0.01)
    key_check = ee.readfrom_mem(80,0,8)
    if(not key_check == key):
        raise Exception('WP is possibly set, cannot write')

    banks = int(size/256) + 1
    offset = 8

    for i in range(1 + len(data)//8):
        chunk = data[i*8:i*8 + 8]
        bank = offset//256
        ee.writeto_mem(80 + bank, offset%256, chunk)
        __wait_for_i2c_ready(ee, 80)
        print(f"burning: {100 * (offset-8)//size}%")
        offset += 8
  
    time.sleep(0.01)

    # Define the header bytes
    header = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
    header.extend(struct.pack('>H', size))
    ee.writeto_mem(80,0,header)
    time.sleep(0.01)
    print(f"done")
