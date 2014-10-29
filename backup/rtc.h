#ifndef RTC_H
#define RTC_H

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

int32_t open_rtc();
int32_t write_rtc(const char* buf);
int32_t close_rtc();
int32_t read_rtc();

void rtc_handler();
#endif /* RTC_H */
