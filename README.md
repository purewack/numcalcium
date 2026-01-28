# Numcalcium32 
An ESP32 calculator running MicroPython

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

