import time
from hardware import MatrixKeyboard

kb = MatrixKeyboard()

def key_event(_):
    key = kb.get_key()
    if key is not None:
        print(key, chr(key))

kb.set_callback(key_event)

while True:
    time.sleep_ms(20)
