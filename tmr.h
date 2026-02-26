#ifndef TMR_H_
#define TMR_H_

#include <stdint.h>
#include <xc.h>
#include "shared-defs.h"

#define tmr_isOn(N) (T ## N ## CONbits.TMR ## N ## ON)
#define tmr_on(N) do{T ## N ## CONbits.TMR ## N ## ON = 1;} while(0)
#define tmr_off(N) do{T ## N ## CONbits.TMR ## N ## ON = 0;} while(0)
#define tmr_set(N, DUR_ms)\
do{\
	static const uint16_t TMR_VAL = (uint16_t)(65536.0f - (DUR_ms) * 32.768f + 0.5f);\
	TMR ## N ## H = TMR_VAL >> 8;\
	TMR ## N ## L = TMR_VAL & 0xFF;\
} while(0)

static inline void tmr_setup(void)
{
	T1CON = 0b10001111; // T1OSC/T1CKI, 1:1, T1OSCEN, No sync, 16-bit R/W, On
	T3CON = 0b10001110; // Same but off
	T5CON = 0b10001110;
	tmr_set(1, 2);
}

#endif