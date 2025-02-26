import os
import sys
import time
import esp32
import machine
import gc
import _thread

import board
import nav

keys = board.keys()
lcd = board.LCD()

def reset():
    lcd.clear()
    board.clearLights()
    nav.shouldBack(0)


def main():
    reset()
    if(keys == (keys.SHIFT | keys.A | keys.E | keys.F1)):
        os.dupterm(lcd)
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
            while nav.shouldBack(): pass
            
            name = program_keys[index]
            path = programs[program_keys[index]][0]

            sys.path.insert(0, path)
            
            pre_path = os.getcwd()
            
            # Backup sys.modules before importing
            old_modules = sys.modules.copy()

            # Remove _program and its dependencies
            for mod in list(sys.modules):
                if mod.startswith("_program") or mod.startswith(name):
                    del sys.modules[mod]

            os.chdir(path)
            reset()

            print(f"Launching {name} from {path}")
            print("----------------")
            try:
                __import__("_program", None, None, [])
            except Exception as e:
                print(f"Failed in program [{name}]")
                sys.print_exception(e)

            finally:
                print("----------------")
                sys.path.pop(0)
                sys.modules.clear()
                sys.modules.update(old_modules)
                os.chdir(pre_path)
                gc.collect()
                print("Post program - MEM (free,alloc):",gc.mem_free(), gc.mem_alloc())

            reset()
            selectProgram(index)
            while nav.shouldBack(): pass

        elif turns:
            index = (index + turns) % len(program_keys)
            selectProgram(index)
            keys.clearDown(keys.N5)
        time.sleep(0.01)

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
                manifest_content = None

                program_path = item_path + "/_program.py"
                progname = item

                print(progname,item)
                try:
                    with open(manifest_path, "r") as file:
                        manifest_content = file.readline().strip()  # Read program name
                except:
                    pass
                try:
                    with open(program_path, "r") as file:
                        pass
                    progname = item_path.split('/')[-1]
                    manifest_dict[progname] = (item_path,program_path,manifest_content)
                except:
                    pass
        except OSError:
            pass  # Ignore if directory or manifest doesn't exist

    return manifest_dict

def resetWatchdog():
    ticks = 0
    while True:
        time.sleep(1)
        if nav.shouldBack():
            ticks += 1
            if ticks == 7:
                reset()
                machine.reset()
        else:
            ticks = 0;

_thread.start_new_thread(resetWatchdog, ())

main()
