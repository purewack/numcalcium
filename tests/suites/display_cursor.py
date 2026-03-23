

class LCD():
    def __init__(self):
        self.WIDTH = 320
        self.HEIGHT = 170
        self.FW = 6
        self.FH = 13
        self.FX = self.WIDTH//self.FW
        self.FY = self.HEIGHT//self.FH
        self.scale = 1
        self._cursor = [0,0]
        
    def cursor(self, *args, **kwargs):
        s = self.scale 
        ww = self.WIDTH
        hh = self.HEIGHT
        fh = self.FH
        fw = self.FW
        
        if('bottom' in kwargs and not isinstance(kwargs['bottom'],bool)):
            hh = kwargs['bottom']
            # print('setting region bot')
        
        sfY = hh / (fh*s)
        
        ## GET
        if(not len(args)):
            c = self._getset_cursor()
            x = c[0]
            y = c[1]
            if(kwargs.get('pixels',False)):
                # get from raw to px
                xx = (x*fw*s)
                yy = (y*fh*s)
                if(kwargs.get('bottom',False)):
                    # print('get px bot',xx,yy,hh,x,y)
                    yy = hh - yy + 0.0000001
                return (int(xx),int(yy))
    
            if(kwargs.get('bottom',False)):
                y = sfY - y
            return (float(x),float(y))
        if(not len(args) == 2):
            raise ValueError("Need both x and y positions")
        
        
        ## SET
        x = args[0]
        y = args[1]
        
        if(kwargs.get('pixels',False)):
            # set from px to raw
            ss = 'set-px'
            x = x / fw / s
            y = y / fh / s
            if(kwargs.get('bottom',False)):
                y = sfY - y
                ss += '-bottom'
            return self._getset_cursor(x,y,ss)
        
        bb = 'set-raw'
        if(kwargs.get('bottom',False)):
            bb += '-bot'
            y = sfY - y
        return self._getset_cursor(x,y,bb)
        
    def _getset_cursor(self,*args):
        if(len(args)):
            print('setting',args)
            self._cursor = [args[0],args[1]]
            return
        return (self._cursor[0],self._cursor[1])


from _testing import Testing

tester = Testing()

l = LCD()

l.scale = 1
l.cursor(2,1)
tester.eq_array(l.cursor(),(2.0,1.0),'top scale 1 raw->raw')
tester.eq_array(l.cursor(pixels=True),(12,13),'top scale 1 raw->px')
l.cursor(16,16,pixels=True)
tester.eq_array(l.cursor(pixels=True),(16,16),'top scale 1 px->px')
tester.eq_array_float(l.cursor(),(2.666666667,1.230769231),'top scale 1 px->raw')

l.scale = 2
l.cursor(2,1)
tester.eq_array(l.cursor(),(2.0,1.0),'top scale 2 raw->raw')
tester.eq_array(l.cursor(pixels=True),(24,26),'top scale 2 raw-px')
l.cursor(16,16,pixels=True)
tester.eq_array(l.cursor(pixels=True),(16,16),'top scale 2 px->px')
tester.eq_array_float(l.cursor(),(1.333333334,0.615384615),'top scale 2 px->raw')


l.scale = 1
l.cursor(2,1,bottom=True)
tester.eq_array(l.cursor(bottom=True),(2.0,1.0),'bot scale 1 raw->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.0,12.076923077),'bot scale 1 raw->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(12, 157),'bot scale 1 raw->px ref top')
tester.eq_array(l.cursor(bottom=True,pixels=True),(12, 13),'bot scale 1 raw->px ref bot')
l.cursor(16,16,bottom=True,pixels=True)
tester.eq_array_float(l.cursor(bottom=True),(2.6666,1.230769231),'bot scale 1 px->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.6666,11.846153846),'bot scale 1 px->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(16, 170-16),'bot scale 1 px->px ref top')
tester.eq_array(l.cursor(bottom=True,pixels=True),(16, 16),'bot scale 1 px->px ref bot')



l.scale = 2
l.cursor(2,1,bottom=True)
tester.eq_array(l.cursor(bottom=True),(2.0,1.0),'bot scale 2 raw->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.0, 5.538461538),'asdfasd bot scale 2 raw->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(24, 144),'bot scale 2 raw->px ref top')
tester.eq_array(l.cursor(bottom=True,pixels=True),(24, 26),'bot scale 2 raw->px ref bot')
l.cursor(16,16,bottom=True,pixels=True)
tester.eq_array_float(l.cursor(bottom=True),(1.333333,0.615384616),'bot scale 2 px->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(1.33333,5.923076923),'bot scale 2 px->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(16, 170-16),'bot scale 2 px->px ref top')
tester.eq_array(l.cursor(bottom=True,pixels=True),(16, 16),'bot scale 2 px->px ref bot')




l.scale = 1
bt = 130
l.cursor(2,1,bottom=bt)
tester.eq_array(l.cursor(bottom=bt),(2.0,1.0),'bot REG y=130 scale 1 raw->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.0,9.0),'bot REG y=130 scale 1 raw->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(12, bt-13),'bot REG y=130 scale 1 raw->px ref top')
tester.eq_array(l.cursor(bottom=bt,pixels=True),(12, 13),'bot REG y=130 scale 1 raw->px ref bot')
l.cursor(16,16,bottom=bt,pixels=True)
tester.eq_array_float(l.cursor(bottom=bt),(2.6666, 1.230769231),'bot REG y=130 scale 1 px->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.6666, 8.769230769),'bot REG y=130 scale 1 px->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(16, bt-16),'bot REG y=130 scale 1 px->px ref top')
tester.eq_array(l.cursor(bottom=bt,pixels=True),(16, 16),'bot REG y=130 scale 1 px->px ref bot')

l.scale = 2
bt = 130
l.cursor(2,1,bottom=bt)
tester.eq_array(l.cursor(bottom=bt),(2.0,1.0),'bot REG y=130 scale 1 raw->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(2.0,4.0),'bot REG y=130 scale 1 raw->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(24, bt-26),'bot REG y=130 scale 1 raw->px ref top')
tester.eq_array(l.cursor(bottom=bt,pixels=True),(24, 26),'bot REG y=130 scale 1 raw->px ref bot')
l.cursor(16,16,bottom=bt,pixels=True)
tester.eq_array_float(l.cursor(bottom=bt),(1.33333, 0.615384616),'bot REG y=130 scale 1 px->raw ref bot')
tester.eq_array_float(l.cursor(bottom=False),(1.3333, 4.384615384),'bot REG y=130 scale 1 px->raw ref top')
tester.eq_array(l.cursor(bottom=False,pixels=True),(16, bt-16),'bot REG y=130 scale 1 px->px ref top')
tester.eq_array(l.cursor(bottom=bt,pixels=True),(16, 16),'bot REG y=130 scale 1 px->px ref bot')
