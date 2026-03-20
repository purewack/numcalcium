import numcalc

l = numcalc.LCD()
l.print('test default font')

import font_mono22

l.options(font=font_mono22)

l.cursor(0,3)
l.print('Testing Mono22')
l.scale(2)
l.cursor(0,2)
l.print('Testing\x1d')

#mem leak test

l.scale(1)
l.options(autoWrap=False)
hue = 0
while True:
    
    l.options(font=None,foreground=hue)
    l.cursor(0,32,pixels=True)
    l.print('Font load INT')
    
    l.options(font=font_mono22,foreground=hue)
    l.cursor(l.WIDTH//2,32,pixels=True)
    l.print('Font load EXT')
    
    hue += 2
    hue &= 0xffff