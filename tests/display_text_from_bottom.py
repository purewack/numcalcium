import numcalc

l = numcalc.LCD()

def printCheck(txt,where):
    print(txt,where,' cur from top',l.cursor(),l.cursor(pixels=True))
    print(txt,where,' cur from bot',l.cursor(bottom=True),l.cursor(bottom=True,pixels=True))
    print('---')
    l.print(txt)


printCheck('first','top')

l.cursor(0,1)
printCheck('second','top')




l.cursor(0,l.FH,pixels=True,bottom=True)
printCheck('first','bot')


l.cursor(0,2,bottom=True)
printCheck('second','bot')



bb = l.HEIGHT//2
l.fill(l.WIDTH//2,0,50,bb,"#073333")

l.cursor(l.WIDTH//2,l.FH,pixels=True,bottom=bb)

printCheck('first','mid')

l.cursor(20,2,bottom=bb)
printCheck('second','mid')
