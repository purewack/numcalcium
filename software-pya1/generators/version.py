import sys
import datetime

# Command-line argument handling
if len(sys.argv) != 3:
    print("Usage: python version.py <revision_number> <output_py>")
else:
    _,version_no, output_py = sys.argv

    with open(output_py,'w') as f:
        f.write(f"_version='{version_no}'\n")
        d = datetime.datetime.now().strftime("%Y-%m-%d-%H%M")
        f.write(f"_build='{d}'")