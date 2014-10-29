# interrupt and exception wrappers

.text

.globl keyboard_wrapper, rtc_wrapper

keyboard_wrapper:
	pushal
	call keyboard_handler
	popal
	iret

rtc_wrapper:
	pushal
	call rtc_handler
	popal
	iret
