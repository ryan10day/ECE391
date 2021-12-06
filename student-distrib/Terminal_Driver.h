#include "types.h"


#define KEYBOARD_BUF_SIZE 128
#define NUM_COMMAND 5

#define VIDEO                 0xB8000
#define TERMINAL1_VIDEO       0xB9000
#define TERMINAL2_VIDEO       0xBA000
#define TERMINAL3_VIDEO       0xBB000
#define FOUR_KILA             0x01000

/* set extern variable for keyboard buffer, keyboard index and keyboard flag */
//unsigned char keyboard_buffer[KEYBOARD_BUF_SIZE];
//extern volatile unsigned int keyboard_buffer_index;
//extern volatile int keyboard_flag;
//extern unsigned char command_buffer[NUM_COMMAND][KEYBOARD_BUF_SIZE];
//extern int command_idx;

int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_open (const uint8_t* filename);
int32_t terminal_close (int32_t fd);

