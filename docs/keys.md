# Keys Class Reference

The `Keys` class in `software-pya1/modules/__keys.py` provides access to the Numcalcium button matrix and encoder input. It is backed by an ESP32 ULP program that scans keys continuously, including during deep sleep, and stores press/release events in buffered state registers.

## Hardware

The keys are divided into groups of 3, reading from bottom to top and from left to right, giving 7 groups, with the last one having 2 keys.

The ULP manages the input and output states of only 4 pins to read 20 keys, multiplexing pin functions to drive a shift register on board driving each group one by one and reading each group state, accumulating results over the scanning period and placing the results in the buffered register, which the Keys class has access to via `mem32`.

![io timing](/hardware-pya1/ref/numcalc_io_dt.png)

## Overview

- The ULP scans all physical keys and the rotary encoder.
- Key press and release events are buffered until explicitly cleared.
- Encoder turns and the encoder "home" click are also managed by the ULP.
- Raw read methods return current hardware state without clearing buffered events.

## Key encoding and mapping

Keys are represented as bits within a 20-bit integer.

- Each key corresponds to one bit.
- Bits are ordered from bottom-to-top, left-to-right.
- A set bit indicates the key has been pressed or released, depending on the buffer.

The class exposes named key constants, for example:

- `Keys.SHIFT`
- `Keys.DOT`
- `Keys.N0` through `Keys.N9`
- `Keys.F1`, `Keys.F2`, `Keys.F3`
- `Keys.A`, `Keys.B`, `Keys.C`, `Keys.D`, `Keys.E`

Alias names are also available for readability:

- `Keys.KEY_SHIFT`
- `Keys.KEY_DOT`
- `Keys.KEY_0` through `Keys.KEY_9`
- `Keys.KEY_F1`, `Keys.KEY_F2`, `Keys.KEY_F3`
- `Keys.KEY_SIDE_A` through `Keys.KEY_SIDE_E`

## ULP-managed buffering

The ULP keeps two buffered registers:

- `bdown` stores keys that have been pressed down.
- `bup` stores keys that have been released.

Buffer state is preserved across normal operation and deep sleep, allowing the application to read events later without missing input.

### Down and up queries

- `keys.isDown(key, reset=False)` returns whether the specified key is recorded as down.
- `keys.isUp(key, reset=False)` returns whether the specified key is recorded as up.

Both methods support an optional `reset=True` to clear the buffer bit for that key.

### Batch reads

- `keys.getAllDown()` returns a list of all keys currently buffered as down, then clears that buffer.
- `keys.getAllUp()` returns a list of all keys currently buffered as up, then clears that buffer.
- `keys.getNextDown()` returns the next buffered down key and clears it.
- `keys.getNextUp()` returns the next buffered up key and clears it.

### Clearing

- `keys.clearDown(key)` clears a single down bit.
- `keys.clearUp(key)` clears a single up bit.
- `keys.clearAllDown()` clears all down events.
- `keys.clearAllUp()` clears all up events.
- `keys.clearAll()` clears both up/down buffers and resets turn counts.

## Encoder support

Encoder movement and clicks are handled by the same ULP process.

- `keys.raw_turns()` returns the current raw encoder turn count.
- `keys.turns(invert=False)` reads and clears the turn counter.
- `keys.raw_home()` returns the current raw encoder click state.

### Home button behavior

The encoder click is treated specially as a "home" button, because its primary role is to return to the app launcher.

- `keys.shouldHome(value=None)` reads or writes the buffered home request flag.
- `keys.wasHomeRequested()` returns `True` if the home button was pressed, and clears the buffered state.

The internal `bhome` register is used to preserve the encoder click state until the main application consumes it.

## Raw state access

Raw read methods allow polling of the current hardware state without affecting buffered event registers:

- `keys.raw()` returns the current scanned key matrix value from `bscan`.
- `keys.raw_home()` returns the raw encoder click state.
- `keys.raw_turns()` returns the current encoder turn count.

These raw methods are useful for diagnostics and immediate state monitoring while keeping buffered press/release history intact.

## Initialization and driver loading

When a `Keys` object is constructed, it:

1. creates an `esp32.ULP_RV()` instance,
2. loads the ULP binary from `__ulpio`, and
3. starts the ULP program.

An optional scan period may be provided to `Keys(period)` to adjust the ULP wakeup interval.

## Example Usage

```python
from __keys import Keys

keys = Keys()

if keys.isDown(Keys.KEY_1, reset=True):
    print("Key 1 pressed")

turns = keys.turns()
if turns != 0:
    print("Encoder moved", turns)

if keys.wasHomeRequested():
    print("Home button clicked")

print("raw key bits", hex(keys.raw()))
```

## Notes

- The ULP scans keys even while the main CPU is asleep, making input reliable across sleep cycles.
- Press/release events are buffered independently from raw state reads.
- The encoder click is buffered as a home event rather than a normal key event.
- Clearing buffered values is explicit; raw reads do not consume events.
