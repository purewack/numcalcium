import board

terminal = board.LCD()
terminal.clear()

terminal.fill(48,48,16,16,terminal.GREEN)

for i in range(128):
    terminal.plot(i,i,terminal.GREEN)

for i in range(128):
    terminal.plot(128-i,i,terminal.RED)

for i in range(128):
    terminal.plot(64,i,terminal.BLUE)

terminal.cursor(0,0)
terminal.color(terminal.WHITE)
terminal.print("hello HELLO\n123 !#@\n\rYO deleted\b\b\ro")

