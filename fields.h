#ifndef FIELDS_H_
#define FIELDS_H_

#include <stdbool.h>
#include <stdint.h>

enum Fields
{
	CENTURY_FLD, YEAR_FLD, DAY_FLD, MONTH_FLD, WKDY_FLD, HOUR_FLD,
	MIN_FLD, SEC_FLD, A1_ON_FLD, A2_ON_FLD, A1_HR_FLD, A1_MIN_FLD,
	A2_HR_FLD, A2_MIN_FLD, TMR_HR_FLD, TMR_MIN_FLD, STOPW_FLD, SNZ_FLD,
	SND_FLD, BAT_FLD, SMR_FLD, FMT_FLD, ALM_FLD, NUM_FLDS
};

typedef struct Field Field;
struct Field
{
	uint16_t const max;
	uint8_t const min: 1;
	uint8_t const editable: 1;
	uint8_t const screen: 6;
	uint16_t (*const get)(void);
	void (*const set)(uint16_t);
	void (*const inc)(Field const *self);
	void (*const dec)(Field const *self);
};

extern Field const fields[NUM_FLDS];

void startTimerEdit(void);
void endTimerEdit(void);
bool inTimerEdit(void);

#endif