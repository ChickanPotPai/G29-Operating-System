// code is commented

#include "rtc.h"

#define UPPER_MASK	0xF0
#define SET 		1
#define CLEAR 		0

// flag to track interrupts
volatile static uint8_t rtc_flag = CLEAR;
static uint8_t rtc_open_flag = 0;

/*	rtc_init
 *	Initialize the rtc so the rtc can begin making interrupts
 *	INPUTS: None
 *	RETURN: None
 *	EFFECT: add rtc_handler into the IDT and enable rtc IRQs
 *			set initial freqeuency to 2Hz (~2 interrupts/second)
 */
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

/*	rtc_handler
 *	the rtc handler/driver
 *	INPUTS: None
 *	RETURN: None
 *	EFFECT: call test_interrupts -> change every chracter on screen
 */
void rtc_handler() {
	disable_irq(RTC_IRQ);

	// set flag
	rtc_flag = SET;

	// read register C to re-enable rtc interrupts
	outb(REG_C_BLK, RTC_INDEX);
	(void) inb(RTC_BYTE);
	
	// signal that interrupt has been handled
	send_eoi(RTC_IRQ);
    enable_irq(RTC_IRQ);
}

/*	rtc_open
 * opens the rtc to be used by a program
 *	INPUTS: filename, curr_pcb
 *	RETURN: -1 on failure, 0 on sucess
 *	EFFECT: rtc flag is set to open
 */
int32_t rtc_open(const uint8_t* filename, pcb_t* curr_pcb) {
	if (rtc_open_flag == 1) {return -1;}
	// rtc_open_flag = 1;
	//enable_irq(RTC_IRQ);
	return 0;
}

/*	rtc_write
 *	Function will write a given frequency (power of 2 between 2 and 1024 in hz) to the RTC
 *	INPUTS: buf (contains frequency)
 *	RETURN: break on bad frequency value, 0 on success
 *	EFFECT: frequency written to rtc
 */
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

/*	rtc_close
 *	INPUTS: none
 *	RETURN: 0 in all cases
 *	EFFECT: clears rtc flags to be closed
 */
int32_t rtc_close(int32_t fd, pcb_t* curr_pcb) {
	rtc_open_flag = 0;

	curr_pcb->file_array[fd].fops_ptr = NULL;
	curr_pcb->file_array[fd].inode_ptr = NULL;
	curr_pcb->file_array[fd].file_position = 0;
	curr_pcb->file_array[fd].flags = 0;

	return 0;
}

/*	rtc_read
 * 	Function is called when rtc is open
 *	Function will wait for an interrupt to occur before returning
 *	INPUTS: none
 *	RETURN: 0 once interrupt occurs
 *	EFFECT: will wait for the interrupt to happen before returning
 */
int32_t rtc_read(uint32_t fd, const uint8_t* buf, uint32_t nbytes) {
	volatile uint8_t temp_flag = CLEAR;

	sti();

	while (!temp_flag) {temp_flag = rtc_flag;}
	rtc_flag = CLEAR;

	return 0;
}
