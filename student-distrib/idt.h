#include "x86_desc.h"

#define SYSTEM_CALL_INDEX 128  /* 128 corresponds to 0x80*/

/* functions used for SET_IDT_ENTRY function */
void exception_00();
void exception_01();
void exception_02();
void exception_03();
void exception_04();
void exception_05();
void exception_06();
void exception_07();
void exception_08();
void exception_09();
void exception_10();
void exception_11();
void exception_12();
void exception_13();
void exception_14();
void exception_16();
void exception_17();
void exception_18();
void exception_19();

void interrupt_32();
void interrupt_33();
void interrupt_40();

void system_call_80();

void system_call();

extern void do_irq(int irq);

void init_idt();
