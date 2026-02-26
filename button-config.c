#include "button-config.h"
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "alarms.h"
#include "polled-buttons.h"
#include "fields.h"
#include "screens.h"
#include "shared-defs.h"
#include "tmr.h"

static bool leftPressed(void)
{
	return !PORTBbits.RB4;
}

static bool rightPressed(void)
{
	return !PORTBbits.RB5;
}

static bool downPressed(void)
{
	return !PORTCbits.RC4;
}

static bool upPressed(void)
{
	return !PORTCbits.RC5;
}

static bool snoozePressed(void)
{
	return !PORTBbits.RB6;
}

static void cancelAlarms(State *state)
{
	snd_stop();
	state->alarmSounding = false;
	for(uint8_t i = 0; i < NUM_ALMS; i++)
	{
		alarms_data[i].raised = false;
	}
}

static void startSoundTest(State *state)
{
	sounds[fields[SND_FLD].get()].play();
	state->alarmSounding = true;
	state->mode = SOUND_TEST; // Tells main not to clear beepFlag
}

static void leftAction(State *state)
{
	if(state->alarmSounding)
	{
		cancelAlarms(state);
	}
	else if(state->mode == EDIT)
	{
		if(state->field == TMR_HR_FLD && inTimerEdit()) endTimerEdit();
		if(state->field > 0) state->field -= 1;
		state->screen = fields[state->field].screen;
	}
	else
	{
		if(state->screen > 0) state->screen -= 1;
		state->field = screens[state->screen].field;
	}
}

static void rightAction(State *state)
{
	if(state->alarmSounding)
	{
		cancelAlarms(state);
	}
	else if(state->mode == EDIT)
	{
		if(state->field == TMR_MIN_FLD && inTimerEdit()) endTimerEdit();
		if(state->field < NUM_FLDS - 1) state->field += 1;
		state->screen = fields[state->field].screen;
	}
	else
	{
		if(state->screen < NUM_SCNS - 1) state->screen += 1;
		state->field = screens[state->screen].field;
	}
}

static void downAction(State *state)
{
	if(state->alarmSounding)
	{
		cancelAlarms(state);
	}
	else if(state->mode == EDIT)
	{
		Field const *fld = fields + state->field;
		if(fld->editable) fld->dec(fld);
	}
	else if(state->mode == NORMAL)
	{
		state->mode = EDIT;
		ALRMCFGbits.ALRMEN = 0;
	}
}

static void upAction(State *state)
{
	if(state->alarmSounding)
	{
		cancelAlarms(state);
	}
	else if(state->mode == EDIT)
	{
		Field const *fld = fields + state->field;
		if(fld->editable) fld->inc(fld);
	}
	else if(state->mode == NORMAL)
	{
		if(state->screen == SND_SCN) startSoundTest(state);
		else state->mode = INFO;
	}
	else
	{
		state->mode = NORMAL;
	}
}

static void snoozeAction(State *state)
{
	if(state->alarmSounding)
	{
		snd_stop();
		state->alarmSounding = false;
		for(uint8_t i = 0; i < NUM_ALMS; i++)
		{
			if(alarms_data[i].raised && alarms[i].snz != NO_ALM)
			{
				snooze(i, fields[SNZ_FLD].get());
			}
			alarms_data[i].raised = false;
		}
		writeNextAlarm();
	}
	else if(state->mode == EDIT)
	{
		if(inTimerEdit()) endTimerEdit();
		state->mode = NORMAL;
		writeNextAlarm();
	}
	else if(state->screen == STOPW_SCN)
	{
		if(tmr_isOn(3)) tmr_off(3);
		else if(fields[STOPW_FLD].get() > 0) fields[STOPW_FLD].set(0);
		else
		{
			tmr_set(3, 100);
			tmr_on(3);
		}
	}
	else
	{
		state->displayEnabled = false;
	}
}

Button const buttons[NUM_BTNS] =
{
	[LEFT_BTN] = {leftAction, leftPressed, LEFT_BTN, 1},
	[RIGHT_BTN] = {rightAction, rightPressed, RIGHT_BTN, 1},
	[DOWN_BTN] = {downAction, downPressed, DOWN_BTN, 1},
	[UP_BTN] = {upAction, upPressed, UP_BTN, 1},
	[SNOOZE_BTN] = {snoozeAction, snoozePressed, SNOOZE_BTN, 0},
};