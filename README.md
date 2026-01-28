# Numcalcium32 
An ESP32 calculator

*under construction*

Dev patterns:

- Build and flash main firmware:

        make all
        make deploy

- Build and flash firmware with no modules, just c drivers:

        make firmware-no-freeze
        make deploy-no-freeze

- Build ULP code:

        make gen-ulp

- Build font file:
        make gen-font

- Update pin definitions:
        make gen-pins



Tasks:

[x] lcd driver
[x] keypad and encoder driver
[x] deepsleep and wake
[x] headphone SDM dac
[x] buzzer driver
[x] rgb
[x] BMP decoder
[x] terminal redirection `os.dupterm`
[x] battery monitoring
[x] rename `board` module to `numcalc`
[x] allow loading custom fonts, in py file form
[x] cartridge code loading of .mpy code testing
[ ] clean up debug prints from modules and cmodules