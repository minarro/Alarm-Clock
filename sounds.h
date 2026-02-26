#ifndef SOUNDS_H_
#define SOUNDS_H_

#include <stdbool.h>
#include <stdint.h>

enum Sounds{PLAIN, RISE, INCR, QUIET, SRN1, SRN2, NUM_SNDS};

typedef struct Sound Sound;
struct Sound
{
	char const *const name;
	bool (*const play)(void);
};

extern Sound const sounds[NUM_SNDS];

/* Disables the sound and resets the sequence counter */
void snd_stop(void);

/* These functions produce a sequence of beeps or tones using PWM.
 * They return after each step in the sequence and must be called again when
 * TMR5 rolls over. They return false when end of sequence is reached,
 * otherwise true. Only one function should be used at a time!
 */

bool snd_plain(void);
bool snd_rise(void);
bool snd_incr(void);
bool snd_quiet(void);
bool snd_siren1(void);
bool snd_siren2(void);

#endif
