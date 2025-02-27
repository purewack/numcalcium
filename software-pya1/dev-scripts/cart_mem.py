import machine
import time
import struct
import random

ee = machine.I2C(1,scl=1,sda=2)
print(ee.scan())

def _check_ready():
    time.sleep(0.01)

def burn(data):
    size = len(data)
    
    if(size > 2048):
        raise Exception('data too large')
    
    key = bytearray([random.randint(93, 126) for _ in range(8)])
    ee.writeto_mem(80,0,key)
    _check_ready()
    key_check = ee.readfrom_mem(80,0,8)
    if(not key_check == key):
        raise Exception('WP is possibly set, cannot write')

    banks = int(size/256) + 1
    offset = 8

    for i in range(1 + len(data)//8):
        chunk = data[i*8:i*8 + 8]
        bank = offset//256
        ee.writeto_mem(80 + bank, offset%256, chunk)
        print("write",bank,offset%256,chunk)
        offset += 8

        _check_ready()

    # Define the header bytes
    header = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
    header.extend(struct.pack('>H', size))
    ee.writeto_mem(80,0,header)
    _check_ready()

def read():
    _check_ready()
    header = ee.readfrom_mem(80,0,8)
    size = struct.unpack('>H', header[-2:])[0]
    print("read size",size,header)
    
    offset = 8
    data = bytearray()
    for i in range(1 + size//8):
        bank = offset//256
        data += ee.readfrom_mem(80 + bank,offset%256,8)
        offset += 8

    return data

#clear = bytearray([0xff] * 2048)
#burn(clear)

with open('natmod/_hello.mpy','rb') as f:
    data = f.read()
    print(data)
    burn(data)

code = read()
print(code)

import mpy
mpy.from_memory(code)
