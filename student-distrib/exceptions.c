// exception handler declarations

#include "exceptions.h"
#include "lib.h"


// display error type
void divide_error() {
	clear();
	printf("divide error exception\n");
	while(1) {}
}

void reserved() {
	clear();
	printf("reserved\n");
	while(1) {}
}

void nmi_interrupt() {
	clear();
	printf("NMI interrupt\n");
	while(1) {}
}

void breakpoint() {
	clear();
	printf("breakpoint\n");
	while(1) {}
}

void overflow() {
	clear();
	printf("overflow\n");
	while(1) {}
}

void bound_range_exceeded() {
	clear();
	printf("bound range exceeded\n");
	while(1) {}
}

void invalid_opcode() {
	clear();
	printf("invalid opcode\n");
	while(1) {}
}

void device_not_available() {
	clear();
	printf("device not available\n");
	while(1) {}
}

void double_fault() {
	clear();
	printf("double fault\n");
	while(1) {}
}

void coprocessor_segment_overrun() {
	clear();
	printf("coprocessor segment overrun\n");
	while(1) {}
}

void invalid_tss() {
	clear();
	printf("invalid tss\n");
	while(1) {}
}

void segment_not_present() {
	clear();
	printf("segement not present\n");
	while(1) {}
}

void stack_segment_fault() {
	clear();
	printf("stack segment fault\n");
	while(1) {}
}

void general_protection() {
	// clear();
	printf("general protection\n");
	while(1) {}
}

void page_fault() {
	clear();
	printf("page fault\n");
	while(1) {}
}

void math_fault() {
	clear();
	printf("x87 FPU floating-point error (math fault)\n");
	while(1) {}
}

void alignment_check() {
	clear();
	printf("alignment check\n");
	while(1) {}
}

void machine_check() {
	clear();
	printf("machine check\n");
	while(1) {}
}

void simd_floating_exception() {
	clear();
	printf("SIMD floating-point exception\n");
	while(1) {}
}
