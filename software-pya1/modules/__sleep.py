import machine
import esp32
import __ulpio


def readTick():
    print(machine.mem32[__ulpio.data['symbols']['ulp_tick']])

def enterLowPowerSleep():
    # if for some reason ULP is not running, start up the program again.
    u = esp32.ULP_RV()
    u.load_binary(__ulpio.data['binary'])
    u.run()
    
    s = machine.Pin.board.SLEEP_REQ
    s.init(machine.Pin.OPEN_DRAIN, hold=True)
    machine.mem32[__ulpio.data['symbols']['system_sleeping']] = 1
    esp32.wake_on_ulp(True)
    machine.deepsleep()
    
def exitLowPowerSleep():
    s = machine.Pin.board.SLEEP_REQ
    s.init(machine.Pin.IN, hold=False)