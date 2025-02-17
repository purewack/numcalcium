import os
import sys
import time
import esp32
import machine
import gc

import board
import nav

keys = board.keys()
lcd = board.LCD()

u = esp32.ULP()
u.pause()
u.run_embedded()
u.write(u.VAR_SYSTEM_SLEEPING,0)
u.set_wakeup_period(1000)
u.resume()

def start():
    if(keys == (keys.SHIFT | keys.A | keys.E | keys.F1)):
        os.dupterm(lcd)
        lcd.clear()
        print("Non-standard Boot")
        return

    print("Standard Boot")
    programs = collect_manifest_paths()
    program_keys = list(programs.keys())
    print(programs)
    if not len(program_keys):
        lcd.print("No Programs")
        return

    index = 0
    def selectProgram(index):
        nonlocal programs, program_keys
        lcd.clear()
        lcd.scale(2)
        lcd.print("Programs: \n\r")
        for i in range(len(program_keys)):
            lcd.print(str(i + 1))
            lcd.print(".")
            lcd.print(program_keys[i])
            if i == index:
                lcd.print(" <")
            lcd.print("\n\r")
    selectProgram(index)
    while True:
        turns = nav.turns()
        if nav.wasBackRequested():  # Select and run program
            print("exec")
            keys.clearDown(keys.A)
            run_program(program_keys[index], programs[program_keys[index]])
            selectProgram(index)
            while nav.wasBackRequested(): pass
            time.sleep(0.2)
        elif turns:
            index = (index + turns) % len(program_keys)
            selectProgram(index)
            print("up")
            keys.clearDown(keys.N5)
        time.sleep(0.1)

def collect_manifest_paths():
    manifest_dict = {}
    current_directory = os.getcwd()
    print("looking in",current_directory)
    try:
        items = os.listdir(current_directory)
    except OSError:
        return manifest_dict  # Return empty dict if directory doesn't exist

    for item in items:
        item_path = current_directory + "/" + item
        try:
            stat = os.stat(item_path)
            if stat[0] & 0x4000:  # Check if it's a directory
                manifest_path = item_path + "/_manifest.txt"
                progname = item
                print(progname,item)
                try:
                    with open(manifest_path, "r") as file:
                        progname = file.readline().strip()  # Read program name
                    manifest_dict[progname] = item_path
                except:
                    manifest_dict[f"<{item}>"] = item_path # Store path
                    pass
        except OSError:
            pass  # Ignore if directory or manifest doesn't exist

    return manifest_dict

def run_program(name, path):
    print(f"Launching {name} from {path}")

    sys.path.insert(0, path)
    
    pre_path = os.getcwd()
    
   # Backup sys.modules before importing
    old_modules = sys.modules.copy()

    # Remove _program and its dependencies
    for mod in list(sys.modules):
        if mod.startswith("_program") or mod.startswith(name):
            del sys.modules[mod]

    while nav.shouldBack(): pass
    nav.wasBackRequested()

    os.chdir(path)
    lcd.clear()
    board.statusLight(0,0,0)

    try:
        __import__("_program", None, None, [])
    except Exception as e:
        print(f"Failed in program [{name}]")
        sys.print_exception(e)

    finally:
        sys.path.pop(0)
        sys.modules.clear()
        sys.modules.update(old_modules)
        os.chdir(pre_path)
        gc.collect()
        print("MEM (free,alloc):",gc.mem_free(), gc.mem_alloc())

start()

# programs = collect_manifest_paths()
# program_keys = list(programs.keys())
# print(program_keys)
# run_program(program_keys[0], programs[program_keys[0]])
