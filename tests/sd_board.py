import board
import os

sd = board.SD()
os.mount(sd,'/sd')
print(os.listdir('/sd'))
