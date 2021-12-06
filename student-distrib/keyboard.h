#include "x86_desc.h"
#include "i8259.h"

#define KEYBOARD_IRQ      0x01
#define KEYBOARD_DATA     0x60

unsigned char char_typed;
/* initialize keyboard */
void keyboard_init();
/* interrupt handler for keyboard */
extern void keyboard_get_char();
