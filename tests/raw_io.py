
"""
int b_scan;
int pwm_ratio;

void main(){
	int p_scan = 0;
	int scan_val = 1;
	int pwm_time = 0;

	while(1){
		if(isMainCpuSleeping()){
			if(readWakeButton())
				wakeUp()
			continue;
		}

		//else

		setPinsAsOutputs();
		shiftOut(scan_val)
		setPinsAsInputs();
		p_scan |= (readPinValues() << (scan_val * 3))

		scan_val = (scan_val<<1) & 0xff;

		if(scan_val == 0){
			scan_val = 1;
			b_scan = p_scan;
			p_scan = 0;
		}

		if(pwm_time & 0xff === 0){
			pwm_time |= 255;
			pwmPinFlip()
		}
	}
}

input banking:
each bank has 3 bits
7 banks for 21 total inputs

0b gg fff eee ddd ccc bbb aaa
LSB = button 1
MSB = button N-1
"""

import machine
import time
import esp32

u = esp32.ULP()
u.pause()

pCK = machine.Pin(12)
pD  = machine.Pin(13)
pT  = machine.Pin(14, machine.Pin.IN, machine.Pin.PULL_DOWN)
pOE = machine.Pin(11, machine.Pin.OUT)


def setOuts():
    pCK = machine.Pin(12,machine.Pin.OUT)
    pD =  machine.Pin(13,machine.Pin.OUT)

def setIns():
    pCK = machine.Pin(12, machine.Pin.IN, machine.Pin.PULL_DOWN)
    pD  = machine.Pin(13, machine.Pin.IN, machine.Pin.PULL_DOWN)


def shiftOut(val):
    pCK.off()
    for a in range(8):
        pD.value(val & (1<<a))
        pCK.on()
        pCK.off()
    
def readPinValues():
    return (pT.value() << 2) | (pD.value() << 1) | (pCK.value() << 0)


scan_val = 0
p_scan = 0
print("start")

while True:
    setOuts()

    pOE.off()
    shiftOut(1<<scan_val)
    pOE.on()
    setIns()
    r = readPinValues()
    pOE.off()

    p_scan |= (r << (scan_val * 3))
#    print(scan_val,(scan_val * 3))

    if(scan_val == 7):
        scan_val = 0
        print("scan",p_scan,bin(p_scan))
        p_scan = 0
    else:
        scan_val = (scan_val+1)%8

    time.sleep(0.01)
    
