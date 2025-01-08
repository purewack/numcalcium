import board
import os

sd = board.SD()
os.mount(sd,'/sd')
os.listdir('/sd')
