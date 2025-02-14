from micropython import const
import asyncio
import framebuf
import random
import time
import board
import neopixel
import machine
import keys
import _thread
import usb.device
from usb.device.keyboard import KeyboardInterface as USBHID
from hid_services import Keyboard as BLEHID
from lib.microbmp import MicroBMP

lock = _thread.allocate_lock()  # Mutex for thread-safe access
key_presses = 0

neo = neopixel.NeoPixel(machine.Pin(47),1)
neo.fill((0,0,0))
neo.write()

class KeypadDriver():
    _keymap_numpad = [ 
        0,
        0x27, # 0 
        0x63, # .
        0x58, # enter

        0x1e, # 1
        0x1f, # 2
        0x20, # 3
        0,

        0x21, # 4
        0x22, # 5
        0x23, # 6
        0,

        0x24, # 7
        0x25, # 8
        0x26, #'9
        0,

        0,
        0,
        0,
        0x2a  # backspace
    ]

    _keymap_arrows = [
        0,
        0, # 0 
        0,
        0,

        0x50,
        0x51,
        0x4f,
        0x58,

        0x2a,
        0x52,
        0x4c,
        0,

        0,
        0,
        0,
        0,

        0,
        0,
        0,
        0
    ]

    def __init__(self, mode='ble'):
        self.switchMappings('numpad')
        self.scanner = keys.Keys()
        self.usb = USBHID()
        self.ble = BLEHID('NumCalcium Numpad')
        self.mode = mode
        self.down_keys = [0,0,0,0,0,0]

    def start(self):
        if(self.mode == 'ble'):
            self.ble.start()
        else:
             usb.device.get().init(self.usb, builtin_driver=True)

    def switchMappings(self, to):
        if to == 'numpad':
            self._map = self._keymap_numpad
        elif to == 'arrows':
            self._map = self._keymap_arrows
        self.mapping = to

    def scan(self):
        if self.scanner.isAnyDown():
            i = self.scanner.getFirstDown()
            self.down_key(i)

        elif self.scanner.isAnyUp():
            i = self.scanner.getFirstUp()
            self.up_key(i)

    def down_key(self,k):
        
        global key_presses,lock
        with lock:
            key_presses += 1
        if(k == self.scanner.KEY_F1):
            self.switchMappings('numpad')
        elif(k == self.scanner.KEY_F2):
            self.switchMappings('arrows')
        elif(k == self.scanner.KEY_F3):
            self.switchMappings('custom')
        else:
            key = self._map[k]
            if not key: 
                return

            for i in range(6):
                if self.down_keys[i] == 0:
                    self.down_keys[i] = key
                    break
            if(self.mode == 'ble'):
                self.ble.keypresses = self.down_keys
                self.ble.notify_hid_report()
            else:
                self.usb.send_keys(self.down_keys)
            neo.write()

    def up_key(self,k):
        key = self._map[k]
        if not key:
            return

        global key_presses,lock
        with lock:
            key_presses += 1

        for i in range(6):
            if self.down_keys[i] == key:
                self.down_keys[i] = 0
                break
        if(self.mode == 'ble'):
            self.ble.keypresses = self.down_keys
            self.ble.notify_hid_report()
        else:
            self.usb.send_keys(self.down_keys)
        neo.write()


    def readyCheck(self): 
        if(self.mode == 'usb'):
            if not self.usb.is_open:
                usb.device.get().init(self.usb, builtin_driver=True)
                while not self.is_open:
                    time.sleep(0.5)
                neo[0] = (0,5,0)
                neo.write()
            return

        if self.ble.get_state() is BLEHID.DEVICE_CONNECTED:
            neo[0] = (0,0,5)
            neo.write()
            return

        self.ble.start_advertising()
        while True:
            if self.ble.get_state() is BLEHID.DEVICE_CONNECTED:
                break
            if(self.ble.get_state() is BLEHID.DEVICE_ADVERTISING):
                neo[0] = (10,0,10)
                neo.write()
                time.sleep(0.5)
                neo[0] = (10,10,0)
                neo.write()
                time.sleep(0.5)
        if self.ble.get_state() is BLEHID.DEVICE_ADVERTISING:
            self.ble.stop_advertising()
        neo[0] = (0,0,5)
        neo.write()

k = KeypadDriver('ble')
def keyboard():
    global k
    k.start()
    k.readyCheck()
    while True:
        k.readyCheck()
        k.scan()
        time.sleep(0.001) 

def loadFrame(path, scale = 1):
    img = MicroBMP().load(path)
    frame = framebuf.FrameBuffer(bytearray(100 * 100 * 2), 100, 100, framebuf.RGB565)
    x=0
    y=0
    for y in range(100):
        for x in range(100):
            px = x + y*100
            p = img[x,y]
            c = img.palette[p]
            r = c[0] 
            g = c[1] 
            b = c[2] 
            rgb565 = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3)
            irgb = ((rgb565 & 0xff)<<8) | (rgb565>>8)
            
            for sy in range(scale):
                for sx in range(scale):
                    frame.pixel((x*scale)+sx,(y*scale)+sy,irgb)
    return frame

def screen():
    lcd = board.LCD()
    lcd.clear()
    
    global frames, k, key_presses, lock
    frame_current = 0

    shade = lcd.WHITE
    lcd.fill(0,0,320,170,shade)
    
    last_key_presses = -1
    _key_presses = 0

    while True:
        
        time.sleep(0.05)
        with lock:
            _key_presses = key_presses

        if last_key_presses == key_presses:
            continue

        last_key_presses = key_presses
        frame = frames[frame_current]
        frame_current = (frame_current + 1) % len(frames)
        lcd.buffer(frame, 320//2 - 50, 170//2 - 70, 100, 100)
        lcd.options(scale=2, background=lcd.WHITE, foreground=lcd.BLACK)
        lcd.cursor(7,4)
        lcd.print(k.mapping)

frames = [loadFrame('bongo1_.bmp'),loadFrame('bongo2_.bmp')]

_thread.start_new_thread(screen, ())

keyboard()
