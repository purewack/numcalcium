from PIL import Image
import numpy
import sys

#python 'name' height width
if not len(sys.argv) == 3 or sys.argv[0] == '--help':
    print("usage: 'filename' 'outpath'")
    sys.exit()


out_name = sys.argv[1]
out_path = sys.argv[2]
img_name = out_name + ".png"
font_name = out_name.split('/')[-1].split('.')[0]

img = Image.open(img_name)
data = numpy.asarray(img.getdata())

w = img.size[0]
h = img.size[1]

if(h > 16): 
    raise Exception('Image height too large')

char_set_count = 104
char_width = w//char_set_count
char_height = h

with open(out_path+"/font_"+font_name+".h", 'w') as header:
    header.write("#ifndef _FONT_HEADER\n")
    header.write("#define _FONT_HEADER\n\n")
    header.write("#include <stdint.h>\n")
    header.write(f'#define FONT_NAME "{font_name}"\n')
    header.write("#define FONT_TALL " + str(char_height) + "\n")
    header.write("#define FONT_WIDE " + str(char_width) + "\n")
    header.write("extern const uint16_t font_data [" + str(w) + "];\n")
    header.write("#endif //_FONT_HEADER\n")

with open(out_path+"/font_"+font_name+".c", 'w') as source:
    source.write(f'#include "font_{font_name}.h"\n\n')
    source.write(f'//Data generated for FONT_NAME="{font_name}"\n')
    source.write("const uint16_t font_data [" + str(w) + "] = {\n")
    byte = 0
    for x in range(w):
        for y in range(h):
            d = data[x + (y*w)][0]
            if(d > 64):
                byte |= (1<<y)
        end =  ",\n" if (x<w-1) else " \n"
        source.write("\t" + hex(byte) + end)
        byte = 0
    source.write("};\n")

with open(out_path+"/font_"+font_name+'.py','w') as f:
    pixels = []
    for x in range(w):
        for byte_idx in range(1 + (h // 8)):
            byte = 0
            for y in range(8):
                pixel_y = byte_idx * 8 + y
                if pixel_y < h:
                    d = data[x + (pixel_y * w)][0]
                    if d > 64:
                        byte |= (1 << y)
            pixels.append(byte)
    data = {
        'width': char_width,
        'height': char_height,
        'data': bytearray(pixels)
    }
    f.write(f'# {char_set_count} character set: {font_name}\n\r')
    f.write("data=")
    print(data,file=f)

print("Generated font files for:",font_name)