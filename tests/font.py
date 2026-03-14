import numcalc

l = numcalc.LCD()
l.print('test default font')

import font_mono22

l.font(font_mono22.data)

l.cursor(0,3)
l.print('Testing Mono22')
l.scale(2)
l.cursor(0,2)
l.print('Testing\x1d')