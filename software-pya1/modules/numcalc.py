import _board
import __keys 
import __sleep
import __cartridge
import machine
import neopixel
import os
import sys
import time
import _thread
import __numcalcium_version

NUMCALC_VER = __numcalcium_version._version
NUMCALC_BUILD = __numcalcium_version._build

_board.init()
__sleep.exitLowPowerSleep()

def shutdown():
    clearLeds()
    __sleep.enterLowPowerSleep()

def runFromCartridge():
    __cartridge.run()

def burnToCartridge(data):
    __cartridge.burn(data)
    if(not __cartridge.check() == True):
        raise Exception('Error during burning, check wiring and chip')

class Keys(__keys.Keys):
    pass

class SD(_board.SD):
    pass

class DAC(_board.DAC):
    pass

class LCD(_board.Terminal):
    RED    = 0xf800
    GREEN  = 0x07e0
    BLUE   = 0x001f
    PURPLE = 0xf81f
    YELLOW = 0xffe0
    CYAN   = 0x07ff
    BLACK  = 0x0000
    WHITE  = 0xffff
    GRAY   = 0x8410
    
    WIDTH  = 320
    HEIGHT = 170
    
    def __init__(self):
        super().__init__()
        if not hasattr(self,'__lock'):
            self.__lock = _thread.allocate_lock()
        self._bl = None
        self.XN = self._CHARS_X
        self.YN = self._CHARS_Y
        self.FW = self.FONT_W
        self.FH = self.FONT_H
        self.FN = self.FONT_NAME
        self.current_font = None
        self.canvas = None
        self.reset()
        self.setBacklight(127)

    def __del__(self):
        self.setBacklight(0)

    def _sendcmd(self):
        pass

    def _senddata(self):
        pass

    def reset(self):
        self.options(font=None,canvas=None,background=self.BLACK,foreground=self.WHITE,scale=1)
        self.clear()

    def clear(self):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().clear()
        self.cursor(0,0)

    def fill(self, x,y,w,h,color):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        if isinstance(color, str):
            super().fill(x,y,w,h,self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().fill(x,y,w,h,self.rgbTo565(*color))
        else:
            super().fill(x,y,w,h,color)

    def plot(self, x,y,color):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        if isinstance(color, str):
            super().plot(x,y,self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().plot(x,y,self.rgbTo565(*color))
        else:
            super().plot(x,y,color)

    def line(self, x,y,x2,y2,color):
        dy = (y2-y)
        dx = (x2-x)
        nx = dx*-1 if dx < 0 else dx
        ny = dy*-1 if dy < 0 else dy

        if(nx > ny):
            if(dx < 0):
                for i in range(dx,0,-1):
                    yy = ((i)*dy)//dx
                    self.plot((i+x),yy+y,color)
            else:
                for i in range(dx):
                    yy = ((i)*dy)//dx
                    self.plot((i+x),yy+y,color)
            
        else:
            if(dy < 0):
                for i in range(dy,0,-1):
                    xx = ((i)*dx)//dy
                    self.plot(xx+x,(i+y),color)
            else:
                for i in range(dy):
                    xx = ((i)*dx)//dy
                    self.plot(xx+x,(i+y),color)
    
    def createCanvas(self):
        if(self.canvas):
            self.options(canvas=None)
        self.canvas = bytearray(self.WIDTH*self.HEIGHT*2)
        self.options(canvas=self.canvas)
        return self.canvas
    
    def update(self,*args):
        if(self.canvas):
            if(len(args) == 4):
                x,y,w,h = args
                if(x >= self.WIDTH): return
                if(y >= self.HEIGHT): return
                if(x+w > self.WIDTH):
                    w -= x
                if(y+h > self.HEIGHT):
                    h -= y
                super().buffer(self.canvas,x,y,w,h,True)
            else:
                super().buffer(self.canvas,0,0,self.WIDTH,self.HEIGHT)
    
    def buffer(self,buf,x,y,width,height):
        if not self.__lock.acquire(False): return
        super().buffer(buf,x,y,width,height)
        self.__lock.release()

    def bitmap(self,x,y,image):
        if not self.__lock.acquire(False): return
        super().buffer(image['buffer'],x,y,image['width'],image['height'])
        self.__lock.release()
    
    def print(self, *args):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().print(*args)

    def cursor(self, *args, **kwargs):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        s = super().options()['scale'] 
        ww = self.WIDTH
        hh = self.HEIGHT
        fh = self.FH
        fw = self.FW
        
        if(not len(args)):
            c = super().cursor()
            if(kwargs.get('pixels',False)):
                xx = (c[0]*fw*s)
                yy = (c[1]*fh*s)
                # if(kwargs.get('bottom',False)):
                #     yy = ((hh-c[1]-(fh*s))/hh)*((hh/fh)/s)
                return [xx,yy]
            return c
        if(not len(args) == 2):
            raise ValueError("Need both x and y positions")
        
        x = args[0]
        y = args[1]
        
        if(kwargs.get('pixels',False)):
            xx = (x/ww)*((ww/fw)/s)
            yy = (y/hh)*((hh/fh)/s)
            if(kwargs.get('bottom',False)):
                yy = ((hh-y-(fh*s))/hh)*((hh/fh)/s)
            return super().cursor(xx,yy)
        else:
            if(kwargs.get('bottom',False)):
                y = (hh/(fh*s)) - (y+1)
            return super().cursor(x,y)
        
    def scale(self, scale=None):
        if scale == None:
            return super().options()['scale']

        if not self.__lock.acquire(False): return
        self.__lock.release()
        currentCursor = self.cursor()
        super().options(scale=scale)
        
    def measure_text(self, text, pixels=False):
        s = self.scale()
        fh = self.FONT_H
        fw = self.FONT_W
        text = text.strip('\r').split('\n')
        cx = max(len(t) for t in text)
        cy = len(text)
        return (cx*fw*s,cy*fh*s) if pixels else (cx,cy)
    
    def background(self, color=None):
        if color == None:
            return super().options()['background']

        if not self.__lock.acquire(False): return
        self.__lock.release()
        if isinstance(color, str):
            super().options(background=self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().options(background=self.rgbTo565(*color))
        else:
            super().options(background=color)

    def foreground(self, color=None):
        if color == None:
            return super().options()['foreground']

        if not self.__lock.acquire(False): return
        self.__lock.release()
        if isinstance(color, str):
            super().options(foreground=self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().options(foreground=self.rgbTo565(*color))
        else:
            super().options(foreground=color)
    
    def color(self, color=None):
        return self.foreground(color)

    def bg(self, color=None):
        return self.background(color)

    def invert(self, state):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().options(invert=state)
    
    def options(self, **kwargs):
        if not self.__lock.acquire(False): return
        self.__lock.release()

        if('foreground' in kwargs):
            if isinstance(kwargs['foreground'], str):
                kwargs['foreground'] = self.htmlTo565(kwargs['foreground'])
            elif isinstance(kwargs['foreground'], tuple):
                kwargs['foreground'] = self.rgbTo565(*kwargs['foreground'])

        if('background' in kwargs):
            if isinstance(kwargs['background'], str):
                kwargs['background'] = self.htmlTo565(kwargs['background'])
            elif isinstance(kwargs['background'], tuple):
                kwargs['background'] = self.rgbTo565(*kwargs['background'])
        
        if('canvas' in kwargs):
            self.canvas = kwargs['canvas']
            kwargs.pop('canvas')
            if(self.canvas == None):
                super()._unset_canvas()
            else:
                super()._set_canvas(self.canvas)
            
        if('font' in kwargs):
            cur = self.cursor(pixels=True)
            font = kwargs['font']
            kwargs.pop('font')
            if(font == None):
                super().unloadFont()
                self.FW = self.FONT_W
                self.FH = self.FONT_H
                self.FN = self.FONT_NAME
                self.XN = self._CHARS_X
                self.YN = self._CHARS_Y
                self.current_font = None
            else:
                try:
                    data = font.data
                    self.FW = data['width']
                    self.FH = data['height']
                    self.FN = data.get('name','EXT')
                    self.XN = self.WIDTH//self.FW
                    self.YN = self.HEIGHT//self.FH
                    super().loadFont(data['width'],data['height'],data['data'],data['count'])
                    self.current_font = font
                except:
                    raise ValueError('font module missing "data" attribute')
            self.cursor(*cur,pixels=True)
        return dict({'font': self.current_font, 'canvas':self.canvas},**super().options(**kwargs))

    # brightness 0-127
    def setBacklight(self, brightness):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        if(not self._bl):
            self._bl = machine.PWM(machine.Pin.board.LCD_LED)
        self._bl.duty(brightness<<3)

    def framebufColor(self, color):
        return (color&0xff)<<8 | (color>>8)

    def rgbTo565(self, r, g, b):
        """Convert RGB values to RGB565."""
        r5 = (r >> 3) & 0x1F
        g6 = (g >> 2) & 0x3F
        b5 = (b >> 3) & 0x1F
        return (r5 << 11) | (g6 << 5) | b5

    def htmlTo565(self, html_color):
        """Convert HTML color string to RGB565."""
        html_color = html_color.lstrip('#')
        r = int(html_color[0:2], 16)
        g = int(html_color[2:4], 16)
        b = int(html_color[4:6], 16)
        return self.rgbTo565(r, g, b)

    def printException(self,e):
        self.scale(1)
        self.cursor(0,0)
        self.__lock.acquire()
        os.dupterm(self)
        sys.print_exception(e)
        os.dupterm(None)
        self.__lock.release()
        


def tone(note=None, velocity=None):
    midi = note if note else 'c4'
    if(isinstance(note,str)):
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
        octave = int(note[-1])
        note = note[:-1].upper()

        if note in note_names:
            midi = note_names.index(note) + 12 * (octave + 1)
        else:
            raise("Invalid note name")

    spk = machine.PWM(machine.Pin.board.BUZZER)
    if note == None:
        spk.freq(100)
        spk.duty(0)
        return
    spk.duty(velocity>>1)
    spk.freq(int(pow(2,(midi-69)/12)*440))


__neo = neopixel.NeoPixel(machine.Pin.board.LEDS,21)

def clearLeds():
    __neo.fill((0,0,0))
    __neo.write()

def statusLed(r,g,b):
    __neo[0] = (r,g,b)
    __neo.write()

def led(keyNumber,r,g,b):
    __neo[keyNumber + 1] = (r,g,b)
    __neo.write()



class Battery:
    def __init__(self):
        self.vbat = machine.ADC(machine.Pin.board.VBAT_MON)
        self.vbus = machine.Pin.board.CHR_STATE
        self.vbus.init(machine.Pin.IN,pull=machine.Pin.PULL_UP)
        self.DIFF_THRESH = 0.05

    def present(self):
        return True if self.voltage() else False

    def isAC(self):
        r = [0,0,0,0]
        for i in range(4):
            r[i] = not self.vbus.value()
            time.sleep(0.005)
        return bool((r[0]+r[1]+r[2]+r[3])/4)

    def voltage(self):
        r = [0,0,0,0]
        for i in range(4):
            r[i] = self.vbat.read_uv()*2/1000/1000
            time.sleep(0.005)

        self.isAC()
        avr = (r[0]+r[1]+r[2]+r[3])/4
        diff = max(*r) - min(*r);
        return 0 if diff > self.DIFF_THRESH else avr

    def level(self):
        v = self.voltage()
        low = 3
        high = 4.2
        return max(0,(v-low)/(high-low))

