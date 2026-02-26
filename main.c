/* Project: Alarm Clock
 * File: main.c
 * Device: PIC18F27J53
 * Date: Original 2015, this revision 2025
 * Description: Alarm clock with 7-segment LED display
 */

// CONFIG1L
#pragma config WDTEN = OFF // controlled by SWDTEN
#pragma config PLLDIV = 1
#pragma config CFGPLLEN = OFF
#pragma config STVREN = OFF
#pragma config XINST = OFF // not supported by XC8

// CONFIG1H
#pragma config CPUDIV = OSC1
#pragma config CP0 = OFF

// CONFIG2L
#pragma config OSC = INTOSC
#pragma config SOSCSEL = HIGH // alt: LOW or DIG
#pragma config CLKOEC = OFF
#pragma config FCMEN = OFF
#pragma config IESO = OFF

// CONFIG2H
#pragma config WDTPS = 32768

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF
#pragma config RTCOSC = T1OSCREF
#pragma config DSBOREN = OFF
#pragma config DSWDTEN = OFF
#pragma config DSWDTPS = G2

// CONFIG3H
#pragma config IOL1WAY = ON
#pragma config ADCSEL = BIT12
#pragma config MSSP7B_EN = MSK7

// CONFIG4L
#pragma config WPFP = PAGE_0
#pragma config WPCFG = ON

// CONFIG4H
#pragma config WPDIS = ON
#pragma config WPEND = PAGE_WPFP
#pragma config LS48MHZ = SYS48X8 // bypassed in full-speed mode

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "alarms.h"
#include "display.h"
#include "fields.h"
#include "hardware.h"
#include "polled-buttons.h"
#include "screens.h"
#include "shared-defs.h"
#include "sounds.h"
#include "standby.h"
#include "tmr.h"

/* Flicker noticable at 50 Hz with peripheral vision
 * Flicker noticable at 80 Hz with eyes unfocused
 * Flicker not noticable at 100 Hz
 */

#define DISPLAY_UPDATE_RATE_Hz 100U
#define SECT_TIME_ms (uint16_t)(1000 / DISPLAY_UPDATE_RATE_Hz / NSECT)
#define AUTO_OFF_ms UINT24_C(300000)
#define BLINK_SHOW_ms 600U
#define BLINK_HIDE_ms 200U
#define BTN_POLL_PERIOD_ms 20U
#define SHIFT_TIME_ms 1100U
#define UPDATE_PERIOD_ms 100U

#define AUTO_OFF_CNT_MAX (AUTO_OFF_ms / SECT_TIME_ms)
#define BLINK_SHOW_CNT_MAX (BLINK_SHOW_ms / SECT_TIME_ms)
#define BLINK_HIDE_CNT_MAX (BLINK_HIDE_ms / SECT_TIME_ms)
#define BTN_CNT_MAX (BTN_POLL_PERIOD_ms / SECT_TIME_ms)
#define SHIFT_CNT_MAX (SHIFT_TIME_ms / SECT_TIME_ms)
#define UPDATE_CNT_MAX (UPDATE_PERIOD_ms / SECT_TIME_ms)

// File scope variables for isr
volatile bool autoOffFlag = false;
volatile bool alarmFlag = false;
volatile bool beepFlag = false;
volatile bool blinkFlag = false;
volatile bool buttonFlag = false;
volatile bool shiftFlag = false;
volatile bool stopwatchFlag = false;
volatile bool updateFlag = false;
volatile bool wakeupFlag = false;

volatile uint24_t autoOffCountdown = AUTO_OFF_CNT_MAX;
volatile uint16_t blinkCountdown = BLINK_SHOW_CNT_MAX;
volatile uint16_t buttonCountdown = BTN_CNT_MAX;
volatile uint16_t shiftCountdown = SHIFT_CNT_MAX;
volatile uint16_t updateCountdown = UPDATE_CNT_MAX;

void __interrupt() isr(void)
{
	if(PIR1bits.TMR1IF)
	{
		tmr_set(1, SECT_TIME_ms);

		if(!(--blinkCountdown))
		{
			blinkFlag = !blinkFlag;
			if(blinkFlag) blinkCountdown = BLINK_HIDE_CNT_MAX;
			else blinkCountdown = BLINK_SHOW_CNT_MAX;
			updateFlag = true;
			updateCountdown = UPDATE_CNT_MAX;
		}

		if(!(--shiftCountdown))
		{
			shiftFlag = !shiftFlag;
			shiftCountdown = SHIFT_CNT_MAX;
			updateFlag = true;
			updateCountdown = UPDATE_CNT_MAX;
		}

		if(!(--buttonCountdown))
		{
			buttonFlag = true;
			buttonCountdown = BTN_CNT_MAX;
		}

		if(!(--updateCountdown))
		{
			updateFlag = true;
			updateCountdown = UPDATE_CNT_MAX;
		}

		if(!tmr_isOn(3) && !(--autoOffCountdown))
		{
			autoOffFlag = true;
			autoOffCountdown = AUTO_OFF_CNT_MAX;
		}

		multiplex();
		PIR1bits.TMR1IF = 0;
	}
	else if(PIR2bits.TMR3IF)
	{
		tmr_set(3, 100);
		stopwatchFlag = true;
		PIR2bits.TMR3IF = 0;
	}
	else if(PIR5bits.TMR5IF)
	{
		beepFlag = true;
		PIR5bits.TMR5IF = 0;
	}
	else if(PIR3bits.RTCCIF)
	{
		alarmFlag = true;
		PIR3bits.RTCCIF = 0;
	}
	else if(INTCONbits.INT0IF)
	{
		wakeupFlag = true;
		INTCONbits.INT0IF = 0;
	}
}

void main(void)
{
	setup();

	State state = {NORMAL, HOUR_SCN, HOUR_FLD, false, true};

	#define NDLY 3
	static uint8_t const buttonDelays[NDLY] =
	{
		40 / BTN_POLL_PERIOD_ms,
		160 / BTN_POLL_PERIOD_ms,
		220 / BTN_POLL_PERIOD_ms
	};

	uint8_t debounceCountdown = 0;
	uint8_t delayIndex = NDLY - 1;
	uint8_t currBtn = NO_BTN;

	// Initialize global variables (see shared-defs.h)
	blinkFlag = false;
	shiftFlag = false;

	while(1)
	{
		if(buttonFlag)
		{
			if(debounceCountdown) debounceCountdown--;
			else
			{
				uint8_t prevBtn = currBtn;
				currBtn = pollButtons();
				bool buttonHeld = (currBtn == prevBtn);
				if(currBtn != NO_BTN && (!buttonHeld || buttons[currBtn].repeating))
				{
					if(buttonHeld)
					{
						if(delayIndex) delayIndex--;
						blinkFlag = false;
						blinkCountdown = BLINK_SHOW_CNT_MAX;
					}
					else
					{
						delayIndex = NDLY - 1;
						if(state.mode == EDIT && (currBtn == DOWN_BTN || currBtn == UP_BTN))
						{
							blinkFlag = false;
							blinkCountdown = BLINK_SHOW_CNT_MAX;
						}
						else
						{
							blinkFlag = true;
							blinkCountdown = BLINK_HIDE_CNT_MAX;
						}
					}

					buttons[currBtn].action(&state);
					autoOffCountdown = AUTO_OFF_CNT_MAX;

					// Don't stop the beep if button press started sound test
					if(state.mode == SOUND_TEST) state.mode = NORMAL;
					else beepFlag = false;

					shiftFlag = true;
					shiftCountdown = SHIFT_CNT_MAX;
					updateFlag = true;
					updateCountdown = UPDATE_CNT_MAX;
					debounceCountdown = buttonDelays[delayIndex];
				}
			}
			buttonFlag = false;
		}

		if(updateFlag && state.displayEnabled)
		{
			clearDisplayBuffer();
			Screen const *s = screens + state.screen;
			if(state.mode == INFO) s->drawInfo(s);
			else s->draw(&state);
			loadDisplayBuffer();
			updateFlag = false;
		}

		if(alarmFlag)
		{
			state.alarmSounding = raiseAlarms();
			alarmFlag = false;
		}

		if(beepFlag)
		{
			state.alarmSounding = sounds[fields[SND_FLD].get()].play();
			beepFlag = false;
		}

		if(autoOffFlag)
		{
			state.displayEnabled = false;
			autoOffFlag = false;
		}

		if(wakeupFlag)
		{
			state.displayEnabled = true;
			updateFlag = true;
			updateCountdown = UPDATE_CNT_MAX;
			wakeupFlag = false;
		}

		if(stopwatchFlag)
		{
			Field const *sw = fields + STOPW_FLD;
			if(sw->get() < sw->max)
			{
				sw->inc(sw);
				updateFlag = true;
				updateCountdown = UPDATE_CNT_MAX;
			}
			stopwatchFlag = false;
		}

		if(!state.alarmSounding)
		{
			if(state.displayEnabled) SLEEP();
			else
			{
				clearDisplayBuffer();
				loadDisplayBuffer();
				__delay_ms(1000 / DISPLAY_UPDATE_RATE_Hz); // Wait for buffer to be loaded
				currBtn = NO_BTN;
				buttonCountdown = BTN_CNT_MAX;
				buttonFlag = false;
				delayIndex = NDLY - 1;
				debounceCountdown = buttonDelays[delayIndex];
				state.mode = NORMAL;
				if(state.screen != TMR_SCN || !alarms_data[TMR_ALM].enabled)
				{
					state.screen = HOUR_SCN;
					state.field = HOUR_FLD;
				}
				standby();
			}
		}
	}
}
