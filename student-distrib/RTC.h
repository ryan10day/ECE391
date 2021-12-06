#include "x86_desc.h"
#include "i8259.h"

#define RTC_IRQ      0x08
#define RTC_INDEX   0x70
#define RTC_DATA    0x71
#define TWO_HZ 0xF
#define MAX_HZ 0x6
#define RTC_A 0x8A
#define RTC_B 0x8B
#define RTC_C 0x0C
/*initializes the RTC and set to default frequency*/
int RTC_init();

/*handles rtc interrupt, calls test_interrupt for now*/
int RTC_handler();

/*reset frequency to 1024hz*/
int RTC_open(const uint8_t* filename);

/*does nothing*/
int RTC_close(int32_t fd);

/*return once RTC interrupt occurs, returns 0*/
int RTC_read(int32_t fd, void* buf, int32_t nbytes);

/*change frequency, get its input through a buffer and not read the value directly, four byte integer rate in HZ*/
int RTC_write(int32_t fd, const void* buf, int32_t nbytes);

