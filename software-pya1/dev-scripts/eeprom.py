import machine
import time
import struct

ee = machine.I2C(1,scl=1,sda=2)
ee.scan()

def _check_ready():
    time.sleep(0.01)

def burn(data):
    size = len(data)
    
    if(size > 2048):
        raise Exception('data too large')    

    banks = int(size/256) + 1
    offset = 8

    for i in range(1 + len(data)//8):
        chunk = data[i*8:i*8 + 8]
        bank = offset//256
        ee.writeto_mem(80 + bank, offset%256, chunk)
#        print("write",bank,offset%256,chunk)
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

#b = bytearray("basd" * 500,'utf-8')
#with open('hello.mpy','rb') as f:
#    data = f.read()
#    print(data)
#    burn(data)
#
code = read()

import mpy
mpy.exec_buffer(code)

