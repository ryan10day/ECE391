/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* void i8259_init(void);
 * Inputs: void
 * Return Value: none
 * Function: initialize the 8259 PIC */
/* Initialize the 8259 PIC */
void i8259_init(void) {

    // unsigned int flags;
    // cli_and_save(flags);
    cli();
    master_mask = 0xff;
    slave_mask = 0xff;
    //send out control words for master pic
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    //send out control words for slave pic
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);
    //mask all interrupt
    outb(master_mask,MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
    sti();
    // restore_flags(flags);
}

/* void enable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function: Enable (unmask) the specified IRQ */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    //check for invalid irq num
    if (irq_num>(2*PIC_WIDTH)){
        return;
    }
    //if in master, mask master pic
    if(irq_num<PIC_WIDTH){
        master_mask &= ~(1<<irq_num);
        outb(master_mask,MASTER_8259_DATA);
    }else{
        //if in slave, enable master pic irq2 and update slave pic mask
        master_mask &= ~(1<<2);
        outb(master_mask,MASTER_8259_DATA);
        slave_mask &= ~(1<<(irq_num-PIC_WIDTH));
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* void disable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function: Disable (mask) the specified IRQ */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    //check for invalid irq num
    if (irq_num>(2*PIC_WIDTH)){
        return;
    }
    //if in master, mask master pic
    if(irq_num<PIC_WIDTH){
        master_mask |= (1<<irq_num);
        outb(master_mask,MASTER_8259_DATA);
    }else{
         //if in slave, enable master pic irq2 and update slave pic mask
        slave_mask |= (1<<(irq_num-PIC_WIDTH));
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function: Send end-of-interrupt signal for the specified IRQ */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    //check for invalid irq num
    if (irq_num>(2*PIC_WIDTH)){
        return;
    }
    //if in slave, send EOI on both
    if(irq_num>=PIC_WIDTH){
        outb(EOI|(irq_num-PIC_WIDTH),SLAVE_8259_PORT);
        outb(EOI|2, MASTER_8259_PORT);
    }else{
        //if in master, send EOI on master
        outb(EOI|irq_num, MASTER_8259_PORT);
    }
}
