import machine
import time
import struct
import random
import mpy

def check(scl=machine.Pin.board.PORT_SCL,sda=machine.Pin.board.PORT_SDA):
    ee = machine.I2C(1,scl=scl,sda=sda)
    devices = ee.scan()
    if not 80 in devices:
        return -1

    header = ee.readfrom_mem(80,0,8)
    size = struct.unpack('>H', header[-2:])[0]
    check = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
    if not size or not header[0:6] == check:
        return -2

    return True

def run(scl=machine.Pin.board.PORT_SCL,sda=machine.Pin.board.PORT_SDA):
    ee = machine.I2C(1,scl=scl,sda=sda)

    with open("sim-cart.mpy",'rb') as f:
        code = f.read()
        
    # time.sleep(0.01)
    # header = ee.readfrom_mem(80,0,8)
    # size = struct.unpack('>H', header[-2:])[0]
    # print("read size",size,header)
    
    # offset = 8
    # code = bytearray()
    # for i in range(1 + size//8):
    #     bank = offset//256
    #     code += ee.readfrom_mem(80 + bank,offset%256,8)
    #     offset += 8

    mpy.from_memory(code)


def burn(data, scl=machine.Pin.board.PORT_SCL,sda=machine.Pin.board.PORT_SDA):
    ee = machine.I2C(1,scl=scl,sda=sda)

    size = len(data)
    
    if(size > 2048):
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
        print("write",bank,offset%256,chunk)
        offset += 8
  
    time.sleep(0.01)

    # Define the header bytes
    header = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
    header.extend(struct.pack('>H', size))
    ee.writeto_mem(80,0,header)
    time.sleep(0.01)