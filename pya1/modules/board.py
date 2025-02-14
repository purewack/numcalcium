import _board
import keys
import machine
import pins
import neopixel

_board.init()

class LCD(_board.Terminal):

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
        b = machine.PWM(machine.Pin.board.LCD_LED)
        b.duty(brightness<<3)

class SD(_board.SD):
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


__neo = neopixel.NeoPixel(machine.Pin.board.LEDS,1)
def statusLight(r,g,b):
    __neo[0] = (r,g,b)
    __neo.write()
