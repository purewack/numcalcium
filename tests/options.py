import numcalc
import font_mono22

l = numcalc.LCD()
print(l.options())
l.print('hey')
print(l.cursor(),l.cursor(pixels=True))
l.options(background=l.GREEN,foreground=l.WHITE)

print(l.options())
l.options(font=font_mono22)
print(l.options())
l.print('hey')

print("setting 3")
l.options(font=None,background=l.BLUE)
print(l.options())
l.print('hey')