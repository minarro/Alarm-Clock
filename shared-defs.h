#ifndef SHARED_DEFS_H_
#define SHARED_DEFS_H_

#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

#define _XTAL_FREQ 8000000

// Use the following constants to make rest of code indepentent of _XTAL_FREQ
#if _XTAL_FREQ == 8000000
	#define IRCF_SEL 7
	#define TMR2_PS_SEL 2
	#define TMR2_PS_VAL 16
#elif _XTAL_FREQ == 4000000
	#define IRCF_SEL 6
	#define TMR2_PS_SEL 2
	#define TMR2_PS_VAL 16
#elif _XTAL_FREQ == 2000000
	#define IRCF_SEL 5
	#define TMR2_PS_SEL 1
	#define TMR2_PS_VAL 4
#elif _XTAL_FREQ == 1000000
	#define IRCF_SEL 4
	#define TMR2_PS_SEL 1
	#define TMR2_PS_VAL 4
#else
	#error _XTAL_FREQ set to unsupported value
#endif

enum Format{TWELVE_HOUR, TWENTYFOUR_HOUR};
enum Modes{NORMAL, INFO, EDIT, SOUND_TEST};

typedef struct State State;
struct State
{
	uint8_t mode;
	uint8_t screen;
	uint8_t field;
	bool alarmSounding;
	bool displayEnabled;
};

extern volatile bool blinkFlag;
extern volatile bool shiftFlag;

static uint16_t round(uint16_t a)
{
	return ((a + 5) / 10) * 10;
}

static inline uint16_t absDiff(uint16_t a, uint16_t b)
{
	return a > b? a - b : b - a;
}

static inline uint16_t max(uint16_t a, uint16_t b)
{
	return a > b? a : b;
}

static inline uint16_t min(uint16_t a, uint16_t b)
{
	return a < b? a : b;
}

#ifndef nullptr
	#define nullptr (void *)0
#endif

#endif