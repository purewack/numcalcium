
from _testing import Testing

tester = Testing()

##  --  inp
##  bot by_to_y
##  px  px_to_y
##  bpx bpx_to_y

##  --  out
##  bot y_to_by
##  px  y_to_px
#   bpx y_to_bpx


# y_to_px   v
# y_to_bpx  v
# y_to_by   v

# px_to_y   v
# px_to_by 
# px_to_bpx 

# bpx_to_y  v
# bpx_to_by 
# bpx_to_px 

# by_to_y   v
# by_to_px 
# by_to_bpx 



HH = 50
fh = 10
s = 1

def y_to_px(y,fh=fh,s=s):
    return y * fh*s

def y_to_by(y,HH,fh=fh,s=s):
    yH = (HH / fh)/s
    return yH - y - 1

def y_to_bpx(y,HH,fh=fh,s=s):
    return y_to_px(y_to_by(y,HH,fh,s),fh,s)



def px_to_y(px,fh=fh,s=s):
    return px / fh/s

def by_to_y(by,HH,fh=fh,s=s):
    yH = (HH / fh)/s
    return yH - by - 1

def bpx_to_y(bpx,HH,fh=fh,s=s):
    px = HH - bpx
    return px_to_y(px,fh,s)

tester.eq_float(y_to_px(1),10)
tester.eq_float(y_to_px(1,s=2),20)

tester.eq_float(y_to_by(1,HH,s=1),3)
tester.eq_float(y_to_by(0,HH,s=1),4)
tester.eq_float(y_to_by(0,HH,s=2),1.5)

tester.eq_float(y_to_bpx(0,HH,s=1),40)
tester.eq_float(y_to_bpx(4,HH,s=1),0)
tester.eq_float(y_to_bpx(2,HH,s=1),20)
tester.eq_float(y_to_bpx(0,HH,s=2),30)
tester.eq_float(y_to_bpx(1,HH,s=2),10)

# #

tester.eq_float(px_to_y(10,s=1),1)
tester.eq_float(px_to_y(20,s=2),1)
tester.eq_float(px_to_y(15,s=1),1.5)

tester.eq_float(by_to_y(1,HH),3)
tester.eq_float(by_to_y(0,HH),4)
tester.eq_float(by_to_y(0,HH,s=2),1.5)
tester.eq_float(by_to_y(1,HH,s=2),0.5)


# tester.eq_float(bpx_to_y(10),4)
# tester.eq_float(bpx_to_y(30),2)
# tester.eq_float(bpx_to_y(10,s=2),2)