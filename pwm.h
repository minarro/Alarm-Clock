#ifndef PWM_H_
#define PWM_H_

#include <xc.h>
#include <stdint.h>
#include "shared-defs.h"
#include "tmr.h"

#define pwm_on() do{tmr_on(2); CCP7CONbits.CCP7M = 0b1100;} while(0)
#define pwm_off() do{CCP7CONbits.CCP7M = 0; tmr_off(2);} while(0)
#define pwm_toggle()\
do{\
	if(CCP7CONbits.CCP7M) pwm_off();\
	else pwm_on();\
} while(0)

#define pwm_set(FREQ, DUTY)\
do{\
	static const float PWM_VAL = _XTAL_FREQ / ((FREQ) * 4.0f * (float)TMR2_PS_VAL);\
	static const uint16_t PWM_CCP = (uint16_t)((DUTY) * 4.0f * PWM_VAL + 0.5f);\
	PR2 = (uint8_t)(PWM_VAL + 0.5f) - 1;\
	CCPR7L = (PWM_CCP >> 2) & 0xFF;\
	CCP7CONbits.DC7B = PWM_CCP & 0x3;\
} while(0)

static inline void pwm_setup(void)
{
	LATBbits.LB7 = 0;
	TRISBbits.TRISB7 = 0;
	T2CONbits.T2CKPS = TMR2_PS_SEL;
}

#endif