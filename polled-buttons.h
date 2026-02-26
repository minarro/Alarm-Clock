#ifndef POLLED_BUTTONS_H_
#define POLLED_BUTTONS_H_

#include <stdbool.h>
#include <stdint.h>
#include "button-config.h"
#include "shared-defs.h"

typedef struct Button Button;
struct Button
{
	void (* const action)(State *);
	bool (*const isPressed)(void);
	uint8_t const index: 7;
	uint8_t const repeating: 1;
};

// Can't be in button-config.h because struct Button isn't defined there
extern Button const buttons[NUM_BTNS];

static inline uint8_t pollButtons(void)
{
	for(uint8_t i = 0; i < NUM_BTNS; i++)
	{
		Button const *btn = buttons + i;
		if(btn->isPressed()) return btn->index;
	}
	return NO_BTN;
}

#endif