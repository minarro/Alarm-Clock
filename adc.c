#include "adc.h"
#include <xc.h>
#include <stdint.h>

void adc_setup(void)
{
	ANCON1bits.VBGEN = 1;
	ADCON0 = 0b00111101;
	ADCON1 = 0b10111011;
}

uint16_t adc_getVdd_mV(void)
{
	ADCON0bits.GO = 1;
	while(ADCON0bits.GO) continue;
	int16_t result = ADRESH << 8 | ADRESL;
	return (uint16_t)(BANDGAP_mV * 4096L / result + BATT_OFFSET_mV);
}
