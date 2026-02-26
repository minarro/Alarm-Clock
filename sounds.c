#include "sounds.h"
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "pwm.h"
#include "tmr.h"

static uint16_t i = 0; // Records position in sequence between calls

void snd_stop(void)
{
	tmr_off(5);
	pwm_off();
	i = 0;
}

bool snd_plain(void)
{
	static uint8_t n = 8; // Beep pattern repeats after n timer periods
	static uint8_t m = 4; // Beeps occur during the 1st m of the n periods

	if(i == 0) // Start of 2-beep section
	{
		tmr_on(5);
		pwm_set(2048, 0.5);
		n = 8;
		m = 4;
	}
	else if(i == 128) // Start of 4-beep section
	{
		n = 16;
		m = 8;
	}
	else if(i == 384) // Start of continuous-beep section
	{
		n = 32;
		m = 32;
	}
	else if(i == 896) // Done
	{
		snd_stop();
		return false;
	}

	if(i < 128) tmr_set(5, 125);
	else if(i < 384) tmr_set(5, 62.5);
	else tmr_set(5, 31.25);

	if(i % n < m) pwm_toggle();
	i++;
	return true;
}

bool snd_rise(void)
{
	static uint8_t n = 8; // Beep pattern repeats after n timer periods
	static uint8_t m = 4; // Beeps occur during the 1st m of the n periods

	if(i == 0) // Start of 2-beep section
	{
		tmr_on(5);
		pwm_set(512, 0.5);
		n = 8;
		m = 4;
	}
	else if(i == 128) // Start of 4-beep section
	{
		pwm_set(1024, 0.5);
		n = 16;
		m = 8;
	}
	else if(i == 384) // Start of continuous-beep section
	{
		pwm_set(2048, 0.5);
		n = 32;
		m = 32;
	}
	else if(i == 896) // Done
	{
		snd_stop();
		return false;
	}

	if(i < 128) tmr_set(5, 125);
	else if(i < 384) tmr_set(5, 62.5);
	else tmr_set(5, 31.25);

	if(i % n < m) pwm_toggle();
	i++;
	return true;
}

bool snd_incr(void)
{
	static uint8_t m = 0; // Beep ceases after m timer periods

	if(i == 0)
	{
		tmr_on(5);
		pwm_set(2048, 0.5);
		m = 0;
	}
	else if(i == 1536)
	{
		snd_stop();
		return false;
	}

	if(i % 384 == 0) m++;

	uint8_t p = (uint8_t)(i % 32);
	if(p == 0 || p == 8) pwm_on();
	else if(p == m || p == m + 8) pwm_off();

	tmr_set(5, 31.25);
	i++;
	return true;
}

bool snd_quiet(void)
{
	static uint8_t n = 32;

	if(i == 0)
	{
		tmr_on(5);
		pwm_set(1024, 0.5);
		n = 32;
	}
	else if(i == 512)
	{
		n = 16;
	}
	else if(i == 768)
	{
		n = 32;
	}
	else if(i == 896)
	{
		snd_stop();
		return false;
	}

	if(i < 512) tmr_set(5, 31.25);
	else if(i < 768) tmr_set(5, 62.5);
	else tmr_set(5, 125);

	if(i % n < 2) pwm_toggle();
	i++;
	return true;
}

#define NTONES 193U

static uint8_t const pr2_seq[NTONES] =
{
	255, 253, 251, 250, 248, 246, 244, 242, 241, 239, 237, 235, 234, 232, 230, 229,
	227, 225, 224, 222, 221, 219, 217, 216, 214, 213, 211, 210, 208, 207, 205, 204,
	202, 201, 199, 198, 196, 195, 194, 192, 191, 189, 188, 187, 185, 184, 183, 181,
	180, 179, 177, 176, 175, 174, 172, 171, 170, 169, 167, 166, 165, 164, 163, 161,
	160, 159, 158, 157, 156, 155, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144,
	143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128,
	127, 126, 125, 124, 123, 122, 122, 121, 120, 119, 118, 117, 116, 116, 115, 114,
	113, 112, 111, 111, 110, 109, 108, 107, 107, 106, 105, 104, 104, 103, 102, 101,
	101, 100,  99,  98,  98,  97,  96,  96,  95,  94,  94,  93,  92,  91,  91,  90,
	 90,  89,  88,  88,  87,  86,  86,  85,  84,  84,  83,  83,  82,  81,  81,  80,
	 80,  79,  78,  78,  77,  77,  76,  76,  75,  75,  74,  73,  73,  72,  72,  71,
	 71,  70,  70,  69,  69,  68,  68,  67,  67,  66,  66,  65,  65,  64,  64,  63,
	 63,
};

static uint8_t const ccpr7l_seq[NTONES] =
{
	128, 127, 126, 125, 124, 123, 122, 121, 120, 120, 119, 118, 117, 116, 115, 114,
	114, 113, 112, 111, 110, 110, 109, 108, 107, 106, 106, 105, 104, 103, 103, 102,
	101, 100, 100,  99,  98,  98,  97,  96,  96,  95,  94,  93,  93,  92,  91,  91,
	 90,  89,  89,  88,  88,  87,  86,  86,  85,  84,  84,  83,  83,  82,  81,  81,
	 80,  80,  79,  79,  78,  77,  77,  76,  76,  75,  75,  74,  74,  73,  73,  72,
	 71,  71,  70,  70,  69,  69,  68,  68,  67,  67,  66,  66,  66,  65,  65,  64,
	 64,  63,  63,  62,  62,  61,  61,  60,  60,  60,  59,  59,  58,  58,  57,  57,
	 57,  56,  56,  55,  55,  55,  54,  54,  53,  53,  53,  52,  52,  52,  51,  51,
	 50,  50,  50,  49,  49,  49,  48,  48,  48,  47,  47,  47,  46,  46,  46,  45,
	 45,  45,  44,  44,  44,  43,  43,  43,  42,  42,  42,  41,  41,  41,  41,  40,
	 40,  40,  39,  39,  39,  39,  38,  38,  38,  37,  37,  37,  37,  36,  36,  36,
	 36,  35,  35,  35,  35,  34,  34,  34,  34,  33,  33,  33,  33,  32,  32,  32,
	 32,
};

static uint8_t const dc7b_seq[NTONES] =
{
	0, 0, 1, 1, 1, 2, 2, 3, 3, 0, 0, 1, 2, 2, 3, 3, 0, 1, 2, 2, 3, 0, 1, 2, 3, 3,
	0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 0, 1, 2, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2,
	0, 1, 3, 0, 2, 3, 1, 2, 0, 2, 3, 1, 3, 0, 2, 0, 1, 3, 1, 3, 0, 2, 0, 2, 0, 2,
	0, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 0, 2, 0, 2, 0, 2, 0, 3, 1, 3, 1, 3,
	2, 0, 2, 0, 3, 1, 3, 2, 0, 2, 1, 3, 2, 0, 2, 1, 3, 2, 0, 3, 1, 0, 2, 1, 3, 2,
	0, 3, 1, 0, 3, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 3, 1, 0, 3, 2, 0, 3,
	2, 1, 0, 2, 1, 0, 3, 2, 1, 0, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3,
	2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0,
};

bool snd_siren1(void)
{
	if(i == 0)
	{
		tmr_on(5);
		PR2 = pr2_seq[0];
		pwm_on();
	}
	else if(i == 4800)
	{
		snd_stop();
		return false;
	}

	uint16_t j = i % ((NTONES - 1) * 2);
	if(j >= (NTONES - 1)) j = (NTONES - 1) * 2 - j;

	PR2 = pr2_seq[j];
	CCPR7L = ccpr7l_seq[j];
	CCP7CONbits.DC7B = dc7b_seq[j];

	tmr_set(5, 10);
	i++;
	return true;
}

bool snd_siren2(void)
{
	if(i == 0)
	{
		tmr_on(5);
		PR2 = pr2_seq[0];
		pwm_on();
	}
	else if(i == 4800)
	{
		snd_stop();
		return false;
	}

	uint16_t j = i % ((NTONES - 1) * 2);
	if(j >= (NTONES - 1)) j = NTONES - 1;

	PR2 = pr2_seq[j];
	CCPR7L = ccpr7l_seq[j];
	CCP7CONbits.DC7B = dc7b_seq[j];

	tmr_set(5, 10);
	i++;
	return true;
}

Sound const sounds[NUM_SNDS] =
{
	[PLAIN] = {"beep", snd_plain},
	[RISE] = {"RISe", snd_rise},
	[INCR] = {"Incr.", snd_incr},
	[QUIET] = {"QUIe.", snd_quiet},
	[SRN1] = {"SRN1", snd_siren1},
	[SRN2] = {"SRN2", snd_siren2},
};