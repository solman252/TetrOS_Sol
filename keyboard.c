#include "keyboard.h"
#include "kernel.h" //for inb() and outb() functions.
#include <stdbool.h>

//modifier state
static bool shift = false;
static bool ctrl = false;
static bool alt = false;
static bool caps_lock = false;

//mapping table for standard scancodes
static const char key_map[128] = {
    [0x00] = 0,
    [0x01] = 27,         //esc
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',       //backspace
    [0x0F] = '\t',       //tab
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',       //enter
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' ',        //space
    //other indices default to 0.
};

//mapping table for scancodes when shift is held
static const char key_map_shift[128] = {
    [0x00] = 0,
    [0x01] = 27,
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '{',
    [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ':',
    [0x28] = '\"',
    [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
    [0x39] = ' ',
};

/* converts a scancode to an ascii character, taking into account modifiers. */
static char convert_scancode_to_ascii(unsigned char code) {
    char ascii = 0;
    if (shift)
        ascii = key_map_shift[code];
    else
        ascii = key_map[code];
    /* for alphabetical keys, if caps lock is active and shift is not, convert to uppercase */
    if (!shift && caps_lock && ascii >= 'a' && ascii <= 'z')
        ascii = ascii - 'a' + 'A';
    return ascii;
}

/* initializes the keyboard driver state. */
void init_keyboard_driver() {
    shift = false;
    ctrl = false;
    alt = false;
    caps_lock = false;
}

/* processes a single scancode for standard keys, handling modifiers. */
static void process_scancode(unsigned char scancode) {
    bool key_release = scancode & 0x80;
    unsigned char code = scancode & 0x7F;

    /* handle modifier keys */
    switch (code) {
        case 0x2A:  //left shift
        case 0x36:  //right shift
            shift = !key_release;
            return;
        case 0x1D:  //left ctrl
            ctrl = !key_release;
            return;
        case 0x38:  //left alt
            alt = !key_release;
            return;
        case 0x3A:  //caps lock (toggle on key press)
            if (!key_release)
                caps_lock = !caps_lock;
            return;
        default:
            break;
    }
    if (key_release)
        return;  //ignore key releases for non-modifiers.

    char ascii = convert_scancode_to_ascii(code);
    if (ascii)
        keyboard_handle_key(ascii);
}

/* main keyboard interrupt handler. */
void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    if (scancode == 0xE0) {
        unsigned char ext_code = inb(0x60);
        /* handle common extended keys (e.g., arrow keys). */
        switch (ext_code) {
            case 0x48:  //up arrow
                keyboard_handle_special(KEY_UP);
                break;
            case 0x50:  //down arrow
                keyboard_handle_special(KEY_DOWN);
                break;
            case 0x4B:  //left arrow
                keyboard_handle_special(KEY_LEFT);
                break;
            case 0x4D:  //right arrow
                keyboard_handle_special(KEY_RIGHT);
                break;
            default:
                break;
        }
    } else {
        process_scancode(scancode);
    }
    //send end-of-interrupt (eoi) to the pic so further interrupts can be processed.
    outb(0x20, 0x20);
}
