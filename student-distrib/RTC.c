#include "RTC.h"
#include "lib.h"
#include "system_call.h"
/* void RTC_init();
 * Inputs: none
 * Return Value: none
 * Function: init the RTC */
volatile int counter = 0;
int freq[3] = {2,2,2};
int flag[3] = {0,0,0};

int RTC_init(){
    cli();			// disable interrupts
    outb(RTC_B,RTC_INDEX );		// select register B, and disable NMI
    char prev=inb(RTC_DATA);	// read the current value of register B
    outb( RTC_B,RTC_INDEX);		// set the index again (a read will reset the index to register D)
    outb(prev | 0x40,RTC_DATA );	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_irq(RTC_IRQ);

    outb(RTC_A,RTC_INDEX);		// set index to register A, disable NMI
    prev=inb(RTC_DATA);	// get initial value of register A
    outb(RTC_A,RTC_INDEX);		// reset index to A
    outb((prev & 0xF0) | MAX_HZ,RTC_DATA); //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
    return 0;
}

/* void RTC_handler();
 * Inputs: none
 * Return Value: none
 * Function: calls test_interrupts  */
int RTC_handler(){
    int i;
    counter ++;
    for(i = 0; i<3;i++){   // cycle through frequencies and set flags for reading
        if(counter%(1024/freq[i])==0){
            flag[i] = 1;
        }
    }
    counter %= 2048;
    cli();
    outb( RTC_C,RTC_INDEX);	// select register C
    inb(RTC_DATA);		// just throw away contents
    sti();
    send_eoi(RTC_IRQ);
    return 0;
}
/* void RTC_open();
 * Inputs: filename: irrelevant for now
 * Return Value: 0 if success
 * Function: reset frequency to 1024hz  */
int RTC_open(const uint8_t* filename){
    cli();
    outb(RTC_A,RTC_INDEX);		// set index to register A, disable NMI
    char prev=inb(RTC_DATA);	// get initial value of register A
    outb(RTC_A,RTC_INDEX);		// reset index to A
    outb((prev & 0xF0) | MAX_HZ,RTC_DATA); //write only our rate to A. Note, rate is the bottom 4 bits.
    sti();
    return 0;
}


/* void RTC_close();
 * Inputs: fd: irrlevant for now
 * Return Value: 0 if success
 * Function: does nothing  */
int RTC_close(int32_t fd){
    
    flag[0] = 0;
    flag[1] = 0;
    flag[2] = 0;
    return 0;
}
/* void RTC_read();
 * Inputs: fd:  irrelevant
            buf: pointer to frequency of the interrupt
            nbyte: irrelevant
 * Return Value: 0 if interrupts occur
                -1 if invalid argument
 * Function: return once RTC interrupt occurs, returns 0  */
int RTC_read(int32_t fd, void* buf, int32_t nbytes){


    while(flag[terminal_running-1]==0){
    }
    flag[terminal_running-1] = 0;
    
    return 0;
}

/* void RTC_write();
 * Inputs: fd:  irrelevant
            buf: pointer to frequency of the interrupt
            nbyte: irrelevant
 * Return Value: 0 if success
 * Function: does nothing since RTC is virtualized  */
int RTC_write(int32_t fd, const void* buf, int32_t nbytes){
    int x = *(int*)buf;     //get frequency needed
    if(x==0){
        return -1;
    }
    if((x&(x-1))!=0){       //if not power of two, return fail
        return -1;
    }
    
    freq[terminal_running-1] = x;
    return 0;
}
