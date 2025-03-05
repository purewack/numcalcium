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

def reset(lcd):
    lcd.color(lcd.WHITE)
    lcd.background(lcd.BLACK)
    lcd.clear()
    board.clearLights()
    nav.shouldBack(0)


def main():
    os.chdir('/')
    board.clearLights()
    nav.shouldBack(0)
    if(keys == (keys.SHIFT | keys.A | keys.E | keys.F1)):
        os.dupterm(lcd)
        print("Non-standard Boot")
        return

    lcd = board.LCD()
    nav.shouldBack(0)
    print("Standard Boot")
    programs = collect_manifest_paths()
    program_keys = list(programs.keys())
    print(programs)
    if not len(program_keys):
        lcd.print("No Programs")
        return

    index = 0
    def listPrograms(offset=0):
        nonlocal programs, program_keys
        lcd.clear()
        lcd.scale(2)
        lcd.fill(0,0,320,13*2,'#4a00c2')
        lcd.color(lcd.WHITE)
        lcd.background('#4a00c2')
        lcd.print("P R O G R A M S")
        lcd.background(lcd.BLACK)
        for i in range(len(program_keys)):
            lcd.cursor(2,i+1)
            lcd.print(str(i + 1))
            lcd.print(".")
            lcd.print(program_keys[i])
    
    def selectProgram(index):
        lcd.scale(2)
        lcd.fill(0, 13*2, 8*2, 170-(13*2), lcd.BLACK)
        for i in range(len(program_keys)):
            if i == index:
                lcd.cursor(0,i+1)
                lcd.print(">")

    listPrograms()
    selectProgram(0)

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
            reset(lcd)
            title = f"Launching {name} from {path}"
            lcd.scale(1)
            lcd.print(title)
            print(title)
            print("----------------")
            try:
                __import__("_program", None, None, [])
            except Exception as e:
                lcd.background('#e69600')
                lcd.color(lcd.WHITE)
                lcd.clear()
                lcd.print(name," @",path)
                os.dupterm(lcd)
                print(f"Failed in program [{name}]")
                sys.print_exception(e)
                os.dupterm(None)
                lcd.print("\n\r Press any button to reset")
                while not (nav.shouldBack() or keys.isAnyDown()):
                    time.sleep(0.1)
                    board.statusLed(10,0,0)
                    time.sleep(0.1)
                    board.statusLed(0,0,0)
                while nav.shouldBack(): pass
                lcd.background(lcd.BLACK)

            finally:
                print("----------------")
                sys.path.pop(0)
                sys.modules.clear()
                sys.modules.update(old_modules)
                os.chdir(pre_path)
                gc.collect()
                print("Post program - MEM (free,alloc):",gc.mem_free(), gc.mem_alloc())
                
                
            reset(lcd)
            listPrograms()
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
                lcd = board.LCD()
                lcd.clear()
                lcd.__del__()
                board.clearLights()
                machine.reset()
        else:
            ticks = 0;

_thread.start_new_thread(resetWatchdog, ())

main()
