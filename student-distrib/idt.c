#include "lib.h"

#include "x86_desc.h"
#include "idt.h"

#include "keyboard.h"

#include "RTC.h"

#include "PIT.h"

#include "system_call.h"

/* void do_irq(int irq);
 * Inputs: int irq
 * Return Value: none
 * Function: call the function we want based on given irq */
void do_irq(int irq)
{
    /* Exception we have based on p145 in 
    *IA-32 Intel® Architecture Software Developer’s Manual
    */
    switch(irq)
    {
        /* case 0 to 19 are exceptions with infinite loop*/
        case 0:
            printf("Exception: Divide Error \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 1:
            printf("Exception: Reserved \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 2:
            printf("Exception: NMI Interrupt \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 3:
            printf("Exception: Breakpoint \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 4:
            printf("Exception: Overflow \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 5:
            printf("Exception: BOUND Range Exceeded \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 6:
            printf("Exception: Invalid Opcode (Undefined Opcode) \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 7:
            printf("Exception: Device Not Available \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 8:
            printf("Exception: Double Fault \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 9:
            printf("Exception: Coprocessor Segment Overrun (reserved) \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 10:
            printf("Exception: Invalid TSS \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 11:
            printf("Exception: Segment Not Present \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 12:
            printf("Exception: Stack-Segment Fault \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 13:
            printf("Exception: General Protection \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 14:
            printf("Exception: Page Fault \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 16:
            printf("Exception: x87 FPU Floating-Point Error (Math Fault) \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 17:
            printf("Exception: Alignment Check \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 18:
            printf("Exception: Machine Check \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        case 19:
            printf("Exception: SIMD Floating-Point Exception \n");
            system_call_halt (63); // random number pass to system call halt
            break;
            // while(1);
        /* case 32 is used for PIT */
        case 32:
            PIT_handler();
            break;
        /* case 33 is used for keyboard interreupt */
        case 33:
            keyboard_get_char();
            break;
        /* case 40 is used for RTC interrupt */
        case 40:
            RTC_handler();
            break;
        case 128:
            printf("system call");
            // while(1);
        default:
        break;
    }

}

void init_idt()
{
    int index;
    /* we set initial value for first 32 which are all exeptions */
    for(index = 0; index < 32; index ++)
    {
        idt[index].seg_selector = KERNEL_CS;
        idt[index].size         = 0x1;
        idt[index].dpl          = 0x0;
        idt[index].present      = 0x0;
        idt[index].reserved4        = 0;
        idt[index].reserved3        = 0;
        idt[index].reserved2        = 1;
        idt[index].reserved1        = 1;
        idt[index].reserved0        = 0;
    }
    /* we set initial value for interrupt, which corrosponds to 32-255 in idt*/
    for(index = 32; index < 256; index ++)
    {
        idt[index].seg_selector = KERNEL_CS;
        idt[index].size         = 0x1;
        idt[index].dpl          = 0x0;
        idt[index].present      = 0x0;
        idt[index].reserved4        = 0;
        idt[index].reserved3        = 0;
        idt[index].reserved2        = 1;
        idt[index].reserved1        = 1;
        idt[index].reserved0        = 0;
    }

    /* set idt offset and present value for exceptions */
    SET_IDT_ENTRY(idt[0], exception_00);
    SET_IDT_ENTRY(idt[1], exception_01);
    SET_IDT_ENTRY(idt[2], exception_02);
    SET_IDT_ENTRY(idt[3], exception_03);
    SET_IDT_ENTRY(idt[4], exception_04);
    SET_IDT_ENTRY(idt[5], exception_05);
    SET_IDT_ENTRY(idt[6], exception_06);
    SET_IDT_ENTRY(idt[7], exception_07);
    SET_IDT_ENTRY(idt[8], exception_08);
    SET_IDT_ENTRY(idt[9], exception_09);
    SET_IDT_ENTRY(idt[10], exception_10);
    SET_IDT_ENTRY(idt[11], exception_11);
    SET_IDT_ENTRY(idt[12], exception_12);
    SET_IDT_ENTRY(idt[13], exception_13);
    SET_IDT_ENTRY(idt[14], exception_14);
    SET_IDT_ENTRY(idt[16], exception_16);
    SET_IDT_ENTRY(idt[17], exception_17);
    SET_IDT_ENTRY(idt[18], exception_18);
    SET_IDT_ENTRY(idt[19], exception_19);

    /* set idt offset and present value for interrup/devices */
    SET_IDT_ENTRY(idt[32], interrupt_32);
    SET_IDT_ENTRY(idt[33], interrupt_33);
    SET_IDT_ENTRY(idt[40], interrupt_40);

    idt[SYSTEM_CALL_INDEX].seg_selector = KERNEL_CS;
    idt[SYSTEM_CALL_INDEX].size         = 0x1;
    idt[SYSTEM_CALL_INDEX].dpl          = 0x3;
    idt[SYSTEM_CALL_INDEX].present      = 0x0;
    idt[SYSTEM_CALL_INDEX].reserved4        = 0;
    idt[SYSTEM_CALL_INDEX].reserved3        = 1;
    idt[SYSTEM_CALL_INDEX].reserved2        = 1;
    idt[SYSTEM_CALL_INDEX].reserved1        = 1;
    idt[SYSTEM_CALL_INDEX].reserved0        = 0;
    /* set idt offset and present value for system call */
    SET_IDT_ENTRY(idt[SYSTEM_CALL_INDEX], system_call_80);

}

