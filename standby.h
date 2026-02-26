#ifndef STANDBY_H_
#define STANDBY_H_

#include "standby.h"
#include <xc.h>
#include "polled-buttons.h"
#include "shared-defs.h"

static inline void standby(void)
{
	LATA = 0;
	LATCbits.LC2 = 0;
	TRISBbits.TRISB1 = 0;
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;
	TRISCbits.TRISC6 = 0;
	TRISCbits.TRISC7 = 0;

	bool tmr1on = T1CONbits.TMR1ON;
	bool tmr3on = T3CONbits.TMR3ON;
	bool tmr5on = T5CONbits.TMR5ON;
	T1CONbits.TMR1ON = 0;
	T3CONbits.TMR3ON = 0;
	T5CONbits.TMR5ON = 0;

	ADCON0bits.ADON = 0;
	ANCON1bits.VBGEN = 0;

	__delay_ms(50);
	while(pollButtons() != NO_BTN) continue;
	__delay_ms(200);

	INTCONbits.INT0IF = 0;
	INTCONbits.INT0IE = 1;
	SLEEP();
	NOP();
	INTCONbits.INT0IE = 0;

	__delay_ms(50);

	T1CONbits.TMR1ON = tmr1on;
	T3CONbits.TMR3ON = tmr3on;
	T5CONbits.TMR5ON = tmr5on;
	ADCON0bits.ADON = 1;
	ANCON1bits.VBGEN = 1;
}

#endif
