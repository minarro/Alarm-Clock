#ifndef TIME_H_
#define TIME_H_

/* Assumptions:
 * 1. t.h < 24, t.m < 60 and t.s < 60
 * 2. If time_inSeconds(t2) < time_inSeconds(t1) then t2 occurs
 * the day after t1. I.e. t2 is always later than t1.
 */

#include <stdbool.h>
#include <stdint.h>

typedef struct Time Time;
struct Time
{
	uint8_t h;
	uint8_t m;
	uint8_t s;
};

Time time_new(uint8_t h, uint8_t m, uint8_t s);
uint24_t time_inSeconds(Time t);
uint24_t time_diff_seconds(Time t2, Time t1);
Time time_sum(Time t2, Time t1);
Time time_diff(Time t2, Time t1);
bool time_isEqual(Time t2, Time t1);
Time time_now(void);
Time time_round(Time t);

#endif
