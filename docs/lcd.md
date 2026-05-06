# LCD Class Reference

The `LCD` class in `software-pya1/modules/numcalc.py` provides a high-level interface for the Numcalcium display. It extends the board-specific `_board.Terminal` implementation and adds convenience methods for drawing, text rendering, color conversion, canvas support, custom fonts, and backlight control.

## Key Features

- 320x170 display resolution (`LCD.WIDTH`, `LCD.HEIGHT`)
- Built-in 16-bit RGB565 color constants and helpers
- Text rendering with cursor control and scaling
- Canvas creation for off-screen drawing and partial updates
- Bitmap support and BMP loading
- Custom font loading from generated font modules
- Backlight brightness control
- Pixel plotting, area fill, and Bresenham-style line drawing

## Constants

The `LCD` class exposes common color constants:

- `LCD.RED`
- `LCD.GREEN`
- `LCD.BLUE`
- `LCD.PURPLE`
- `LCD.YELLOW`
- `LCD.CYAN`
- `LCD.BLACK`
- `LCD.WHITE`
- `LCD.GRAY`

It also includes display dimensions:

- `LCD.WIDTH = 320`
- `LCD.HEIGHT = 170`

## Initialization

Create the LCD object with:

```python
from numcalc import LCD
lcd = LCD()
```

The constructor resets the display and sets the default backlight brightness to a mid-level value.

## Drawing and Display Methods

### Clearing and background

- `lcd.clear()` clears the display and resets the cursor to `(0, 0)`.
- `lcd.fill(x, y, w, h, color)` fills a rectangle.
- `lcd.plot(x, y, color)` plots a single pixel.
- `lcd.line(x, y, x2, y2, color)` draws a line.

The `color` argument accepts:

- an integer RGB565 value,
- an HTML string like `"#ff0000"`, or
- an RGB tuple like `(255, 0, 0)`.

### Bitmaps and canvas

- `lcd.createCanvas()` allocates an off-screen canvas buffer for full-screen drawing.
- `lcd.update(x, y, w, h)` updates the display from the canvas.
- `lcd.buffer(buf, x, y, width, height)` writes raw framebuffer data.
- `lcd.loadBMP(file, headerOnly=False)` loads BMP image data from a file.
- `lcd.bitmap(x, y, image, scale=1)` draws a bitmap, with integer or float scaling.

### Text and cursor control

- `lcd.print(*args)` writes text to the display.
- `lcd.cursor(x, y, pixels=False, bottom=False)` can read or set the cursor position.
- `lcd.scale(value)` changes the text scale.
- `lcd.measure_text(text, pixels=False)` returns text size in character cells or pixels.

The cursor API supports both raw text coordinates and pixel coordinates via `pixels=True`.
It also supports a `bottom=True` mode for bottom-aligned coordinate translation.

### Colors and display options

- `lcd.background(color)` sets the background color.
- `lcd.foreground(color)` or `lcd.color(color)` sets the text/foreground color.
- `lcd.bg(color)` is an alias for `lcd.background(color)`.
- `lcd.invert(state)` enables or disables display inversion.
- `lcd.options(...)` accepts many display options and normalizes color inputs.

### Backlight

- `lcd.setBacklight(brightness)` sets brightness from `0` to `127`.

## Custom Font Support

The LCD supports loading external font modules that define:

- `data['width']`
- `data['height']`
- `data['count']`
- `data['name']`
- `data['data']`

A font module is loaded by passing it to `lcd.options(font=font_module)`.
When a font is unloaded with `lcd.options(font=None)`, the display returns to the default built-in font.

### Generating custom fonts with `make gen-font`

The repository includes a `Makefile` target for generating font assets:

```make
make gen-font
```

This target does the following:

1. Ensures `software-pya1/generated/` exists.
2. Creates the `software-pya1/fonts/$(FONT)` font directory.
3. Runs `software-pya1/generators/font.py` to build font files from the font PNG.
4. Produces generated Python font modules named `font_<name>.py` in `software-pya1/generated`.
5. Compiles each generated font module to MicroPython bytecode with `mpy-cross`.

The default font configuration in `Makefile` is:

- `FONT = gohu13`
- `FONT_CHAR_COUNT = 96`

### Using a generated font

After generating fonts, import the font module and load it into the LCD:

```python
import font_gohu13
from numcalc import LCD

lcd = LCD()
lcd.options(font=font_gohu13)
lcd.print("Hello custom font")
```

Once loaded, the LCD uses the generated font dimensions for text rendering and cursor layout.

## Example Usage

```python
from numcalc import LCD
import font_gohu13

lcd = LCD()
lcd.clear()
lcd.background("#000000")
lcd.foreground((255, 255, 255))
lcd.options(font=font_gohu13)
lcd.scale(2)
lcd.print("Numcalcium LCD")

canvas = lcd.createCanvas()
# draw to canvas with lcd.plot, lcd.fill, lcd.line, etc.
lcd.update()
```

## Notes

- `LCD.createCanvas()` allocates a full-screen off-screen buffer that can be updated incrementally.
- `lcd.scale()` changes the effective drawing grid and text cell size.
- `lcd.htmlTo565()` and `lcd.rgbTo565()` are available for converting standard color formats.

This document describes the high-level `LCD` API and how to generate and load custom fonts for Numcalcium applications.