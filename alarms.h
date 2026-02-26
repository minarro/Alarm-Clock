#ifndef ALARMS_H_
#define ALARMS_H_

#include <stdbool.h>
#include <stdint.h>
#include "sounds.h"
#include "time.h"

enum Alarms{ALM1, ALM2, SNZ1, SNZ2, TMR_ALM, SMR_ALM, STD_ALM, LEAP1_ALM, LEAP2_ALM, NUM_ALMS, NO_ALM};

typedef struct Alarm Alarm;
struct Alarm
{
	char const *const name;
	uint8_t const index; // Index for alarms_data array
	uint8_t const snz: 5; // Index of snooze or NO_ALM
	uint8_t const audible: 1; // Triggers sound playback
	uint8_t const repeating: 1; // Not automatically disabled
	uint8_t const relative: 1; // Alarm time changes if time of day is changed
	void (*const action)(void);
};

typedef struct AlarmData AlarmData;
struct AlarmData
{
	Time time;
	bool enabled;
	bool raised;
};

extern Alarm const alarms[NUM_ALMS]; // In program memory
extern AlarmData alarms_data[NUM_ALMS]; // In data memory

void adjRelAlms(Time adjustment, int8_t direction);
uint8_t findNextAlarm(bool audibleOnly); // To hardware
uint8_t writeNextAlarm(void);
bool raiseAlarms(void);
void snooze(uint8_t alm, uint16_t dur);

#define getNextAlarm() findNextAlarm(false)
#define getNextAudibleAlarm() findNextAlarm(true)

#endif
