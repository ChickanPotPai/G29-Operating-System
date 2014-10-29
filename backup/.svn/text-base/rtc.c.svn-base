// rtc functions

#include "lib.h"
#include "rtc.h"
#include "i8259.h"	

#define UPPER_MASK	0xF0

// flag to track interrupts
static uint8_t rtc_flag = 0;

void rtc_init() {
	uint8_t prev;

	outb(REG_B, RTC_INDEX);
	prev = inb(RTC_BYTE);
	outb(REG_B, RTC_INDEX);
	outb(prev | RTC_MASK, RTC_BYTE);
	
	enable_irq(RTC_IRQ);
}

void rtc_handler() {
	disable_irq(RTC_IRQ);

	// set flag
	rtc_flag = 1;

	// read register C to re-enable rtc interrupts
	outb(REG_C_BLK, RTC_INDEX);
	(void) inb(RTC_BYTE);
	
	send_eoi(RTC_IRQ);
    enable_irq(RTC_IRQ);
}

int32_t open_rtc() {
	char save;
	disable_irq(RTC_IRQ);

	// retrieve previous value of register A
	outb(REG_A, RTC_INDEX);
	save = inb(RTC_BYTE);	

	// write out base rate of 2 Hz
	outb(REG_A, RTC_INDEX);	
	outb((save & UPPER_MASK) | BASE_RATE, RTC_BYTE);

	enable_irq(RTC_IRQ);

	return 0;
}

int32_t write_rtc(const char* buf) {
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

	// write out base rate of 2 Hz
	outb(REG_A, RTC_INDEX);	
	outb((save & UPPER_MASK) | rate, RTC_BYTE);

	enable_irq(RTC_IRQ);

	return 0;
}

int32_t close_rtc() {
	return 0;
}

int32_t read_rtc() {
	while (!rtc_flag) {}
	rtc_flag = 0;
	return 0;
}
