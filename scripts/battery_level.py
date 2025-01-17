import machine
charge_pin = machine.Pin(48, machine.Pin.IN, pull=machine.Pin.PULL_UP)
volt_pin = machine.Pin(15)
volt_adc = machine.ADC(volt_pin,atten=machine.ADC.ATTN_11DB)

volt_calib = 8.01980198

def readBattery():
    r =  volt_adc.read_u16()
    return (8.01980198 * (r/65535))

def batteryLevel():
    return min(100 * readBattery()/4.1,100)

def isCharging():
    return not charge_pin.value()

print(readBattery())
print(batteryLevel())
print(isCharging())
print(8.01980198* (volt_adc.read_u16()/65535))
