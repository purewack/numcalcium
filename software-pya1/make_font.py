from PIL import Image
import numpy
import sys

#python 'name' height width
if not len(sys.argv) == 4 or (len(sys.argv) == 2 and (sys.argv[0] == '-h' or sys.argv[0] == '--help')):
    print("usage: 'filename' {char-width} {char-height}")
    sys.exit()


out_name = sys.argv[1]
char_width = int(sys.argv[2])
char_height = int(sys.argv[3])
img_name = out_name + ".png"
folder_name = '/'.join(out_name.split('/')[:-1])
font_name = out_name.split('/')[-1].split('.')[0]

img = Image.open(img_name)
data = numpy.asarray(img.getdata())

byte = 0
w = img.size[0]
h = img.size[1]

with open(folder_name+"/font.h", 'w') as header:
    header.write("#ifndef _FONT_HEADER\n")
    header.write("#define _FONT_HEADER\n\n")
    header.write("#include <stdint.h>\n")
    header.write(f'#define FONT_NAME "{font_name}"\n')
    header.write("#define FONT_TALL " + str(char_height) + "\n")
    header.write("#define FONT_WIDE " + str(char_width) + "\n")
    header.write("#define FONT_COUNT " + str(int(w/char_width)) + "\n")
    header.write("extern const uint16_t font_data [" + str(w) + "];\n")
    header.write("#endif //_FONT_HEADER\n")

with open(folder_name+"/font.c", 'w') as source:
    source.write('#include "font.h"\n\n')
    source.write(f'//Data generated for FONT_NAME="{font_name}"\n')
    source.write("const uint16_t font_data [" + str(w) + "] = {\n")
    for x in range(w):
        for y in range(h):
            d = data[x + (y*w)][0]
            if(d > 64):
                byte |= (1<<y)
        end =  ",\n" if (x<w-1) else " \n"
        source.write("\t" + hex(byte) + end)
        byte = 0
    source.write("};\n")

print("Generated font files for:",font_name)