from machine import Pin, SPI
import time

bl = Pin(46, Pin.OUT)
bl.value(1)

# Pin assignments
SCK_PIN = 39
MOSI_PIN = 40
CS_PIN = 42
DC_PIN = 45

# Vertical offset for the display
Y_OFFSET = 35# Adjust this value to match your display's offset
Y_SIZE = 170
X_SIZE = 320
COLOR = [0xf8, 0x10]  # Red in RGB565 format
BG = (0x04, 0x40)

# SPI and pin setup
spi = SPI(1, baudrate=40000000, sck=Pin(SCK_PIN), mosi=Pin(MOSI_PIN))
cs = Pin(CS_PIN, Pin.OUT)
dc = Pin(DC_PIN, Pin.OUT)

print("SPI OK")

# Helper functions
def send_command(command):
    """Send a command to the display."""
    dc.value(0)  # Command mode
    cs.value(0)  # Select the display
    spi.write(bytes([command]))
    cs.value(1)  # Deselect the display

def send_data(data):
    """Send data to the display."""
    dc.value(1)  # Data mode
    cs.value(0)  # Select the display
    spi.write(bytes(data))
    cs.value(1)  # Deselect the display

# Display initialization sequence
def init_display():
    """Initialize the ST7789 display."""
    send_command(0x01)  # Software reset
    time.sleep(0.2)     # Delay for reset
    send_command(0x11)  # Sleep out
    time.sleep(0.1)     # Delay for sleep out

    send_command(0x36)  # Memory data access control (MADCTL)
    send_data([0x60])   # Row/column swap, RGB order

    send_command(0x3A)  # Pixel format
    send_data([0x05])   # 16 bits per pixel

    send_command(0x21)  # Inversion on (for proper colors)

    send_command(0x2A)  # Column address set
    send_data([0x00, 0x00, (X_SIZE&0x100)>>8, X_SIZE&0xFF])  # Start at 0, end at 319

    send_command(0x2B)  # Row address set
    start_row = Y_OFFSET  # Clamp values to byte range
    end_row = Y_SIZE + Y_OFFSET  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x13)  # Normal display mode on
    send_command(0x29)  # Display on

def clear_display(color=BG):
    """Clear the display with a specified color (default: black)."""
    send_command(0x2A)  # Column address set
    send_data([0x00, 0x00, (X_SIZE&0x100)>>8, X_SIZE&0xFF])  # Columns 0 to 319

    send_command(0x2B)  # Row address set
    start_row = Y_OFFSET  # Clamp values to byte range
    end_row = Y_SIZE + Y_OFFSET  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x2C)  # Memory write
    
    pixels = [0x07,0x1f] * 320
    for i in range(170):
        send_data(pixels)

def draw_red_square():
    """Draw a red square on the display."""
    # Set drawing region (a 50x50 square at top-left)
    send_command(0x2A)  # Column address set
    send_data([0x00, 0x01, 0x00, 10])  # Columns 0 to 50

    send_command(0x2B)  # Row address set
    start_row = 1 + Y_OFFSET   # Clamp values to byte range
    end_row = 10 + Y_OFFSET  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x2C)  # Memory write

    pixels = [0x22,0x66] * 10
    for i in range(10):
        send_data(pixels)

# Main program
init_display()
print("init ok")
clear_display()  # Clear the screen to black
print("clear ok")
draw_red_square()
print("draw ok")

