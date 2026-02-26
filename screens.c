#include "screens.h"
#include <stdbool.h>
#include <stdint.h>
#include "alarms.h"
#include "display.h"
#include "fields.h"
#include "shared-defs.h"
#include "sounds.h"
#include "time.h"

static void defaultInfo(Screen const *self)
{
	displayString(self->name);
}

static void hourInfo(Screen const *self)
{
	if(fields[FMT_FLD].get() == TWELVE_HOUR && fields[HOUR_FLD].get() >= 12)
	{
		upperDotOn();
	}
	displayString(self->name);
}

static void alxInfo(Screen const *self)
{
	if(fields[FMT_FLD].get() == TWENTYFOUR_HOUR || shiftFlag)
	{
		displayString(self->name);
	}
	else
	{
		uint8_t h = (uint8_t)fields[HOUR_FLD].get();
		uint8_t m = (uint8_t)fields[MIN_FLD].get();
		if(!h && !m) displayString("Mid.");
		else if(h == 12 && !m) displayString("noon");
		else if(h >= 12) displayString("PM");
		else displayString("AM");
	}
}

static void nextAlmInfo(Screen const *self)
{
	uint8_t alm = getNextAudibleAlarm();
	if(alm < NUM_ALMS) displayString(alarms[alm].name);
	else displayString("NONe");
}

static void drawYear(State *state)
{
	uint16_t c = fields[CENTURY_FLD].get();
	uint16_t y = fields[YEAR_FLD].get();

	if(state->mode == EDIT && blinkFlag)
	{
		if(state->field == CENTURY_FLD) print("  %02u", y);
		else print("%02u  ", c);
	}
	else print("%02u%02u", c, y);
}

static void drawDate(State *state)
{
	uint16_t mth = fields[MONTH_FLD].get();
	uint16_t day = fields	[DAY_FLD].get();
	if(state->mode == EDIT && blinkFlag)
	{
		if(state->field == DAY_FLD) print("  .%02u", mth);
		else print("%02u.  ", day);
	}
	else print("%02u.%02u", day, mth);
}

static void drawWkdy(State *state)
{
	static char const *days[7] = {"Sun.", "Mon.", "tUeS.", "Wed.", "thur.", "Fri.", "Sat."};
	if(!(state->mode == EDIT && blinkFlag))
	{
		displayString(days[fields[WKDY_FLD].get()]);
	}
}

static void hr_min_draw(State *state, uint8_t hIdx, uint8_t mIdx)
{
	uint16_t h = fields[hIdx].get();
	uint16_t m = fields[mIdx].get();
	if(fields[FMT_FLD].get() == TWELVE_HOUR)
	{
		if(state->mode == EDIT && h >= 12) upperDotOn();
		if(!h) h = 12;
		else if(h > 12) h -= 12;
	}

	if(state->mode == EDIT && blinkFlag)
	{
		if(state->field == hIdx) print("  :%02u", m);
		else print("%2u:  ", h);
	}
	else
	{
		print("%2u:%02u", h, m);
	}
}

static void drawHour(State *state)
{
	hr_min_draw(state, HOUR_FLD, MIN_FLD);
}

static void drawMin(State *state)
{
	uint16_t m = fields[MIN_FLD].get();
	uint16_t s = fields[SEC_FLD].get();

	if(state->mode == EDIT && blinkFlag)
	{
		if(state->field == MIN_FLD) print("  :%02u", s);
		else print("%2u:  ", m);
	}
	else
	{
		print("%2u:%02u", m, s);
	}
}

static void drawAlmOn(State *state)
{
	displayDigit(1);
	if(state->mode == EDIT && state->field == A1_ON_FLD && blinkFlag) displayChar(' ');
	else displayChar(alarms_data[ALM1].enabled? '*' : 'o');

	displayDigit(2);
	if(state->mode == EDIT && state->field == A2_ON_FLD && blinkFlag) displayChar(' ');
	else displayChar(alarms_data[ALM2].enabled? '*' : 'o');

	if(shiftFlag)
	{
		if(alarms_data[SNZ1].enabled) dpOn(DIG1);
		if(alarms_data[SNZ2].enabled) dpOn(DIG3);
	}
}

static void drawAlm1(State *state)
{
	hr_min_draw(state, A1_HR_FLD, A1_MIN_FLD);
	if(alarms_data[ALM1].enabled) dpOn(DIG4);
}

static void drawAlm2(State *state)
{
	hr_min_draw(state, A2_HR_FLD, A2_MIN_FLD);
	if(alarms_data[ALM2].enabled) dpOn(DIG4);
}

static void drawTmr(State *state)
{
	AlarmData *tmr = alarms_data + TMR_ALM;
	Time t;
	if(inTimerEdit()) t = time_new((uint8_t)fields[TMR_HR_FLD].get(), (uint8_t)fields[TMR_MIN_FLD].get(), 0);
	else if(!tmr->enabled) t = time_new(0, 0, 0);
	else t = time_diff(tmr->time, time_now());

	uint16_t n1, n2;
	if(!inTimerEdit() && time_inSeconds(t) < 60)
	{
		n1 = t.m;
		n2 = t.s;
	}
	else
	{
		n1 = t.h;
		n2 = t.m + (bool)t.s;
	}

	if(state->mode == EDIT && blinkFlag)
	{
		if(state->field == TMR_HR_FLD) print("  :%02u", n2);
		else print("%02u:  ", n1);
	}
	else print("%02u:%02u", n1, n2);
}

static void drawStopw(State *state)
{
	displayUint(fields[STOPW_FLD].get(), 4, '0');
	dpOn(DIG3);
}

static void drawSnz(State *state)
{
	if(!(state->mode == EDIT && blinkFlag))
	{
		displayUint(fields[SNZ_FLD].get(), 2, ' ');
	}
	else
	{
		displayString("  ");
	}
	displayChar('M');
}

static void drawSnd(State *state)
{
	if(!(state->mode == EDIT && blinkFlag))
	{
		
		displayString(sounds[fields[SND_FLD].get()].name);
	}
}

static void drawBat(State *state)
{
	displayUint(fields[BAT_FLD].get() / 10, 3, '0');
	dpOn(DIG1);
	displayChar('V');
}

static void drawSmr(State *state)
{
	if(!(state->mode == EDIT && blinkFlag))
	{
		if(fields[SMR_FLD].get()) displayString("AUTO.");
		else displayString("MAN.");
	}
}

static void drawFmt(State *state)
{
	if(!(state->mode == EDIT && blinkFlag))
	{
		if(fields[FMT_FLD].get() == TWELVE_HOUR) displayString("12hr");
		else displayString("24hr");
	}	
}

static void drawNextAlm(State *state)
{
	uint8_t alm = getNextAudibleAlarm();
	if(alm < NUM_ALMS)
	{
		Time t = alarms_data[alm].time;
		print("%2u:%02u", (unsigned)t.h, (unsigned)t.m);
	}
	else print("--:--");
}

Screen const screens[NUM_SCNS] =
{
	[YEAR_SCN] = {"yeaR", YEAR_FLD, defaultInfo, drawYear},
	[DATE_SCN] = {"date", DAY_FLD, defaultInfo, drawDate},
	[WKDY_SCN] = {"day", WKDY_FLD, defaultInfo, drawWkdy},
	[HOUR_SCN] = {"Hour", HOUR_FLD, hourInfo, drawHour},
	[MIN_SCN] = {"Min.", MIN_FLD, defaultInfo, drawMin},
	[AL_ON_SCN] = {"AL.EN.", A1_ON_FLD, defaultInfo, drawAlmOn},
	[AL1_SCN] = {"AL1", A1_HR_FLD, alxInfo, drawAlm1},
	[AL2_SCN] = {"AL2", A2_HR_FLD, alxInfo, drawAlm2},
	[TMR_SCN] = {"tMR", TMR_MIN_FLD, defaultInfo, drawTmr},
	[STOPW_SCN] = {"St.W.", STOPW_FLD, defaultInfo, drawStopw},
	[SNZ_SCN] = {"SNZ", SNZ_FLD, defaultInfo, drawSnz},
	[SND_SCN] = {"Snd", SND_FLD, defaultInfo, drawSnd},
	[SMR_SCN] = {"d.s.t.", SMR_FLD, defaultInfo, drawSmr},
	[FMT_SCN] = {"dISP.", FMT_FLD, defaultInfo, drawFmt},
	[BAT_SCN] = {"batt.", BAT_FLD, defaultInfo, drawBat},
	[ALM_SCN] = {"ALM", ALM_FLD, nextAlmInfo, drawNextAlm},
};
