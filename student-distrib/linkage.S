
.text


.globl exception_00, exception_01, exception_02, exception_03, exception_04
.globl exception_05, exception_06, exception_07, exception_08, exception_09
.globl exception_10, exception_11, exception_12, exception_13, exception_14
.globl exception_16, exception_17, exception_18, exception_19
.globl interrupt_32
.globl interrupt_33
.globl interrupt_40

.global system_call

.globl common_interrupt


exception_00:       # first exceptoin in idt
    pushal          # save all regs
    pushl $0        # push the argument (irq)
    jmp common_interrupt


exception_01:
    pushal
    pushl $1 
    jmp common_interrupt

exception_02:
    pushal
    pushl $2 
    jmp common_interrupt

exception_03:
    pushal
    pushl $3 
    jmp common_interrupt

exception_04:
    pushal
    pushl $4 
    jmp common_interrupt

exception_05:
    pushal
    pushl $5 
    jmp common_interrupt

exception_06:
    pushal
    pushl $6 
    jmp common_interrupt

exception_07:
    pushal
    pushl $7 
    jmp common_interrupt

exception_08:
    pushal
    pushl $8 
    jmp common_interrupt

exception_09:
    pushal
    pushl $9 
    jmp common_interrupt

exception_10:
    pushal
    pushl $10 
    jmp common_interrupt

exception_11:
    pushal
    pushl $11 
    jmp common_interrupt

exception_12:
    pushal
    pushl $12 
    jmp common_interrupt

exception_13:
    pushal
    pushl $13 
    jmp common_interrupt

exception_14:
    pushal
    pushl $14 
    jmp common_interrupt

exception_16:
    pushal
    pushl $16 
    jmp common_interrupt

exception_17:
    pushal
    pushl $17 
    jmp common_interrupt

exception_18:
    pushal
    pushl $18 
    jmp common_interrupt

exception_19:
    pushal
    pushl $19 
    jmp common_interrupt

interrupt_32:
    pushal
    pushl $32 
    jmp common_interrupt
    
interrupt_33:
    pushal
    pushl $33 
    jmp common_interrupt
    
interrupt_40:
    pushal
    pushl $40 
    jmp common_interrupt

system_call:
    pushal
    pushl $128 
    jmp common_interrupt

common_interrupt:
    call do_irq    
    addl $4, %esp       # pop the argument (irq)
    popal
    iret
