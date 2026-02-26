#include "fields.h"
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "alarms.h"
#include "rtc.h"
#include "screens.h"
#include "shared-defs.h"
#include "sounds.h"
#include "time.h"

static Time timerBuffer = {0};
static bool useTimerBuffer = false;
static uint16_t century = 20;
static volatile uint16_t stopwatchCounter = 0;
static uint8_t snoozeDur = 5;
static uint8_t soundNum = 0;
static uint8_t hourFormat = TWELVE_HOUR;

#define NAVG 32

static uint16_t movingAverage(uint16_t nextValue)
{
	static uint16_t values[NAVG] = {0};
	static uint8_t nfilled = 0;
	static uint8_t nextIndex = 0;

	values[nextIndex] = nextValue;
	if(nfilled < NAVG) nfilled++;
	if(++nextIndex == NAVG) nextIndex = 0;

	uint24_t sum = 0;
	for(uint8_t i = 0; i < nfilled; i++)
	{
		sum += values[i];
	}
	return (uint16_t)(sum / nfilled);
}

static uint16_t getCentury(void){return century;}
static uint16_t getYear(void){return rtc_read(YEAR_REG);}
static uint16_t getDay(void){return rtc_read(DAY_REG);}
static uint16_t getMonth(void){return rtc_read(MONTH_REG);}
static uint16_t getWkdy(void){return rtc_read(WKDY_REG);}
static uint16_t getHour(void){return rtc_read(HOUR_REG);}
static uint16_t getMin(void){return rtc_read(MIN_REG);}
static uint16_t getSec(void){return rtc_read(SEC_REG);}
static uint16_t getA1on(void){return alarms_data[ALM1].enabled;}
static uint16_t getA2on(void){return alarms_data[ALM2].enabled;}
static uint16_t getA1hr(void){return alarms_data[ALM1].time.h;}
static uint16_t getA1min(void){return alarms_data[ALM1].time.m;}
static uint16_t getA2hr(void){return alarms_data[ALM2].time.h;}
static uint16_t getA2min(void){return alarms_data[ALM2].time.m;}
static uint16_t getTmrHr(void){return timerBuffer.h;}
static uint16_t getTmrMin(void){return timerBuffer.m;}
static uint16_t getStopw(void){return stopwatchCounter;}
static uint16_t getSnz(void){return snoozeDur;}
static uint16_t getSnd(void){return soundNum;}
static uint16_t getBat(void){return round(movingAverage(adc_getVdd_mV()));}
static uint16_t getSmr(void){return alarms_data[SMR_ALM].enabled;}
static uint16_t getFmt(void){return hourFormat;}
static uint16_t getAlm(void){return getNextAudibleAlarm();}

static void autoWeekday(void)
{
	uint16_t y = getYear() + getCentury() * 100;
	uint16_t m = getMonth();
	uint16_t d = getDay();
	uint16_t w = (d += m<3? y-- : y-2 , 23*m/9 + d + 4 + y/4 - y/100 + y/400) % 7;
	rtc_write(WKDY_REG, (uint8_t)w);
}

static void setCentury(uint16_t val){century = val;}
static void setYear(uint16_t val)
{
	rtc_write(YEAR_REG, (uint8_t)val);
	autoWeekday();
}
static void setDay(uint16_t val)
{
	rtc_write(DAY_REG, (uint8_t)val);
	autoWeekday();
}
static void setMonth(uint16_t val)
{
	rtc_write(MONTH_REG, (uint8_t)val);
	autoWeekday();
}
static void setWkdy(uint16_t val){rtc_write(WKDY_REG, (uint8_t)val);}
static void setHour(uint16_t val){rtc_write(HOUR_REG, (uint8_t)val);}
static void setMin(uint16_t val){rtc_write(MIN_REG, (uint8_t)val);}
static void setSec(uint16_t val){rtc_write(SEC_REG, (uint8_t)val);}
static void setA1on(uint16_t val)
{
	alarms_data[ALM1].enabled = (bool)val;
	if(!val) alarms_data[SNZ1].enabled = false;
}
static void setA2on(uint16_t val)
{
	alarms_data[ALM2].enabled = (bool)val;
	if(!val) alarms_data[SNZ2].enabled = false;
}
static void setA1hr(uint16_t val){alarms_data[ALM1].time.h = (uint8_t)val;}
static void setA1min(uint16_t val){alarms_data[ALM1].time.m = (uint8_t)val;}
static void setA2hr(uint16_t val){alarms_data[ALM2].time.h = (uint8_t)val;}
static void setA2min(uint16_t val){alarms_data[ALM2].time.m = (uint8_t)val;}
static void setTmrHr(uint16_t val){timerBuffer.h = (uint8_t)val;}
static void setTmrMin(uint16_t val){timerBuffer.m = (uint8_t)val;}
static void setStopw(uint16_t val){stopwatchCounter = val;}
static void setSnz(uint16_t val){snoozeDur = (uint8_t)val;}
static void setSnd(uint16_t val){soundNum = (uint8_t)val;}
// Battery field not editable, use nullptr
static void setSmr(uint16_t val)
{
	alarms_data[SMR_ALM].enabled = (bool)val;
	alarms_data[STD_ALM].enabled = (bool)val;
}
static void setFmt(uint16_t val){hourFormat = (uint8_t)val;}
// Next alarm field not editable, use nullptr
static void defaultInc(Field const *self)
{
	uint16_t val = self->get();
	if(++val > self->max) val = self->min;
	self->set(val);
}

static void defaultDec(Field const *self)
{
	uint16_t val = self->get();
	if(!val || --val < self->min) val = self->max;
	self->set(val);
}

static void incTime(Field const *self, Time const step)
{
	uint16_t val = self->get();
	if(++val > self->max) val = self->min;
	self->set(val);

	adjRelAlms(step, 1);
}

static void decTime(Field const *self, Time const step)
{
	uint16_t val = self->get();
	if(!val || --val < self->min) val = self->max;
	self->set(val);
	adjRelAlms(step, -1);
}

static void incHour(Field const *self){incTime(self, (Time){1, 0, 0});}
static void incMin(Field const *self){incTime(self, (Time){0, 1, 0});}
static void incSec(Field const *self){incTime(self, (Time){0, 0, 1});}
static void decHour(Field const *self){decTime(self, (Time){1, 0, 0});}
static void decMin(Field const *self){decTime(self, (Time){0, 1, 0});}
static void decSec(Field const *self){decTime(self, (Time){0, 0, 1});}

void startTimerEdit(void)
{
	useTimerBuffer = true;
	AlarmData *tmr = alarms_data + TMR_ALM;
	if(!tmr->enabled)
	{
		timerBuffer = time_new(0, 0, 0);
		return;
	}
	timerBuffer = time_diff(tmr->time, time_now());
	if(time_inSeconds(timerBuffer) < 60) timerBuffer.s = 0;
	else timerBuffer = time_round(timerBuffer);
}

void endTimerEdit(void)
{
	useTimerBuffer = false;
	AlarmData *tmr = alarms_data + TMR_ALM;
	if(time_inSeconds(timerBuffer) == 0) tmr->enabled = false;
	else
	{
		tmr->enabled = true;
		tmr->time = time_sum(timerBuffer, time_now());
	}
}

bool inTimerEdit(void)
{
	return useTimerBuffer;
}

static void incTmr(Field const *self)
{
	if(!useTimerBuffer) startTimerEdit();
	uint16_t val = self->get();
	if(++val > self->max) val = self->min;
	self->set(val);
}

static void decTmr(Field const *self)
{
	if(!useTimerBuffer) startTimerEdit();
	uint16_t val = self->get();
	if(val) val--;
	self->set(val);
}

Field const fields[NUM_FLDS] =
{
	[CENTURY_FLD] = {99, 0, 1, YEAR_SCN, getCentury, setCentury, defaultInc, defaultDec},
	[YEAR_FLD] = {99, 0, 1, YEAR_SCN, getYear, setYear, defaultInc, defaultDec},
	[DAY_FLD] = {31, 1, 1, DATE_SCN, getDay, setDay, defaultInc, defaultDec},
	[MONTH_FLD] = {12, 1, 1, DATE_SCN, getMonth, setMonth, defaultInc, defaultDec},
	[WKDY_FLD] = {6, 0, 1, WKDY_SCN, getWkdy, setWkdy, defaultInc, defaultDec},
	[HOUR_FLD] = {23, 0, 1, HOUR_SCN, getHour, setHour, incHour, decHour},
	[MIN_FLD] = {59, 0, 1, HOUR_SCN, getMin, setMin, incMin, decMin},
	[SEC_FLD] = {59, 0, 1, MIN_SCN, getSec, setSec, incSec, decSec},
	[A1_ON_FLD] = {1, 0, 1, AL_ON_SCN, getA1on, setA1on, defaultInc, defaultDec},
	[A2_ON_FLD] = {1, 0, 1, AL_ON_SCN, getA2on, setA2on, defaultInc, defaultDec},
	[A1_HR_FLD] = {23, 0, 1, AL1_SCN, getA1hr, setA1hr, defaultInc, defaultDec},
	[A1_MIN_FLD] = {59, 0, 1, AL1_SCN, getA1min, setA1min, defaultInc, defaultDec},
	[A2_HR_FLD] = {23, 0, 1, AL2_SCN, getA2hr, setA2hr, defaultInc, defaultDec},
	[A2_MIN_FLD] = {59, 0, 1, AL2_SCN, getA2min, setA2min, defaultInc, defaultDec},
	[TMR_HR_FLD] = {23, 0, 1, TMR_SCN, getTmrHr, setTmrHr, incTmr, decTmr},
	[TMR_MIN_FLD] = {59, 0, 1, TMR_SCN, getTmrMin, setTmrMin, incTmr, decTmr},
	[STOPW_FLD] = {9999, 0, 0, STOPW_SCN, getStopw, setStopw, defaultInc, nullptr},
	[SNZ_FLD] = {99, 1, 1, SNZ_SCN, getSnz, setSnz, defaultInc, defaultDec},
	[SND_FLD] = {NUM_SNDS - 1, 0, 1, SND_SCN, getSnd, setSnd, defaultInc, defaultDec},
	[SMR_FLD] = {1, 0, 1, SMR_SCN, getSmr, setSmr, defaultInc, defaultDec},
	[FMT_FLD] = {1, 0, 1, FMT_SCN, getFmt, setFmt, defaultInc, defaultDec},
	[BAT_FLD] = {9999, 0, 0, BAT_SCN, getBat, nullptr, nullptr, nullptr},
	[ALM_FLD] = {NUM_ALMS - 1, 0, 0, ALM_SCN, getAlm, nullptr, nullptr, nullptr},
};
