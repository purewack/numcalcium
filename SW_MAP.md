# Software Map

The exposed pins contribute to a number of functions base on which program is being used.

| **Program** | GND  | TX   | RX   | GND  | MOSI | MISO | CK   | CS   | GND  | SDA  | SCL  | GND | GP_A | GP_B | EPWR |
| Keyboard    |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| Calculator  |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| MIDI        |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| Comms       |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| Signal      |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| GPIO        |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |
| Scope       |      |      |      |      |      |      |      |      |      |      |      |     |      |      |      |


### Base Input Multiplexer
Uses Timer 3 to plex pins A14,A15,B3,B4,B8,B9  and read pins B11,B10,B1,B0
The ISR detects button presses and logs states to io_t struct called io

	uint32_t io.bscan_down //button bits for down presses

Holds all buttons that transitioned from up to down state, buffered so requires clearing after you read the state

	if(io.bscan_down & (1<<K_X)){
		//action

		//clean up
		io.bscan_down = 0;
		//OR
		io.bscan_down &= ~(1<<K_X);
	}

### Soft I2S uses timer 
