#include "time.h"
#include <stdbool.h>
#include <stdint.h>
#include "rtc.h"

uint24_t time_inSeconds(Time t)
{
	return t.h * UINT24_C(3600) + t.m * UINT16_C(60) + t.s;
}

uint24_t time_diff_seconds(Time t2, Time t1)
{
	uint24_t s1 = time_inSeconds(t1);
	uint24_t s2 = time_inSeconds(t2);
	return (s2 > s1)? s2 - s1 : UINT24_C(86400) - s1 + s2;
}

Time time_new(uint8_t h, uint8_t m, uint8_t s)
{
	Time t;
	t.h = h; t.m = m; t.s = s;
	return t;
}

Time time_sum(Time t2, Time t1)
{
	uint8_t s = t2.s + t1.s; // Max 118
	uint8_t m = t2.m + t1.m; // Max 118
	uint8_t h = t2.h + t1.h; // Max 46

	if(s > 59){s -= 60; m += 1;}
	if(m > 59){m -= 60; h += 1;}
	if(h > 23){h -= 24;}

	return time_new(h, m, s);
}

Time time_diff(Time t2, Time t1)
{
	int8_t s = (int8_t)(t2.s - t1.s); // Min -59
	int8_t m = (int8_t)(t2.m - t1.m); // Min -59
	int8_t h = (int8_t)(t2.h - t1.h); // Min -23

	if(s < 0){s += 60; m -= 1;}
	if(m < 0){m += 60; h -= 1;}
	if(h < 0){h += 24;}

	return time_new((uint8_t)h, (uint8_t)m, (uint8_t)s);
}
bool time_isEqual(Time t2, Time t1)
{
	return ((t2.h == t1.h) && (t2.m == t1.m) && (t2.s == t1.s));
}

Time time_now(void)
{
	return time_new(rtc_read(HOUR_REG), rtc_read(MIN_REG), rtc_read(SEC_REG));
}

Time time_round(Time t)
{
	Time r = time_sum(t, (Time){0, 0, 59});
	r.s = 0;
	return r;
}
