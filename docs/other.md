# Other Numcalcium Module Features

This document covers the remaining minor `numcalc` module components: module-level control functions, the `DAC` helper class, LED helpers, and the `Battery` utility.

## Top-Level Control Functions

### `shutdown()`

Shuts down the device cleanly by:

- clearing all NeoPixel LEDs with `clearLeds()`
- entering low-power sleep via `__sleep.enterLowPowerSleep()`

This is the recommended way to put the device into deep power-save mode.

### `runFromCartridge()`

Runs code stored on the external cartridge by delegating to `__cartridge.run()`.

### `burnToCartridge(data)`

Writes `data` to the cartridge using `__cartridge.burn(data)` and then verifies the burn with `__cartridge.check()`.

If the check fails, it raises an exception:

```python
raise Exception('Error during burning, check wiring and chip')
```

## `DAC` Class

The `DAC` class in `software-pya1/modules/numcalc.py` is a thin wrapper around the board-specific audio DAC implementation.

```python
class DAC(_board.DAC):
    pass
```

On this board, the actual audio driver is implemented in C in `software-pya1/cmodules/board_dac.c`.
That module exposes a Sigma-Delta audio class that is used by `_board.DAC`, so `numcalc.DAC` is effectively the Python entry point to the native audio driver.

### Board C module details

- The C implementation lives in `software-pya1/cmodules/board_dac.c`.
- It defines a `SigmaDelta` type with a constructor that accepts a buffer, a callback, and optional output pin configuration.
- The C driver supports single or dual buffer audio streaming and schedules callbacks when half or all samples have been consumed.
- It exposes `deinit()`, `pause()`, and `resume()` methods for managing playback.

### Typical usage

Examples in the repository show audio playback using `numcalc.DAC` with a buffer and a callback:

```python
import numcalc

sdm = numcalc.DAC(buffer_a, buffer_callback)
```

or with explicit output pins:

```python
sdm = numcalc.DAC(buffer_a, buffer_callback, [machine.Pin.board.A_OUT_L, machine.Pin.board.A_OUT_R])
```

A tuple of two buffers may also be passed for ping-pong streaming.


## LED Helper Functions

The module exposes simple NeoPixel-based LED control helpers backed by a `NeoPixel` object attached to `machine.Pin.board.LEDS`.

### `clearLeds()`

Turns all LEDs off and writes the change immediately.

### `statusLed(r, g, b)`

Sets LED index `0` to the provided RGB color and updates the strip.

This is usually used for a status indicator separate from the key LEDs.

### `led(keyNumber, r, g, b)`

Sets the LED for a given key index.

- `keyNumber` is mapped to NeoPixel index `keyNumber + 1`
- the first LED (`index 0`) is reserved for status

Example:

```python
numcalc.led(3, 255, 0, 0)
```

## `Battery` Class

The `Battery` class provides information about the battery and USB charge state.

### Initialization

On construction, `Battery()` initializes:

- `self.vbat` as an ADC on `machine.Pin.board.VBAT_MON`
- `self.vbus` as the charge-state pin `machine.Pin.board.CHR_STATE`
- `self.DIFF_THRESH = 0.05`

### `present()`

Returns `True` when a valid battery voltage is present.

In practice, it calls `self.voltage()` and interprets a non-zero voltage reading as present.

### `isAC()`

Reads the charger state pin four times with short delays and returns a boolean indicating whether external power is present.

### `voltage()`

Samples the battery voltage four times and computes the average.

- reads `self.vbat.read_uv()`
- converts microvolts to volts
- doubles the measured value to account for the voltage divider
- rejects the reading if the sample difference exceeds `DIFF_THRESH`

Returns `0` if the readings are inconsistent, otherwise returns the averaged voltage.

### `level()`

Converts the measured battery voltage into a fractional state-of-charge value between `0.0` and `1.0`.

- `low = 3.0`
- `high = 4.2`

The result is clamped to a minimum of `0`.

## `tone()` Function

Provides buzzer output using PWM on `machine.Pin.board.BUZZER`.

### Usage

- `tone('c4', velocity)` plays a note name with an optional velocity.
- `tone(midi_number, velocity)` plays a MIDI note number.
- `tone(None)` stops the buzzer by setting duty to `0`.

Note handling uses a standard 12-tone formula:

```python
freq = int(pow(2, (midi - 69) / 12) * 440)
```

Velocity is scaled by `>> 1` before it is applied to PWM duty.

## Notes

- `DAC` is a convenience alias for the board's DAC implementation.
- `Battery` is primarily a read-only helper; it does not control charging.
- `shutdown()` is the safe way to transition the device into its low-power sleep state.
- LED helpers always call `__neo.write()` so changes appear immediately.
