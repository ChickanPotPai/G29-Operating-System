// rtc functions

#include "rtc.h"

#define UPPER_MASK	0xF0
#define SET 		1
#define CLEAR 		0

// flag to track interrupts
volatile static uint8_t rtc_flag = CLEAR;
static uint8_t rtc_open_flag = 0;

void rtc_init() {
	uint8_t prev;
	char save;

	outb(REG_B, RTC_INDEX);
	prev = inb(RTC_BYTE);
	outb(REG_B, RTC_INDEX);
	outb(prev | RTC_MASK, RTC_BYTE);

	// retrieve previous value of register A
	outb(REG_A, RTC_INDEX);
	save = inb(RTC_BYTE);	

	// write out base rate of 2 Hz
	outb(REG_A, RTC_INDEX);	
	outb((save & UPPER_MASK) | BASE_RATE, RTC_BYTE);

	enable_irq(RTC_IRQ);
}

void rtc_handler() {
	disable_irq(RTC_IRQ);

	// set flag
	rtc_flag = SET;

	// read register C to re-enable rtc interrupts
	outb(REG_C_BLK, RTC_INDEX);
	(void) inb(RTC_BYTE);
	
	send_eoi(RTC_IRQ);
    enable_irq(RTC_IRQ);
}

int32_t rtc_open(const uint8_t* filename, pcb_t* curr_pcb) {
	if (rtc_open_flag == 1) {return -1;}
	// rtc_open_flag = 1;
	//enable_irq(RTC_IRQ);
	return 0;
}

int32_t rtc_write(uint32_t fd, const uint8_t* buf, uint32_t nbytes) {
	int i;
	char save;
	uint16_t rate = RATE_MAX;
	uint16_t freq = buf[0] + (buf[1] << 8);

	for(i = 0; i < RATE_MAX; i++) {
		freq /= 2;
		if ((freq == 1) | (freq == 0)) {break;}
	}

	rate = RATE_MAX - i;
	disable_irq(RTC_IRQ);

	// retrieve previous value of register A
	outb(REG_A, RTC_INDEX);
	save = inb(RTC_BYTE);	

	// write out rate
	outb(REG_A, RTC_INDEX);	
	outb((save & UPPER_MASK) | rate, RTC_BYTE);

	enable_irq(RTC_IRQ);

	return 0;
}

int32_t rtc_close(int32_t fd, pcb_t* curr_pcb) {
	rtc_open_flag = 0;

	curr_pcb->file_array[fd].fops_ptr = NULL;
	curr_pcb->file_array[fd].inode_ptr = NULL;
	curr_pcb->file_array[fd].file_position = 0;
	curr_pcb->file_array[fd].flags = 0;

	return 0;
}

int32_t rtc_read(uint32_t fd, const uint8_t* buf, uint32_t nbytes) {
	volatile uint8_t temp_flag = CLEAR;

	sti();

	while (!temp_flag) {temp_flag = rtc_flag;}
	rtc_flag = CLEAR;

	return 0;
}
