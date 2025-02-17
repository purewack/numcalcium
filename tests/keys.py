import keys
import time

k = keys.Keys()

while True:
    print(k.getAllDown())
    print(k.getAllUp())
    time.sleep(0.5)