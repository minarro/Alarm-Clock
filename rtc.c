#include "rtc.h"
#include <xc.h>
#include <stdint.h>
#include "alarms.h"
#include "bcd.h"
#include "shared-defs.h"
#include "time.h"

/* To unlock the RTCC, RTCWREN must be set directly after
 * writing 0b01010101 and 0b10101010 to EECON2
 */
static void rtc_unlock(void)
{
	uint8_t gie = INTCONbits.GIE;
	INTCONbits.GIE = 0;
	asm
	(
		"MOVLB	0x0F\n"
		"movlw	0x55\n"
		"movwf	EECON2\n"
		"movlw	0xAA\n"
		"movwf	EECON2\n"
		"BSF	RTCCFG,5,1\n"
	);
	INTCONbits.GIE = gie;
}

void rtc_setup(void)
{
	rtc_unlock();

	// Initialize the RTCC and alarm registers
	RTCCFGbits.RTCPTR1 = 1;
	RTCCFGbits.RTCPTR0 = 1; // Year
	RTCVALL = 0; // 2000
	RTCVALH = 0; // Decrements "pointer" automatically
	RTCVALL = 1; // 1st
	RTCVALH = 1; // January
	RTCVALL = 0; // Hour
	RTCVALH = 0; // Sunday
	RTCVALL = 0; // Second
	RTCVALH = 0; // Minute

	ALRMCFGbits.ALRMPTR = 0b01; // Weekday / hour
	ALRMVALL = 0;
	ALRMCFGbits.ALRMPTR = 0b00; // Minute / second
	ALRMVALL = 0;
	ALRMVALH = 0;

	ALRMCFGbits.AMASK = DAILY;
	ALRMCFGbits.CHIME = 1; // Alarm won't disable itself
	RTCCAL = 0b00000100; // Crystal error compensation
	RTCCFGbits.RTCEN = 1; // Module enabled
}

uint8_t rtc_read(enum RtcRegSel reg)
{
	uint8_t gie = INTCONbits.GIE;
	INTCONbits.GIE = 0;
	RTCCFGbits.RTCPTR1 = (bool)(reg & 0b100);
	RTCCFGbits.RTCPTR0 = (bool)(reg & 0b010);
	while(RTCCFGbits.RTCSYNC) continue;
	uint8_t val = (reg & 0b001)? RTCVALH : RTCVALL;
	INTCONbits.GIE = gie;
	return bcd2b(val);
}

void rtc_write(enum RtcRegSel reg, uint8_t val)
{
	uint8_t gie = INTCONbits.GIE;
	uint8_t alrmen = ALRMCFGbits.ALRMEN;
	INTCONbits.GIE = 0;
	ALRMCFGbits.ALRMEN = 0;
	RTCCFGbits.RTCPTR1 = (bool)(reg & 0b100);
	RTCCFGbits.RTCPTR0 = (bool)(reg & 0b010);
	val = b2bcd(val);
	while(RTCCFGbits.RTCSYNC) continue;
	if(reg & 0b001) RTCVALH = val;
	else RTCVALL = val;
	ALRMCFGbits.ALRMEN = alrmen;
	INTCONbits.GIE = gie;
}

void rtc_writeAlarm(AlarmData *alm)
{
	Time t = alm->time;
	ALRMCFGbits.ALRMEN = 0;
ALRMCFGbits.ALRMPTR = 0b01;
	ALRMVALL = b2bcd(t.h);
	ALRMCFGbits.ALRMPTR = 0b00;
	ALRMVALL = b2bcd(t.s);
	ALRMVALH = b2bcd(t.m);
	ALRMCFGbits.ALRMEN = alm->enabled;
}
