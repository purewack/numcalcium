import terminal

terminal.console(False)
terminal.clear()
terminal.color(terminal.WHITE)
terminal.cursor(48,48)
terminal.fill(32,32)

terminal.color(terminal.GREEN)
terminal.cursor(48,48)
terminal.fill(16,16)

terminal.color(terminal.GREEN)
for i in range(128):
    terminal.cursor(i,i)
    terminal.plot()

terminal.color(terminal.RED)
for i in range(128):
    terminal.cursor(128-i,i)
    terminal.plot()

terminal.color(terminal.BLUE)
for i in range(128):
    terminal.cursor(64,i)
    terminal.plot()

terminal.cursor(0,0)
terminal.print("hello HELLO\n123 !#@\n\rYO deleted\b\b\ro")

input()
