kernel_ds:
	.long 0x0018

.globl syscall_handler

# save all registers and call, then pop and iret
syscall_handler:

	pushl	%ebp
	movl	%esp,%ebp
	
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

	pushl %esp
	pushl %ebp

	movl kernel_ds, %edx
	movl %edx, %ds

	decl	%eax
	call	*system_calls_table(,%eax,4)

	pushl %ebp
	pushl %esp

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




