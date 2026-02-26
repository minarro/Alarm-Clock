#include "display.h"
#include <xc.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SPACE 0
#define BLANK SPACE
#define MINUS 128
#define HYPHEN MINUS
#define DASH MINUS
#define UNDERSCORE 8
#define QUESTION 163
#define SLASH 162
#define COLON 3
#define APOSTROPHE 4
#define ASTERISK 195

#define DECIMAL_POINT 0b00010000
#define FULL_STOP DECIMAL_POINT

static uint8_t buf1_index = SECT_D1;
static uint8_t buf1[NSECT] = {0};
static uint8_t buf2[NSECT] = {0};
static bool loadFlag = false;

static uint8_t const digitPatterns[10] =
{
	111,   6, 171, 143, 198,
	205, 237,   7, 239, 207,
};

static uint8_t const upperLetterPatterns[26] =
{
	231, 239, 105, 111, 233, 225, 109,
	230,   6,  14, 234, 104,  99, 103,
	111, 227, 199,  97, 205, 232,
	110, 110, 108,  15, 198, 171,
};

static uint8_t const lowerLetterPatterns[26] =
{
	175, 236, 168, 174, 235, 225, 207,
	228,   4,  15, 234, 104, 164, 164,
	172, 227, 119, 160, 205, 232,
	 44,  44,  44, 140, 206, 171,
};
static inline void disableSections(void)
{
	TRISB |= 0b00001110;
	TRISC |= 0b11000000;
}

static inline void enableSection(uint8_t n)
{
	switch(n)
	{
		case SECT_D1:
			TRISCbits.TRISC6 = 0;
			break;
		case SECT_D2:
			TRISCbits.TRISC7 = 0;
			break;
		case SECT_D3:
			TRISBbits.TRISB2 = 0;
			break;
		case SECT_D4:
			TRISBbits.TRISB3 = 0;
			break;
		case SECT_L:
			TRISBbits.TRISB1 = 0;
			break;
	}
}

static inline void outputToSegments(uint8_t pattern)
{
	LATA = pattern;
	LATCbits.LC2 = (bool)(pattern & DECIMAL_POINT); // 18F27J53 has no RA4
}

void multiplex(void)
{
	static uint8_t s = SECT_D1;
	disableSections();
	outputToSegments(buf2[s]);
	enableSection(s);
	if(++s == NSECT)
	{
		s = SECT_D1;
		if(loadFlag)
		{
			memcpy(buf2, buf1, sizeof(buf2));
			loadFlag = false;
		}
	}
}

void clearDisplayBuffer(void)
{
	memset(buf1, 0, sizeof(buf1));
	buf1_index = 0;
}

void loadDisplayBuffer(void)
{
	loadFlag = true;
}

void dpOn(uint8_t pos)
{
	buf1[pos] |= DECIMAL_POINT;
}

void dpOff(uint8_t pos)
{
	buf1[pos] &= ~DECIMAL_POINT;
}

void colonOn(void)
{
	buf1[SECT_L] |= COLON;
}

void colonOff(void)
{
	buf1[SECT_L] &= ~COLON;
}

void upperDotOn(void)
{
	buf1[SECT_L] |= APOSTROPHE;
}

void upperDotOff(void)
{
	buf1[SECT_L] &= ~APOSTROPHE;
}

void displayDigit(uint8_t d)
{
	if(d > 9 || buf1_index >= NDIG) return;
	buf1[buf1_index] &= DECIMAL_POINT;
	buf1[buf1_index] |= digitPatterns[d];
	if(buf1_index < NDIG) buf1_index++;
}

static inline uint8_t getSecondHalf(char c)
{
	if(c == 'M') return 71;
	if(c == 'm') return 164;
	if(c == 'W') return 46;
	if(c == 'w') return 44;
	if(c == 'X') return 105;
	if(c == 'x') return 168;
	return 0;
}

void displayChar(char c)
{
	if(buf1_index >= NDIG) return;
	uint8_t pattern = 0;
	if(c >= 'a' && c <= 'z') pattern = lowerLetterPatterns[c - 'a'];
	else if(c >= 'A' && c <= 'Z') pattern = upperLetterPatterns[c - 'A'];
	else if(c >= '0' && c <= '9') pattern = digitPatterns[c - '0'];
	else if(c == ' ') pattern = SPACE;
	else if(c == '-') pattern = MINUS;
	else if(c == '_') pattern = UNDERSCORE;
	else if(c == '?') pattern = QUESTION;
	else if(c == '/') pattern = SLASH;
	else if(c == '*') pattern = ASTERISK;

	buf1[buf1_index] &= DECIMAL_POINT;
	buf1[buf1_index] |= pattern;
	if(buf1_index < NDIG) buf1_index++;

	pattern = getSecondHalf(c);
	if(pattern && buf1_index < NDIG)
	{
		buf1[buf1_index] &= DECIMAL_POINT;
		buf1[buf1_index] |= pattern;
		if(buf1_index < NDIG) buf1_index++;
	}
}

void displayUint(unsigned x, uint8_t fieldWidth, char padding)
{
	uint8_t ndigits;
	if(x > 9999) ndigits = 5;
	else if(x > 999) ndigits = 4;
	else if(x > 99) ndigits = 3;
	else if(x > 9) ndigits = 2;
	else ndigits = 1;

	uint8_t padLen = (fieldWidth > ndigits)? fieldWidth - ndigits : 0;
	for(uint8_t i = 0; i < padLen; i++)
	{
		displayChar(padding);
	}

	uint16_t d, n;
	static uint16_t const pow10[5] = {1, 10, 100, 1000, 10000};
	for(uint8_t i = ndigits - 1; i > 0; i--)
	{
		n = pow10[i];
		d = x / n;
		displayDigit((uint8_t)d);
		x %= n;
	}
	displayDigit((uint8_t)x); // Print last digit even if zero
}

void displayInt(int x, uint8_t fieldWidth)
{
	if(x >= 0)
	{
		displayUint((unsigned)x, fieldWidth, ' ');
		return;
	}

	unsigned abs_x = (unsigned)(x * -1);
	uint8_t nchars; // Including minus
	if(abs_x > 9999) nchars = 6;
	else if(abs_x > 999) nchars = 5;
	else if(abs_x > 99) nchars = 4;
	else if(abs_x > 9) nchars = 3;
	else nchars = 2;

	uint8_t padLen = (fieldWidth > nchars)? fieldWidth - nchars : 0;
	for(uint8_t i = 0; i < padLen; i++)
	{
		displayChar(' ');
	}
	displayChar('-');
	displayUint(abs_x, 0, ' ');
}

void displayString(char const *s)
{
	char c;
	while((c = *s++))
	{
		if(c == '.')
		{
			if(buf1_index > DIG1 && buf1_index <= NDIG) buf1[buf1_index - 1] |= DECIMAL_POINT;
		}
		else
		{
		displayChar(c);
		}
	}
}

/* Note idiosyncratic usage, especially %d, :, ', and . */
void print(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	char c;
	while((c = *fmt++))
	{
		switch(c)
		{
			case '.':
				if(buf1_index > DIG1 && buf1_index <= NDIG)
				{
					buf1[buf1_index - 1] |= DECIMAL_POINT;
				}
				break;
			case ':':
				colonOn();
				break;
			case '\'':
				upperDotOn();
				break;
			case '%':
				switch((c = *fmt++))
				{
					case '%':
						buf1[NDIG - 1] = SLASH | DECIMAL_POINT;
						upperDotOn();
						break;
					case 'd':
						displayDigit(va_arg(ap, uint8_t));
						break;
					case 'c':
						displayChar(va_arg(ap, char));
						break;
					case 's':
						displayString(va_arg(ap, char const *));
						break;
					case 'i':
						displayInt(va_arg(ap, int), 0);
						break;
					case 'u':
						displayUint(va_arg(ap, unsigned), 0, ' ');
						break;
					case '0':
						if(isdigit(*fmt) && *(fmt + 1) == 'u')
						{
							uint8_t fieldWidth = (uint8_t)(*fmt - '0');
							displayUint(va_arg(ap, unsigned), fieldWidth, '0');
							fmt += 2;
						}
						break;
					default:
						if(isdigit(c))
						{
							uint8_t fieldWidth = (uint8_t)(c - '0');
							char d = *fmt++;
							if(d == 'i')
							{
								displayInt(va_arg(ap, int), fieldWidth);
							}
							else if(d == 'u')
							{
								displayUint(va_arg(ap, unsigned), fieldWidth, ' ');
							}
						}
						break;
				}
				break;
			default:
				displayChar(c);
				break;
		}
	}
	va_end(ap);
}
