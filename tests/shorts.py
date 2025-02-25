from machine import Pin
from collections import OrderedDict

def traverse_ordereddict(data):
    keys = list(data.keys())
    for i in range(len(keys)):
        prev_key = keys[i - 1] if i > 0 else None
        curr_key = keys[i]
        next_key = keys[i + 1] if i < len(keys) - 1 else None

        prev_value = data[prev_key] if prev_key else None
        curr_value = data[curr_key]
        next_value = data[next_key] if next_key else None

        yield (prev_key, prev_value), (curr_key, curr_value), (next_key, next_value)

def railShorts(pins):
    for curr_key in pins.keys():
        testPin = pins[curr_key]

        testPin.init(Pin.OUT,value=1)
        if(not testPin.value()):
            print("\033[31m","Rails:",curr_key,"\t- can't set high, possibly shorted to GND",'\033[0m')
        else:
            print("Rails:",curr_key,"\t\033[32m","GND OK",'\033[0m')

        testPin.init(Pin.OUT,value=0)
        if(testPin.value()):
            print("\033[31m","Rails:",curr_key,"\t - can't set low, possibly shorted to VDD",'\033[0m')
        else:
            print("Rails:",curr_key,"\t\033[32m","VDD OK",'\033[0m')

        testPin.init(Pin.IN)


def adjacentShorts(pins):
    for prev_item, curr_item, next_item in traverse_ordereddict(pins):
        prev_key, beforePin = prev_item
        curr_key, testPin = curr_item
        
        if(prev_key == None): continue
        print(f"Adjacent: [{curr_key}] \t {prev_key}: \t", end='')
        
        testPin.init(Pin.OUT,value=1)
        beforePin.init(Pin.IN,pull=Pin.PULL_DOWN)
        testBeforeOn = testPin.value() and not beforePin.value()

        testPin.init(Pin.OUT,value=0)
        beforePin.init(Pin.IN,pull=Pin.PULL_DOWN)
        testBeforeOff = not testPin.value() and not beforePin.value()

        if not testBeforeOn == testBeforeOff:
            print("\033[33m",curr_key,"shorted with",prev_key, " :", testBeforeOn, testBeforeOff,'\033[0m')
        else:
            print("\033[32m","ok",'\033[0m')

connectorJ1Pins = OrderedDict()
connectorJ1Pins['lcd_cs'] = Pin.board.LCD_CS
connectorJ1Pins['MISO'] = Pin.board.MISO
connectorJ1Pins['MOSI'] = Pin.board.MOSI
connectorJ1Pins['CK'] = Pin.board.CK
connectorJ1Pins['sd_cs'] = Pin.board.SD_CS
connectorJ1Pins['lcd_dc'] = Pin.board.LCD_DC
connectorJ1Pins['lcd_led'] = Pin.board.LCD_LED
connectorJ1Pins['buz_drive'] = Pin.board.BUZZER
connectorJ1Pins['status_led'] = Pin.board.LEDS
connectorJ1Pins['aud_l'] = Pin.board.A_OUT_L
connectorJ1Pins['aud_r'] = Pin.board.A_OUT_R
print("-----Connector J1 Test-----")
adjacentShorts(connectorJ1Pins)
railShorts(connectorJ1Pins)
