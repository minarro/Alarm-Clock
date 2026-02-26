#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <xc.h>

#include "adc.h"
#include "pwm.h"
#include "rtc.h"
#include "shared-defs.h"
#include "tmr.h"

static inline void setup(void)
{
	OSCCONbits.IRCF = IRCF_SEL;
	OSCCONbits.SCS = 0b11; // Postscaled internal clock
	OSCCON2bits.PRISD = 0; // Primary osc drive curcuit off (zero power)

	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	INTCON2bits.INTEDG0 = 0; // Falling edge for wakeup
	PIR3bits.RTCCIF = 0;
	PIR1bits.TMR1IF = 0;
	PIR2bits.TMR3IF = 0;
	PIR5bits.TMR5IF = 0;
	PIE3bits.RTCCIE = 1;
	PIE1bits.TMR1IE = 1;
	PIE2bits.TMR3IE = 1;
	PIE5bits.TMR5IE = 1;

	PMDIS3 = 0b11101111;
	PMDIS2 = 0b11110111;
	PMDIS1 = 0b11010001;
	PMDIS0 = 0b11111110;

// Use RC4 and RC5 as digital inputs (can't be outputs)
	UCONbits.USBEN = 0; // Disable USB module
	UCFGbits.UTRDIS = 1; // Disable on-chip USB transeiver

	WDTCONbits.REGSLP = 1; // On-chip regulator enters low-power state in sleep
	ANCON1 = 0b10011111; // Bandgap on, AN12 - AN8 are digital
	ANCON0 = 0b11111111; // AN7 - AN0 are digital
	LATA = 0;
	LATB = 0;
	LATC = 0;
	TRISA = 0; // Output to 7-segment display (no RA4 pin)
	TRISB = 0b01111111; // Speaker, 3 buttons, 3 digits, ext. interrupt
	TRISC = 0b11110000; // 2 digits, 2 buttons, no RC3 pin, DP segment, 2 XTAL

	adc_setup();
	pwm_setup();
	tmr_setup();
	rtc_setup();
}

#endif