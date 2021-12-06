
.text


.globl system_call_80

system_call_80:

    pushl %ecx       # push all register except eax
	pushl %edx
    pushl %ebx
	pushl %esi
	pushl %edi
    pushfl           # push all flags pushfl
    
    cmpl $1, %eax    # check whether eax smaller then 1
    jl invalid_command
    cmpl $10, %eax    # check whether eax greater then 10
    jg invalid_command

    pushl %edx       # push the third argument
    pushl %ecx       # push the second argument
    pushl %ebx       # push the first argument
    addl $-1, %eax
    call *jump_table(, %eax, 4)
    popl %ebx
    popl %ecx
    popl %edx
    jmp done

invalid_command:
    movl $-1, %eax

done: 
    popfl            # pop all flags popfl
    popl %edi        # pop register except eax 
    popl %esi
    popl %ebx
    popl %edx
    popl %ecx
    iret

jump_table:
.long system_call_halt, system_call_execute, system_call_read, system_call_write, system_call_open, system_call_close, system_call_getargs, system_call_vidmap, system_call_set_handler, system_call_sigreturn
