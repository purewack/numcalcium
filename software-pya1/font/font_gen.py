from PIL import Image
import numpy
import sys

#python 'name' height width

out_name = sys.argv[1]
char_height = int(sys.argv[2])
char_width = int(sys.argv[3])
img_name = out_name + ".png"

img = Image.open(img_name)
data = numpy.asarray(img.getdata())

byte = 0
w = img.size[0]
h = img.size[1]

with open(out_name + ".h", 'w') as header:
    header.write("#ifndef _FONT_HEADER\n")
    header.write("#define _FONT_HEADER\n\n")
    header.write("#include <stdint.h>\n")
    header.write("const uint8_t font_tall = " + str(char_height) + ";\n")
    header.write("const uint8_t font_wide = " + str(char_width) + ";\n")
    header.write("const uint16_t font_count = " + str(int(w/char_width)) + ";\n")
    header.write("const uint16_t font_data [" + str(w) + "] = {\n")
    for x in range(w):
        for y in range(h):
            d = data[x + (y*w)][0]
            if(d > 64):
                byte |= (1<<y)
        end =  ",\n" if (x<w-1) else " \n"
        header.write("\t" + hex(byte) + end)
        byte = 0
    header.write("};\n")
    header.write("#endif //_FONT_HEADER\n")

