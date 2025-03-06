import board
import machine
import time

vbat = machine.ADC(machine.Pin.board.VBAT_MON)
chrg = machine.Pin.board.CHR_STATE
chrg.init(machine.Pin.IN,pull=machine.Pin.PULL_UP)
lcd = board.LCD()

def dischargeCycle():
    for i in range(20):
        time.sleep(0.01)
        board.led(i,200,200,200)
    for i in range(20):
        time.sleep(0.01)
        board.led(i,0,0,0)

discharing = False

while True:
    if(board.keys.isAnyDown()):
        board.keys.clearAll()
        discharing = not discharing

    if discharing:
        dischargeCycle()

    lcd.clear()
    lcd.print("VBAT",vbat.read_uv()*2/1000/1000)
    lcd.print("Charging",not chrg.value())
    time.sleep(0.1)