#include "PIT.h"
#include "lib.h"
#include "scheduler.h"
#include "system_call.h"

/* void PIT_init();
 * Inputs: none
 * Return Value: none
 * Function: init the PIT */

int PIT_init(){
    //cli();  
    outb(0x36, PIT_CMD);             
    outb(SWITCH_FREQ & 0xFF, PIT_DATA);   
    outb(SWITCH_FREQ >> 8, PIT_DATA);
    enable_irq(PIT_IRQ);     
    //sti();
    return 0;
}

/* void PIT_handler();
 * Inputs: none
 * Return Value: none
 * Function: calls test_interrupts  */
int PIT_handler(){
    //test_interrupts();
    send_eoi(PIT_IRQ);
    target_terminal = (terminal_running % 3) + 1;   // run the round-robin cycle 
    scheduler();
    
    return 0;
}
/* void PIT_open();
 * Inputs: filename: irrelevant for now
 * Return Value: 0 if success
 * Function: reset frequency to 1024hz  */
int PIT_open(const uint8_t* filename){
    return 0;
}


/* void PIT_close();
 * Inputs: fd: irrlevant for now
 * Return Value: 0 if success
 * Function: does nothing  */
int PIT_close(int32_t fd){
    return 0;
}

/* void PIT_read();
 * Inputs: fd:  irrelevant
            buf: pointer to frequency of the interrupt
            nbyte: irrelevant
 * Return Value: 0 if interrupts occur
                -1 if invalid argument
 * Function: return once PIT interrupt occurs, returns 0  */
int PIT_read(int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

/* void PIT_write();
 * Inputs: fd:  irrelevant
            buf: pointer to frequency of the interrupt
            nbyte: irrelevant
 * Return Value: 0 if success
 * Function: does nothing since PIT is virtualized  */
int PIT_write(int32_t fd, const void* buf, int32_t nbytes){
    return 0;
}
