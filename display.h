#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

enum Digits{DIG1, DIG2, DIG3, DIG4, NDIG};
enum Sections{SECT_D1, SECT_D2, SECT_D3, SECT_D4, SECT_L, NSECT};

void multiplex(void);
void displayDigit(uint8_t);
void displayInt(int, uint8_t fieldWidth);
void displayUint(unsigned, uint8_t fieldWidth, char padding);
void displayChar(char);
void displayString(char const *);
void clearDisplayBuffer(void);
void loadDisplayBuffer(void);
void dpOn(uint8_t pos);
void dpOff(uint8_t pos);
void colonOn(void);
void colonOff(void);
void upperDotOn(void);
void upperDotOff(void);
void print(char const *, ...);

#endif