#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>
#include "alarms.h"

// Values for ALRMCFGbits.AMASK
enum AlarmMask
{
	EVERY_HALF_SEC, EVERY_SEC, EVERY_TEN_SEC,
	EVERY_MIN, EVERY_TEN_MIN, HOURLY, DAILY,
	WEEKLY, MONTHLY, YEARLY
};

enum RtcRegSel
{
	SEC_REG, MIN_REG, HOUR_REG, WKDY_REG,
	DAY_REG, MONTH_REG, YEAR_REG
};

void rtc_setup(void);
uint8_t rtc_read(enum RtcRegSel);
void rtc_write(enum RtcRegSel, uint8_t val);
void rtc_writeAlarm(AlarmData *);

#endif