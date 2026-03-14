import numcalc

l = numcalc.LCD()
l.clear()
l.print('hex code test:\x1f\x11\x00 string after')
l.cursor(0,3)
l.scale(2)
l.print('hex code test:\x1f\xfe\x00 string after')