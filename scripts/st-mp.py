from machine import Pin, SPI
import time

# Pin assignments
SCK_PIN = 39
MOSI_PIN = 40
CS_PIN = 42
DC_PIN = 45

# Vertical offset for the display
Y_OFFSET = 35# Adjust this value to match your display's offset
COLOR = [0xf8, 0x10]  # Red in RGB565 format
BG = (0x04, 0x40)

# SPI and pin setup
spi = SPI(2, baudrate=40000000, sck=Pin(SCK_PIN), mosi=Pin(MOSI_PIN))
cs = Pin(CS_PIN, Pin.OUT)
dc = Pin(DC_PIN, Pin.OUT)

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
    send_data([0x00, 0x00, 0x01, 0x3F])  # Start at 0, end at 319

    send_command(0x2B)  # Row address set
    start_row = max(0, min(255, Y_OFFSET))  # Clamp values to byte range
    end_row = max(0, min(255, 239 + Y_OFFSET))  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x13)  # Normal display mode on
    send_command(0x29)  # Display on

def clear_display(color=BG):
    """Clear the display with a specified color (default: black)."""
    send_command(0x2A)  # Column address set
    send_data([0x00, 0x00, 0x01, 0x3F])  # Columns 0 to 319

    send_command(0x2B)  # Row address set
    start_row = max(0, min(255, Y_OFFSET))  # Clamp values to byte range
    end_row = max(0, min(255, 239 + Y_OFFSET))  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x2C)  # Memory write

    # Fill the screen with the specified color
    pixel_data = color * (320 * 170)
    for i in range(0, len(pixel_data), 512):  # Write in chunks
        send_data(pixel_data[i:i+512])

def draw_red_square():
    """Draw a red square on the display."""
    # Set drawing region (a 50x50 square at top-left)
    send_command(0x2A)  # Column address set
    send_data([0x00, 0x00, 0x00, 0x31])  # Columns 0 to 50

    send_command(0x2B)  # Row address set
    start_row = max(0, min(255, Y_OFFSET))  # Clamp values to byte range
    end_row = max(0, min(255, 50 + Y_OFFSET))  # Clamp values to byte range
    send_data([0x00, start_row, 0x00, end_row])

    send_command(0x2C)  # Memory write

    # Send red color data (50x50 pixels, 16 bits per pixel)
    color = COLOR
    pixel_data = color * 50 * 50
    for i in range(0, len(pixel_data), 512):  # Write in chunks
        send_data(pixel_data[i:i+512])

# Main program
init_display()
clear_display()  # Clear the screen to black
draw_red_square()

