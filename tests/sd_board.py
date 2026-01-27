import numcalc
import os

sd = numcalc.SD()
os.mount(sd,'/sd')
print(os.listdir('/sd'))
