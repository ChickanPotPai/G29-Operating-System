#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "i8259.h"	
#include "syscalls.h"
#include "tasks.h"

#define RTC_IRQ		8
#define BASE_RATE	15
#define RATE_MAX	15
#define RTC_INDEX 	0x70
#define RTC_BYTE	0x71
#define REG_A		0x8A
#define REG_B		0x8B
#define REG_C		0x8C
#define REG_C_BLK	0x0C
#define RTC_MASK	0x40

void rtc_init();

int32_t rtc_open(const uint8_t* filename, pcb_t* curr_pcb);
int32_t rtc_write(uint32_t fd, const uint8_t* buf, uint32_t nbytes);
int32_t rtc_close(int32_t fd, pcb_t* curr_pcb);
int32_t rtc_read(uint32_t fd, const uint8_t* buf, uint32_t nbytes);

void rtc_handler();
#endif /* RTC_H */
