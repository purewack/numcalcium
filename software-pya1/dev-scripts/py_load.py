from machine import I2C, Pin
import uio
import struct

class EEPROMStreamer(uio.IOBase):
    def __init__(self, i2c):
        self.i2c = i2c
        self.address = 0x50  # Default EEPROM I2C address
        self.offset = 0
        self.size = self._read_file_size()

    def burn(self, path):
        

    def _read_file_size(self):
        # Read the first 2 bytes of EEPROM to determine the file size
        size_data = self.i2c.readfrom_mem(self.address, 0x00, 2)
        size = struct.unpack(">H", size_data)[0]  # Big-endian unsigned short
        if size <= 0 or size > 2048:  # Validate size for a 24C16 (max 2 KB)
            raise ValueError("Invalid file size in EEPROM")
        return size

    def read(self, n=-1):
        if n == -1 or n > self.size - self.offset:
            n = self.size - self.offset
        if n <= 0:
            return b""
        addr = 2 + self.offset  # Skip the first 2 bytes (file size)
        # Handle reading within EEPROM page boundaries
        page_offset = addr % 16
        chunk_size = min(n, 16 - page_offset)
        data = self.i2c.readfrom_mem(self.address, addr, chunk_size)
        self.offset += len(data)
        return data

    def readinto(self, buf):
        data = self.read(len(buf))
        for i in range(len(data)):
            buf[i] = data[i]
        return len(data)

    def seek(self, offset, whence=0):
        if whence == 0:  # SEEK_SET
            self.offset = offset
        elif whence == 1:  # SEEK_CUR
            self.offset += offset
        elif whence == 2:  # SEEK_END
            self.offset = self.size + offset
        self.offset = max(0, min(self.offset, self.size))

    def tell(self):
        return self.offset


i2c = I2C(1, scl=Pin(1), sda=Pin(2)) 
streamer = EEPROMStreamer(i2c)

# Load and execute the `.py` file
try:
    # Read the Python script from the EEPROM
    script_data = streamer.read()
    
    # Decode to string and execute it
    script = script_data.decode('utf-8')
    exec(script)
except Exception as e:
    print("Error executing script from EEPROM:", e)

