from PIL import Image
import numpy
import sys

#python 'name' height width
if not len(sys.argv) == 4 or sys.argv[0] == '--help':
    print("Usage: python font.py <filename> <output_path> <font_char_count> - standard font set has 104 supported characters")
    sys.exit()


out_name = sys.argv[1]
out_path = sys.argv[2]
img_name = out_name + ".png"
font_name = out_name.split('/')[-1].split('.')[0]
safe_name = font_name.replace('-','_')


img = Image.open(img_name)
data = numpy.asarray(img.getdata())

w = img.size[0]
h = img.size[1]

dataType = 'uint8_t'

char_set_count = int(sys.argv[3])
char_width = w//char_set_count
char_height = h

bytes_per_height = (h//8+1)
bytes_total = bytes_per_height*w

def build_pixel_bytes(onNewByte):
    byte = 0
    for x in range(w):
        for y in range(h):
            d = data[x + (y*w)][0]
            if(d > 64):
                byte |= (1<<(y%8))
            if (y+1) % 8 == 0: 
                onNewByte(byte)
                byte = 0
        onNewByte(byte)
        byte = 0

with open(out_path+"/font_"+safe_name+".h", 'w') as header:
    header.write(f"#ifndef _FONT_HEADER_{safe_name}\n")
    header.write(f"#define _FONT_HEADER_{safe_name}\n\n")
    header.write("#include <stdint.h>\n")
    header.write(f'#define FONT_NAME_{safe_name} "{safe_name.upper()}"\n')
    header.write(f'#define FONT_NAME_Q_{safe_name} "MP_QSTR_{safe_name.upper()}"\n')
    header.write(f"#define FONT_TALL_{safe_name} {str(char_height)} \n")
    header.write(f"#define FONT_WIDE_{safe_name} {str(char_width)} \n")
    header.write(f"#define FONT_CHARS_{safe_name} {str(char_set_count)} \n")
    header.write(f"extern const uint8_t font_data_{safe_name} [{str(bytes_total)}];\n")
    header.write(f"#endif //_FONT_HEADER_{safe_name}\n")

with open(out_path+"/font_"+safe_name+".c", 'w') as source:
    source.write(f'#include "font_{safe_name}.h"\n\n')
    source.write(f'//Data generated for {safe_name.upper()}"\n')
    source.write(f"const uint8_t font_data_{safe_name} [{str(bytes_total) }] = " + "{\n")
    
    px = []
    def onNewByte(b):
        px.append(b)
    build_pixel_bytes(onNewByte)
    
    for i,a in enumerate(px):
        end =  "\n" if (i == len(px)-1) else ",\n"
        source.write("\t" + hex(a) + end)
    
    source.write("};\n")

with open(out_path+"/font_"+safe_name+'.py','w') as f:
    pixels = []
    
    def onNewByte(b):
        pixels.append(b)
    build_pixel_bytes(onNewByte)
    
    data = {
        'width': char_width,
        'height': char_height,
        'count': char_set_count,
        'name': safe_name.upper(),
        'data': bytearray(pixels)
    }
    f.write(f'# {char_set_count} character set: {safe_name.upper()}\n\r')
    f.write("data=")
    print(data,file=f)

print("Generated font files for:",safe_name.upper())