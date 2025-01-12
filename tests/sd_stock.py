import os
import machine

sd = machine.SDCard(slot=2,sck=39,miso=41,mosi=40,cs=38)
fat=os.VfsFat(sd)
os.mount(fat, "/sd") 

print(os.listdir('/sd'))

os.umount(fat)
