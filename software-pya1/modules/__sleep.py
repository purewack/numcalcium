import machine
import esp32

def enterLowPowerSleep():
    u = esp32.ULP_RV()
    u.pause()
    
    machine.Pin.board.SLEEP_REQ.init(machine.Pin.OPEN_DRAIN, hold=True)
    esp32.wake_on_ext0(machine.Pin.board.B_OK, esp32.WAKEUP_ALL_LOW)
    machine.Pin.board.B_OK.init(machine.Pin.IN,pull=None,hold=True)
    machine.deepsleep()
    
def exitLowPowerSleep():
    esp32.wake_on_ext0(None, esp32.WAKEUP_ALL_LOW)
    machine.Pin.board.B_OK.init(machine.Pin.IN,pull=None,hold=False)
    machine.Pin.board.SLEEP_REQ.init(machine.Pin.IN, hold=False)