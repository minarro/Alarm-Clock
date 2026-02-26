#ifndef BCD_H_
#define BCD_H_

#include <stdint.h>

static inline uint8_t bcd2b(uint8_t n)
{
	return ((n & 0xF0) >> 4) * 10 + (n & 0x0F);
}

static inline uint8_t b2bcd(uint8_t n)
{
	return (uint8_t)(((n / 10) << 4) + (n % 10));
}

#endif