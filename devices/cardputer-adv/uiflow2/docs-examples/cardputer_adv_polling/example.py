import time
from hardware import MatrixKeyboard

kb = MatrixKeyboard()

while True:
    if kb.is_pressed():
        text = kb.get_string()
        print(text)
    time.sleep_ms(20)
