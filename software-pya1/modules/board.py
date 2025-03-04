import _board
import keys
import machine
import pins
import neopixel
import esp32
#import bmp

u = esp32.ULP()
u.pause()
u.run_embedded()
u.set_wakeup_period(5000)
u.resume()

class LCD(_board.Terminal):
    def __init__(self):
        super()
        self._bl = None
        self.setBacklight(127)

    def __del__(self):
        self.setBacklight(0)

    def scale(self, scale):
        currentCursor = self.cursor()
        self.options(scale=scale)

    def background(self, color):
        self.options(background=color)

    def foreground(self, color):
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
#
class SD(_board.SD):
    pass

class DAC(_board.DAC):
    pass

__keys = keys.Keys()
def keys():
    return __keys

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
