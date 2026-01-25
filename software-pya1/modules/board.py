import _board
import __keys 
import __sleep
import machine
import neopixel
import os
import sys
import time
import _thread

_board.init()
__sleep.exitLowPowerSleep()

def shutdown():
    # clearLeds()
    __sleep.enterLowPowerSleep()

class Keys(__keys.Keys):
    pass

class SD(_board.SD):
    pass

class DAC(_board.DAC):
    pass

class LCD(_board.Terminal):
    def __init__(self):
        super().__init__()
        if not hasattr(self,'__lock'):
            self.__lock = _thread.allocate_lock()
        self._bl = None
        self.reset()
        self.setBacklight(127)

    def __del__(self):
        self.setBacklight(0)

    def _sendcmd(self):
        pass

    def _senddata(self):
        pass

    def reset(self):
        self.options(background=self.BLACK,foreground=self.WHITE,scale=1)
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

    def buffer(self,buf,x,y,width,height):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().buffer(buf,x,y,width,height)

    def bitmap(self,x,y,image):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().buffer(image['buffer'],x,y,image['width'],image['height'])
    
    def print(self, *args):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        super().print(*args)

    def cursor(self, *args, **kwargs):
        if not self.__lock.acquire(False): return
        self.__lock.release()
        if(not len(args)):
            return super().cursor()
        if(not len(args) == 2):
            raise ValueError("Need both x and y positions")
        x = args[0]
        y = args[1]
        if(kwargs.get('pixels',False)):
            s = super().options()['scale']
            ww = self.WIDTH
            hh = self.HEIGHT
            fh = self.FONT_H
            fw = self.FONT_W
            xx = (x/ww)*((ww/fw)/s)
            yy = (y/hh)*((hh/fh)/s)
            return super().cursor(xx,yy)
        else:
            return super().cursor(x,y)

    def scale(self, scale=None):
        if scale == None:
            return super().options()['scale']

        if not self.__lock.acquire(False): return
        self.__lock.release()
        currentCursor = self.cursor()
        super().options(scale=scale)
    
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

        if(kwargs.get('foreground',False)):
            if isinstance(kwargs['foreground'], str):
                kwargs['foreground'] = self.htmlTo565(kwargs['foreground'])
            elif isinstance(kwargs['foreground'], tuple):
                kwargs['foreground'] = self.rgbTo565(*kwargs['foreground'])

        if(kwargs.get('background',False)):
            if isinstance(kwargs['background'], str):
                kwargs['background'] = self.htmlTo565(kwargs['background'])
            elif isinstance(kwargs['background'], tuple):
                kwargs['background'] = self.rgbTo565(*kwargs['background'])

        return super().options(**kwargs)

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
    __neo[1 + keyNumber] = (r,g,b)
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

