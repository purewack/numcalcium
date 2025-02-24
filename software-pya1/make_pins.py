import re
import csv
import sys

def extract_pins_from_header(header_file, output_csv, output_py):
    pattern = re.compile(r"#define\s+BOARD_PIN_([A-Za-z0-9_]+)\s+(\d+)")
    
    pins = []
    micropython_lines = ["from micropython import const\n"]
    
    try:
        with open(header_file, "r") as file:
            for line in file:
                match = pattern.match(line.strip())
                if match:
                    pin_name, gpio_number = match.groups()
                    gpio_number_int = int(gpio_number)  # Convert to integer
                    gpio_number_str = f"GPIO{gpio_number_int}"  # Format as two digits
                    
                    pins.append((pin_name, gpio_number_str))
                    micropython_lines.append(f"{pin_name} = const({gpio_number_int})")
        
        if not pins:
            print("No pin definitions found.")
            return

        # Write to CSV
        with open(output_csv, "w", newline="") as file:
            writer = csv.writer(file)
            writer.writerows(pins)

        # Write to MicroPython module
        with open(output_py, "w") as file:
            file.write("\n".join(micropython_lines) + "\n")

        print(f"CSV file '{output_csv}' and MicroPython module '{output_py}' generated successfully.")

    except FileNotFoundError:
        print(f"Error: File '{header_file}' not found.")
    except Exception as e:
        print(f"Error: {e}")

# Command-line argument handling
if len(sys.argv) != 4:
    print("Usage: python generate_pins.py <input_header> <output_csv> <output_py>")
else:
    _, header_file, output_csv, output_py = sys.argv
    extract_pins_from_header(header_file, output_csv, output_py)

