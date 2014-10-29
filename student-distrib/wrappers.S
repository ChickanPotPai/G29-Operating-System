# interrupt and exception wrappers

kernel_ds:
	.long 0x0018

.text

.globl keyboard_wrapper, rtc_wrapper, syscall_wrapper

keyboard_wrapper:
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	pushl	%esp
	pushl	%ebp

	pushl	%esi
	pushl	%edi

    pushw 	%ds
    pushw 	%es
    pushw 	%fs
    pushw 	%gs

	call keyboard_handler
	
	popw 	%gs
    popw 	%fs
    popw 	%es
    popw 	%ds
    
	popl	%edi
	popl	%esi

	popl	%ebp
	popl	%esp

	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax

	iret

rtc_wrapper:
	pushal
	call rtc_handler
	popal
	iret

syscall_wrapper:
	cmpl	$0, %eax
	jle		fail
	cmpl	$10, %eax
	jg		fail

	pushl %edi
	pushl %esi

	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs

	pushl %edx
	pushl %ecx
	pushl %ebx

	movl kernel_ds, %edx
	movl %edx, %ds

	decl	%eax
	call	*system_calls_table(,%eax,4)

	popl %ebx
	popl %ecx
	popl %edx

	popl %gs
	popl %fs
	popl %es
	popl %ds

	popl %esi
	popl %edi

	iret

fail:
	movl $-1, %eax
	iret

system_calls_table:
	.long system_halt, system_execute, system_read, system_write, system_open, system_close, system_getargs, system_vidmap, system_set_handler, system_sigreturn
