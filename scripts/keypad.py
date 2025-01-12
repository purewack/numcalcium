from micropython import const
import time
import esp32
import usb.device
import terminal
import neopixel
import machine
from usb.device.hid import HIDInterface

_INTERFACE_PROTOCOL_KEYBOARD = const(0x01)

u = esp32.ULP()
u.run_embedded()

_keymap = [
    0,
    0x27, # 0 
    0,
    0x28,

    0x1e, # 1
    0x1f, # 2
    0x20, # 3
    0,

    0x21,
    0x22,
    0x23,
    0,

    0x24,
    0x25,
    0x26,
    0,

    0,
    0,
    0,
    0x2a
]

terminal.clear()
terminal.options(scale=3)
terminal.print("USB: Keypad")

neo = neopixel.NeoPixel(machine.Pin(47),21)
neo.fill((0,0,0))
neo.write()

def keypad_scanner():
    k = KeypadInterface()

    usb.device.get().init(k, builtin_driver=True)
    
    while not k.is_open():
        time.sleep_ms(100)
    
    while True:
        down = u.read(u.VAR_BDOWN)
        up = u.read(u.VAR_BUP)
        if(down):
            for i in range(20):
                if((1<<i) & down):
                    k.send_key(i)
                    neo[i] = (20,20,20)
                    neo.write()
            u.write(u.VAR_BDOWN,0)
        if(up):
            k.send_key()
            for i in range(20):
                if((1<<i) & up):
                    neo[i] = (0,0,0)
                    neo.write()
            u.write(u.VAR_BUP,0)

        time.sleep_ms(10)

class KeypadInterface(HIDInterface):
    # Very basic synchronous USB keypad HID interface

    def __init__(self):
        super().__init__(
            _KEYPAD_REPORT_DESC,
            set_report_buf=bytearray(1),
            protocol=_INTERFACE_PROTOCOL_KEYBOARD,
            interface_str="NumCalcium Keypad",
        )
        self.numlock = False

    def on_set_report(self, report_data, _report_id, _report_type):
        report = report_data[0]
        b = bool(report & 1)
        if b != self.numlock:
            print("Numlock: ", b)
            self.numlock = b

    def send_key(self, key=None):
        if key is None:
            self.send_report(b"\x00")
        else:
            self.send_report(_keymap[key].to_bytes(1, "big"))


#
# HID Report descriptor for a numeric keypad
#
fmt: off
_KEYPAD_REPORT_DESC = (
    b'\x05\x01'  # Usage Page (Generic Desktop)
        b'\x09\x07'  # Usage (Keypad)
    b'\xA1\x01'  # Collection (Application)
        b'\x05\x07'  # Usage Page (Keypad)
            b'\x19\x00'  # Usage Minimum (0)
            b'\x29\xFF'  # Usage Maximum (ff)
            b'\x15\x00'  # Logical Minimum (0)
            b'\x25\xFF'  # Logical Maximum (ff)
            b'\x95\x01'  # Report Count (1),
            b'\x75\x08'  # Report Size (8),
            b'\x81\x00'  # Input (Data, Array, Absolute)
        b'\x05\x08'  # Usage page (LEDs)
            b'\x19\x01'  # Usage Minimum (1)
            b'\x29\x01'  # Usage Maximum (1),
            b'\x95\x01'  # Report Count (1),
            b'\x75\x01'  # Report Size (1),
            b'\x91\x02'  # Output (Data, Variable, Absolute)
            b'\x95\x01'  # Report Count (1),
            b'\x75\x07'  # Report Size (7),
            b'\x91\x01'  # Output (Constant) - padding bits
    b'\xC0'  # End Collection
)
fmt: on


keypad_scanner()

