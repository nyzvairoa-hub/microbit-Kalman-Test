def on_button_pressed_a():
    serial.write_number(banana.get_accel_x())
input.on_button_pressed(Button.A, on_button_pressed_a)
