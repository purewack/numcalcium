import machine
import board
import os
import framebuf

board.init()

sd = board.SD()
os.mount(sd,'/sd')

p = machine.Pin(21)
spk = machine.PWM(p)
spk.duty_u16(0)
spk.freq(10)

def playNote(n,v):
    f = int(pow(2,(n-69)/12)*440)
    print(f,n,v)
    spk.freq(f)
    spk.duty_u16(int(10000 * v/127))


import struct
import time

class MidiParser:
    def __init__(self, file_path, play_note):
        """
        Initialize the MIDI parser.
        :param file_path: Path to the MIDI file.
        :param play_note: Function to play a note (playNote(note, velocity)).
        """
        self.file_path = file_path
        self.play_note = play_note
        self.ticks_per_quarter_note = 120  # Default, can be updated based on file
        self.tempo = 500000  # Default tempo in microseconds per quarter note

    def parse(self):
        with open(self.file_path, 'rb') as f:
            # Verify MIDI header
            header = f.read(14)
            if header[:4] != b'MThd':
                raise ValueError("Invalid MIDI file")
            _, _, self.ticks_per_quarter_note = struct.unpack(">HHH", header[8:14])
            
            print("playing")
            time.sleep(1)
            
            # Process tracks
            while True:
                chunk_header = f.read(8)
                if len(chunk_header) < 8:
                    break
                chunk_type, chunk_size = struct.unpack(">4sI", chunk_header)
                if chunk_type == b'MTrk':
                    self._parse_track(f, chunk_size)
                else:
                    f.seek(chunk_size, 1)  # Skip non-track chunks

    def _parse_track(self, f, track_length):
        track_data = f.read(track_length)
        pos = 0
        running_status = None
        current_time = 0

        while pos < len(track_data):
            # Read delta-time
            delta_time, pos = self._read_varlen(track_data, pos)
            delay_seconds = self._ticks_to_seconds(delta_time)

            # Pause for the calculated duration
            time.sleep(delay_seconds)

            # Read event type
            status_byte = track_data[pos]
            if status_byte & 0x80:  # New status byte
                running_status = status_byte
                pos += 1
            else:  # Running status
                status_byte = running_status

            # Parse event
            if status_byte >= 0x80 and status_byte < 0x90:  # Note Off
                note, velocity = track_data[pos:pos+2]
                pos += 2
                self.play_note(note, 0)
            elif status_byte >= 0x90 and status_byte < 0xA0:  # Note On
                note, velocity = track_data[pos:pos+2]
                pos += 2
                if velocity > 0:
                    self.play_note(note, velocity)
                else:
                    self.play_note(note, 0)  # Treat Note On with velocity 0 as Note Off
            elif status_byte == 0xFF:  # Meta events
                meta_type = track_data[pos]
                pos += 1
                meta_length, pos = self._read_varlen(track_data, pos)
                if meta_type == 0x51:  # Set tempo
                    self.tempo = int.from_bytes(track_data[pos:pos+meta_length], "big")
                pos += meta_length
            else:  # Skip other events
                pos += 1

    def _read_varlen(self, data, pos):
        value = 0
        while True:
            byte = data[pos]
            pos += 1
            value = (value << 7) | (byte & 0x7F)
            if not (byte & 0x80):
                break
        return value, pos

    def _ticks_to_seconds(self, ticks):
        seconds_per_tick = self.tempo / 1000000.0 / self.ticks_per_quarter_note
        return ticks * seconds_per_tick


# Example
midi_file_path = "/sd/test.mid"  # Replace with your MIDI file path
parser = MidiParser(midi_file_path, playNote)
parser.parse()
