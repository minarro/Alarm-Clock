#ifndef SCREENS_H_
#define SCREENS_H_

#include "shared-defs.h"

enum Screens
{
	YEAR_SCN, DATE_SCN, WKDY_SCN, HOUR_SCN, MIN_SCN,
	AL_ON_SCN, AL1_SCN, AL2_SCN, TMR_SCN, STOPW_SCN,
	SNZ_SCN, SND_SCN, SMR_SCN, FMT_SCN, BAT_SCN,
	ALM_SCN, NUM_SCNS
};

typedef struct Screen Screen;
struct Screen
{
	char const *const name;
	uint8_t const field;
	void (*const drawInfo)(Screen const *self);
	void (*const draw)(State *);
};

extern Screen const screens[NUM_SCNS];

#endif
