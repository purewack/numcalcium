from micropython import const
import time
import board
import neopixel
import machine
import usb.device
from usb.device.keyboard import KeyboardInterface as USBHID
from hid_services import Keyboard as BLEHID
import keys


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

    def start(self):
        if(self.mode == 'ble'):
            self.ble.start()
        else:
             usb.device.get().init(self.usb, builtin_driver=True)
        self.readyCheck()

    def switchMappings(self, to):
        if to == 'numpad':
            self._map = self._keymap_numpad
        elif to == 'arrows':
            self._map = self._keymap_arrows

    def scan(self):
        if self.scanner.isAnyDown():
            i = self.scanner.getFirstDown()
            if(i == self.scanner.KEY_F1):
                self.switchMappings('numpad')
            elif(i == self.scanner.KEY_F2):
                self.switchMappings('arrows')
            elif(i == self.scanner.KEY_F3):
                self.switchMappings('custom')
            else:
                self.down_key(i)

        elif self.scanner.isAnyUp():
            i = self.scanner.getFirstUp()
            self.up_key(i)

        time.sleep_ms(1)

    def down_key(self,k):
        key = self._map[k]
        if not key: 
            return
        if(self.mode == 'ble'):
            self.ble.set_keys(key)
            self.ble.notify_hid_report()
        else:
            self.usb.send_keys([key])
        neo[0] = (10,10,10)
        neo.write()

    def up_key(self,k):
        key = self._map[k]
        if not key:
            return
        if(self.mode == 'ble'):
            self.ble.set_keys()
            self.ble.notify_hid_report()
            neo[0] = (0,0,5)
        else:
            self.usb.send_keys([])
            neo[0] = (0,0,0)
        neo.write()


    def readyCheck(self): 
        if(self.mode == 'usb'):
            if not self.usb.is_open:
                usb.device.get().init(self.usb, builtin_driver=True)
                while not self.is_open:
                    time.sleep(1)
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
                time.sleep(0.25)
                neo[0] = (10,10,0)
                neo.write()
                time.sleep(0.25)
        if self.ble.get_state() is BLEHID.DEVICE_ADVERTISING:
            self.ble.stop_advertising()
        neo[0] = (0,0,5)
        neo.write()

k = KeypadDriver('usb')
k.start()
k.readyCheck()
while True:
    k.readyCheck()
    k.scan()
