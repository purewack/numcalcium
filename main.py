import os
import sys
import time
import esp32
import machine
import gc
import _thread

import board
import nav

keys = board.keys

def reset(lcd):
    lcd.color(lcd.WHITE)
    lcd.background(lcd.BLACK)
    lcd.clear()
    board.clearLights()
    nav.shouldBack(0)


def main():
    board.clearLights()
    nav.shouldBack(0)
    if(keys == (keys.SHIFT | keys.A | keys.E | keys.F1)):
        os.dupterm(lcd)
        print("Non-standard Boot")
        return

    lcd = board.LCD()
    nav.shouldBack(0)
    print("Standard Boot")

    programs = findPrograms()
    print(len(programs),[a['name'] for a in programs])  
    if not len(programs):
        lcd.print("No Programs")
        return

    index = 0
    offset = 0

    def listPrograms(programs, offset=0):
        lcd.clear()
        lcd.scale(2)
        lcd.fill(0,0,320,13*2,'#291475')
        lcd.color(lcd.WHITE)
        lcd.background('#291475')
        lcd.print("Apps - NumCalcium")
        lcd.background(lcd.BLACK)
        for i in range(offset, min(offset + 5, len(programs))):
            lcd.cursor(1.5,i+1.25-offset)
            
            rmt = programs[i]['remote']
            lcd.background('#8aebb9' if rmt else lcd.BLACK)
            lcd.color('#076334' if rmt else lcd.WHITE)
            lcd.print(f"{'@' if rmt else ''}{programs[i]['name']}"[:18])
            lcd.color(lcd.WHITE)
            lcd.background(lcd.BLACK)
        # if(len(programs) > 5 and offset < len(programs)-5):
        #     lcd.scale(1)
        #     lcd.cursor(8,12)
        #     lcd.print("...")
    
    def selectProgram(programs, index):
        lcd.scale(2)
        lcd.fill(0, 13*2, 8*2, 170-(13*2), lcd.BLACK)
        for i in range(offset, min(offset + 5, len(programs))):
            if i == index:
                lcd.cursor(0,i+1.25-offset)
                lcd.print(">")

    listPrograms(programs)
    selectProgram(programs,0)
    if not len(programs):
        return

    while True:
        turns = nav.turns()
        if nav.wasBackRequested() or keys.isDown(keys.A):  # Select and run program
            while nav.shouldBack(): pass
            keys.clearAll()
            
            name = programs[index]['name']
            path = programs[index]['path']

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
                lcd.cursor(0,2)
                lcd.color(lcd.WHITE)
                os.dupterm(lcd)
                print(f"--Failed in program--\n\r{name} [{path}]")
                lcd.cursor(0,5)
                sys.print_exception(e)
                os.dupterm(None)
                lcd.background('#c66600')
                lcd.cursor(0,0)
                lcd.print("Press any button to reset")

                keys.clearAll()
                while not (nav.shouldBack() or keys.isAnyDown()):
                    time.sleep(0.1)
                    board.statusLed(10,0,0)
                    time.sleep(0.1)
                    board.statusLed(0,0,0)
                while nav.shouldBack(): pass
                keys.clearAll()

            finally:
                keys.clearAll()
                while nav.shouldBack(): pass
                print("----------------")
                sys.path.pop(0)
                sys.modules.clear()
                sys.modules.update(old_modules)
                os.chdir(pre_path)
                gc.collect()
                print("Post program - MEM (free,alloc):",gc.mem_free(), gc.mem_alloc())
                
            reset(lcd)
            listPrograms(programs,offset)
            selectProgram(programs, index)

        elif turns or keys.isDown(keys.N5) or keys.isDown(keys.N2):
            prev_offset = offset
            _index = -1 if turns < 0 else 1
            if keys == keys.N2:
                _index = 1
            elif keys == keys.N5:
                _index = -1
            index = max(0,min((index + _index),len(programs)-1))
            if index%5 == 0 and (turns > 0 or keys == keys.N2):
                offset = offset + 5
            elif index%5 == 4 and (turns < 0 or keys == keys.N5):
                offset = max(0, offset - 5)
            if prev_offset != offset:
                listPrograms(programs,offset)
            selectProgram(programs, index)
            print(index,offset)
            keys.clearAll()
        
        elif keys.isDown(keys.E):
            try:
                items = os.listdir('/remote')
                programs = findPrograms()
                offset = 0
                index = 0
                listPrograms(programs,offset)
                selectProgram(programs, index)
            except:
                pass

        time.sleep(0.01)

def collect_manifest_paths(base_path):
    manifest_dict = []
    try:
        items = os.listdir(base_path)
    except OSError:
        return manifest_dict

    for item in items:
        item_path = base_path + "/" + item
        try:
            stat = os.stat(item_path)
            if stat[0] & 0x4000:  # Directory check
                found = False
                try:
                    with open(item_path + "/_program.py", "r") as file:
                        pass
                    found = True
                except OSError:
                    pass
                try:
                    with open(item_path + "/_program.mpy", "r") as file:
                        pass
                    found = True
                except OSError:
                    pass

                if(found):
                    manifest_dict.append({
                        'path':item_path,
                        'name':item,
                        'remote':item_path.startswith('/remote')
                    })
        except OSError:
            pass  # Ignore if directory or manifest doesn't exist
    return manifest_dict

def findPrograms():
    programs = collect_manifest_paths("/programs")
    programs.extend(collect_manifest_paths("/remote"))
    return programs

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
