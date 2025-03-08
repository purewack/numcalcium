import os
import sys
import time
import esp32
import machine
import math
import gc
import _thread

import board
import nav


def reset(lcd):
    lcd.color(lcd.WHITE)
    lcd.background(lcd.BLACK)
    lcd.clear()
    lcd.setBacklight(127)
    board.clearLights()
    nav.shouldBack(0)


def main():
    keys = board.keys
    lcd = board.LCD()   
    lcd.options(scale=1,background=0,foreground=0xffff)
    lcd.reset()
    board.clearLights()
    nav.shouldBack(0)
    if(keys == (keys.SHIFT | keys.A | keys.E | keys.F1)):
        os.dupterm(lcd)
        print("Non-standard Boot")
        return

    nav.shouldBack(0)
    print("Standard Boot")

    programs = findPrograms()
    print(len(programs),[a['name'] for a in programs])  
    if not len(programs):
        lcd.print("No Programs")
        return

    index = 0
    offset = 0

    previous_index = None  # Tracks last selected index
    previous_offset = None  # Tracks last offset to detect page scrolls

    def listPrograms(programs, offset=0, selected_index=None, previous_index=None, force_redraw=False):
        nonlocal previous_offset
        lcd.scale(2)

        total_items = len(programs)
        visible_items = 5  # Number of items visible at a time
        pages = math.ceil(total_items / visible_items) 

        # Determine if we need a scroll bar
        scrollbar_needed = total_items > visible_items
        scrollbar_x = 320 - 4  # 4px wide scrollbar at x = 4
        scrollbar_y = 13*2
        scrollbar_h = 170-scrollbar_y 

        # Calculate scrollbar position
        if scrollbar_needed:
            bar_h = scrollbar_h / pages # Ensure a minimum size
            bar_y = scrollbar_y + (scrollbar_h/pages) * (offset/visible_items)
        else:
            bar_h = 0

        # Full redraw if offset changed or explicitly forced
        if force_redraw or offset != previous_offset:
            lcd.clear()
            lcd.fill(0, 0, 320, 13 * 2, '#291475')
            lcd.color(lcd.WHITE)
            lcd.background('#291475')
            lcd.print("Apps - NumCalcium")
            lcd.background(lcd.BLACK)

            # Draw all visible entries
            for i in range(offset, min(offset + visible_items, total_items)):
                lcd.cursor(1.5, i + 1.25 - offset)
                rmt = programs[i]['remote']
                is_selected = (i == selected_index)
                lcd.background('#FFD700' if is_selected else ('#8aebb9' if rmt else lcd.BLACK))
                lcd.color(lcd.BLACK if is_selected else ('#076334' if rmt else lcd.WHITE))
                lcd.print(f"{'@' if rmt else ''}{programs[i]['name']} "[:18])

            # Draw scroll bar
            lcd.fill(scrollbar_x, scrollbar_y, 4, scrollbar_h, '#333333')  # Clear previous bar area
            if scrollbar_needed:
                lcd.fill(scrollbar_x, int(bar_y), 4, int(bar_h), lcd.WHITE)  # Draw scroll bar

            lcd.background(lcd.BLACK)  # Reset
            previous_offset = offset  # Update offset tracking
            return  # Done, no need for partial update

        # If offset is the same, update only changed lines
        for i in [previous_index, selected_index]:
            if i is None or i < offset or i >= min(offset + visible_items, total_items):
                continue  # Skip out-of-bounds updates

            lcd.cursor(1.5, i + 1.25 - offset)
            rmt = programs[i]['remote']
            is_selected = (i == selected_index)
            lcd.background('#FFD700' if is_selected else ('#8aebb9' if rmt else lcd.BLACK))
            lcd.color(lcd.BLACK if is_selected else ('#076334' if rmt else lcd.WHITE))
            lcd.print(f"{'@' if rmt else ''}{programs[i]['name']} "[:18])

        # Update scroll bar position only if scrolling within the same page
        if scrollbar_needed:
            lcd.fill(scrollbar_x, scrollbar_y, 4, scrollbar_h, '#333333')  # Clear old bar
            lcd.fill(scrollbar_x, int(bar_y), 4, int(bar_h), lcd.WHITE)  # Draw new bar

        lcd.background(lcd.BLACK)  # Reset after rendering


    def selectProgram(programs, index, offset=0):
        nonlocal previous_index
        listPrograms(programs, offset, selected_index=index, previous_index=previous_index, force_redraw=(offset != previous_offset))
        previous_index = index  # Update tracking


    listPrograms(programs,0,0,0,True)
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
            listPrograms(programs,offset,index, previous_index,True)
            selectProgram(programs, index, offset)

        elif len(programs) >1 and turns or keys.isDown(keys.N5) or keys.isDown(keys.N2):
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
            selectProgram(programs, index,offset)
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
