import machine
import time
import struct
import io

class EEPROMStream(io.IOBase):
    def __init__(self, i2c, addr=80, size=2048, block_size=8):
        self.i2c = i2c
        self.addr = addr
        self.size = size
        self.block_size = block_size
        self.position = 8  # Skip header
        self.closed = False
        self._check_ready()

    def _check_ready(self):
        time.sleep(0.01)

    def burn(self, data):
        """Write data to EEPROM with a header."""
        if self.closed:
            raise OSError("Stream closed")
        size = len(data)
        if size > self.size:
            raise Exception('Data too large')

        offset = 8
        for i in range(0, size, self.block_size):
            chunk = data[i:i + self.block_size]
            bank = offset // 256
            self.i2c.writeto_mem(self.addr + bank, offset % 256, chunk)
            offset += len(chunk)
            self._check_ready()

        # Write the header with size
        header = bytearray([ord('M'), ord('P'), ord('Y'), ord('n'), ord('u'), ord('m')])
        header.extend(struct.pack('>H', size))
        self.i2c.writeto_mem(self.addr, 0, header)
        self._check_ready()

        self.position = 8  # Reset position after write

    def seek(self, pos, whence=0):
        """Move read pointer to `pos`."""
        if self.closed:
            raise OSError("Stream closed")
        if whence == 1:  # SEEK_CUR
            pos += self.position
        elif whence == 2:  # SEEK_END
            pos = self.size + pos

        if pos < 8:
            pos = 8  # Prevent seeking into the header
        self.position = min(pos, self.size)

    def tell(self):
        """Return current read pointer position."""
        if self.closed:
            raise OSError("Stream closed")
        return self.position

    def read(self, n=-1):
        """Read up to `n` bytes from EEPROM."""
        if self.closed:
            raise OSError("Stream closed")
        if n < 0:
            return self.read_all()  # Read everything if no size given

        if self.position + n > self.size:
            n = self.size - self.position  # Trim read to available data

        data = bytearray()
        offset = self.position

        while n > 0:
            bank = offset // 256
            chunk_size = min(self.block_size, n)
            chunk = self.i2c.readfrom_mem(self.addr + bank, offset % 256, chunk_size)
            data.extend(chunk)
            offset += len(chunk)
            n -= len(chunk)

        self.position = offset
        return bytes(data)  # Convert to bytes for compatibility

    def readinto(self, buf):
        """Read directly into an existing buffer (faster)."""
        if self.closed:
            raise OSError("Stream closed")
        n = len(buf)
        data = self.read(n)
        buf[: len(data)] = data
        return len(data)

    def close(self):
        """Close the stream (mark it as unusable)."""
        print("EEPROMStream closed")  # Debug print
        self.closed = True

    def ioctl(self, op, arg):
        print(f"ioctl({op}, {arg})")  # Debug print
        if op == 0:  # Polling (used internally by some modules)
            return 0
        elif op == 1:  # Seekable check
            return 1
        elif op == 2:  # Flush (ignore)
            return 0
        elif op == 4:  # Check if close is supported
            return 1  # Indicate that `close()` exists
        return -1  # Operation not supported

# Example Usage
i2c = machine.I2C(1, scl=1, sda=2)
eeprom = EEPROMStream(i2c)

# Load a file and burn it to EEPROM
#with open('natmod/_hello.mpy', 'rb') as f:
#    data = f.read()
#    eeprom.burn(data)

import mpy
mpy.from_stream(eeprom)

eeprom.close()
