import machine
charge_pin = machine.Pin(48, machine.Pin.IN, pull=machine.Pin.PULL_UP)
volt_pin = machine.Pin(15)
volt_adc = machine.ADC(volt_pin,atten=machine.ADC.ATTN_11DB)

volt_calib = 8.01980198

def readBattery():
    return volt_adc.read_u16()
#    r =  volt_adc.read_u16()
#    return (volt_calib * (r/65535))

#def batteryLevel():
#    return min(100 * readBattery()/4.1,100)
#
#def isCharging():
#    return not charge_pin.value()
#
#print(batteryLevel())
#print(isCharging())
print(readBattery())
print(volt_adc.read_u16())
#print(volt_calib * (volt_adc.read_u16()/65535))
