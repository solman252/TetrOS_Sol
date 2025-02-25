#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

//enumeration for non-ascii special keys
typedef enum {
    KEY_UNKNOWN,
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ENTER,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LALT,
    KEY_RALT,
    KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
} key_code_t;

//initializes the keyboard driver
void init_keyboard_driver();

//main keyboard interrupt handler, called from assembly
void keyboard_handler();

//callback functions for key presses
void keyboard_handle_key(char ascii);
void keyboard_handle_special(key_code_t key);

#endif //KEYBOARD_H
