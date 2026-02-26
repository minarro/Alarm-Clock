#include "alarms.h"
#include <stdbool.h>
#include <stdint.h>
#include "fields.h"
#include "rtc.h"
#include "shared-defs.h"
#include "sounds.h"
#include "time.h"

void adjRelAlms(Time adjustment, int8_t direction)
{
	Time (*adjFn)(Time, Time) = direction < 0? time_diff : time_sum;
	for(uint8_t i = 0; i < NUM_ALMS; i++)
	{
		if(alarms[i].relative)
		{
			AlarmData *d = alarms_data + i;
			d->time = adjFn(d->time, adjustment);
		}
	}
}

static bool skipNext = false;

static void bringHourForward(void) // 1 to 2 am on last sunday of March
{
	skipNext = false; // Reset filescope variable, see below
	if(rtc_read(MONTH_REG) == 3 && rtc_read(DAY_REG) >= 25 && rtc_read(WKDY_REG) == 0)
	{
		rtc_write(HOUR_REG, (rtc_read(HOUR_REG) + 1) % 24);
		adjRelAlms((Time){1, 0, 0}, 1);
	}
}

static void bringHourBack(void) // 2 to 1 am on last sunday in October
{
	if(skipNext) // Avoid endless loop 2->1->2->1...
	{
		skipNext = false;
		return;
	}

	if(rtc_read(MONTH_REG) == 10 && rtc_read(DAY_REG) >= 25 && rtc_read(WKDY_REG) == 0)
	{
		rtc_write(HOUR_REG, (rtc_read(HOUR_REG) - 1) % 24);
		skipNext = true;
		adjRelAlms((Time){1, 0, 0}, -1);
	}
}
static bool isLeapYear(uint16_t y)
{
	return !(y % 4) && (y % 100 || !(y % 400));
}

static void scheduleLeapCheck(woid)
{
	if(rtc_read(MONTH_REG) == 2 && rtc_read(DAY_REG) == 28)
	{
		alarms_data[LEAP2_ALM].enabled = true;
	}
}

static void leapCheck(void)
{
	if(isLeapYear(100 * fields[CENTURY_FLD].get() + rtc_read(YEAR_REG)))
	{
		if(rtc_read(MONTH_REG) == 3 && rtc_read(DAY_REG) == 1)
		{
			rtc_write(MONTH_REG, 2);
			rtc_write(DAY_REG, 29);
		}
	}
	else
	{
		if(rtc_read(MONTH_REG) == 2 && rtc_read(DAY_REG) == 29)
		{
			rtc_write(MONTH_REG, 3);
			rtc_write(DAY_REG, 1);
		}
	}
}

uint8_t findNextAlarm(bool audibleOnly)
{
	Time now = time_now();
	uint24_t time = 0;
	uint24_t leastTime = UINT24_MAX;
	uint8_t alm = NO_ALM;

	for(uint8_t i = 0; i < NUM_ALMS; i++)
	{
		Alarm const *a = alarms + i;
		AlarmData *d = alarms_data + i;
		if(d->enabled && (!audibleOnly || a->audible))
		{
			time = time_diff_seconds(d->time, now);
			if(time > 0 && time < leastTime)
			{
				alm = i;
				leastTime = time;
			}
		}
	}
	return alm;
}

uint8_t writeNextAlarm(void)
{
	uint8_t alm = findNextAlarm(false);
	if(alm < NUM_ALMS) rtc_writeAlarm(alarms_data + alm);
	return alm;
}

bool raiseAlarms(void)
{
	bool startSound = false;
	Time now = time_now();
	for(uint8_t i = 0; i < NUM_ALMS; i++)
	{
		AlarmData *d = alarms_data + i;
		if(d->enabled && time_isEqual(d->time, now))
		{
			Alarm const *a = alarms + i;
			if(a->action) a->action();
			if(a->audible)
			{
				startSound = true;
				d->raised = true;
			}
			if(!a->repeating) d->enabled = false;
		}
	}

	writeNextAlarm();
	if(startSound) sounds[fields[SND_FLD].get()].play();
	return startSound;
}

void snooze(uint8_t alm, uint16_t dur)
{
	uint8_t snz = alarms[alm].snz;
	if(snz == NO_ALM) return;

	AlarmData *d = alarms_data + snz;
	d->time = time_sum(time_new((uint8_t)(dur / 60), (uint8_t)(dur % 60), 0), time_now());
	d->enabled = true;
}

Alarm const alarms[NUM_ALMS] =
{
	[ALM1] = {"AL1", ALM1, SNZ1, 1, 1, 0, nullptr},
	[ALM2] = {"AL2", ALM2, SNZ2, 1, 1, 0, nullptr},
	[SNZ1] = {"SNZ1", SNZ1, SNZ1, 1, 0, 1, nullptr},
	[SNZ2] = {"SNZ2", SNZ2, SNZ2, 1, 0, 1, nullptr},
	[TMR_ALM] = {"tMR", TMR_ALM, NO_ALM, 1, 0, 1, nullptr},
	[SMR_ALM] = {"SMR", SMR_ALM, NO_ALM, 0, 1, 0, bringHourForward},
	[STD_ALM] = {"Stnd", STD_ALM, NO_ALM, 0, 1, 0, bringHourBack},
	[LEAP1_ALM] = {"LP1", LEAP1_ALM, NO_ALM, 0, 1, 0, scheduleLeapCheck},
	[LEAP2_ALM] = {"LP2", LEAP2_ALM, NO_ALM, 0, 1, 0, leapCheck},
};

AlarmData alarms_data[NUM_ALMS] =
{
	[ALM1] = {0},
	[ALM2] = {0},
	[SNZ1] = {0},
	[SNZ2] = {0},
	[TMR_ALM] = {0},
	[SMR_ALM] = {{1, 0, 0}, 1, 0},
	[STD_ALM] = {{2, 0, 0}, 1, 0},
	[LEAP1_ALM] = {{23, 59, 59}, 1, 0},
	[LEAP2_ALM] = {0},
};
