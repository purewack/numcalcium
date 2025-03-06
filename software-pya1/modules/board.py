import _board
import keys as _keys
import machine
import pins
import neopixel
import esp32
import os
import sys

u = esp32.ULP()
u.pause()
u.run_embedded()
u.set_wakeup_period(5000)
u.resume()

keys = _keys.Keys()

class LCD(_board.Terminal):
    def __init__(self):
        super().__init__()
        self._bl = None
        self.setBacklight(127)

    def __del__(self):
        self.setBacklight(0)

    def scale(self, scale):
        currentCursor = self.cursor()
        self.options(scale=scale)
    
    def plot(self, x,y,color):
        if isinstance(color, str):
            super().fill(x,y,w,h,self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().fill(x,y,w,h,self.rgbTo565(*color))
        else:
            super().fill(x,y,w,h,color)

    def fill(self, x,y,w,h,color):
        if isinstance(color, str):
            super().fill(x,y,w,h,self.htmlTo565(color))
        elif isinstance(color, tuple):
            super().fill(x,y,w,h,self.rgbTo565(*color))
        else:
            super().fill(x,y,w,h,color)

    def background(self, color):
        if isinstance(color, str):
            self.options(background=self.htmlTo565(color))
        elif isinstance(color, tuple):
            self.options(background=self.rgbTo565(*color))
        else:
            self.options(background=color)

    def foreground(self, color):
        if isinstance(color, str):
            self.options(foreground=self.htmlTo565(color))
        elif isinstance(color, tuple):
            self.options(foreground=self.rgbTo565(*color))
        else:
            self.options(foreground=color)
    
    def color(self, color):
        self.foreground(color)

    def bg(self, color):
        self.background(color)

    def invert(self, state):
        self.options(invert=state)
    
    # brightness 0-127
    def setBacklight(self, brightness):
        if(not self._bl):
            self._bl = machine.PWM(machine.Pin.board.LCD_LED)
        self._bl.duty(brightness<<3)

    def framebufColor(self, color):
        return (color&0xff)<<8 | (color>>8)

    def bitmap(self,x,y,image):
        self.buffer(image['buffer'],x,y,image['width'],image['height'])
    
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
        os.dupterm(self)
        sys.print_exception(e)
        os.dupterm(None)

class SD(_board.SD):
    pass

class DAC(_board.DAC):
    pass


def tone(note, velocity):
    midi = note
    if(isinstance(note,str)):
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
        octave = int(note[-1])
        note = note[:-1].upper()

        if note in note_names:
            midi = note_names.index(note) + 12 * (octave + 1)
        else:
            raise("Invalid note name")

    spk = machine.PWM(machine.Pin.board.BUZZER)
    spk.duty(velocity>>1)
    spk.freq(int(pow(2,(midi-69)/12)*440))


__neo = neopixel.NeoPixel(machine.Pin.board.LEDS,21)
def clearLights():
    __neo.fill((0,0,0))
    __neo.write()

def statusLight(r,g,b):
    __neo[0] = (r,g,b)
    __neo.write()

def statusLed(r,g,b):
    statusLight(r,g,b)

def light(keyNumber,r,g,b):
    __neo[1 + keyNumber] = (r,g,b)
    __neo.write()

def led(keyNumber,r,g,b):
    light(keyNumber,r,g,b)
