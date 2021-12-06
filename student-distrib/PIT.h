#include "x86_desc.h"
#include "i8259.h"

#define PIT_IRQ      0x00
#define PIT_DATA     0x40
#define PIT_CMD      0x43
#define SWITCH_FREQ  (1193182/50)
/*initializes the PIT and set to default frequency*/
int PIT_init();

/*handles PIT interrupt, calls test_interrupt for now*/
int PIT_handler();

/*reset frequency to 1024hz*/
int PIT_open(const uint8_t* filename);

/*does nothing*/
int PIT_close(int32_t fd);

/*return once PIT interrupt occurs, returns 0*/
int PIT_read(int32_t fd, void* buf, int32_t nbytes);

/*change frequency, get its input through a buffer and not read the value directly, four byte integer rate in HZ*/
int PIT_write(int32_t fd, const void* buf, int32_t nbytes);

