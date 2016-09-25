#ifndef _MUSIC_CONTROL_H_
#define _MUSIC_CONTROL_H_

#include "SoundMan.h"

enum MusicList {
	MARIMBAD2_MUSIC,
	MENUMIX_MUSIC,
	NOTHING_A_MUSIC,
	NOTHING_B_MUSIC,
	NOTHING_C_MUSIC,
	NOTHING_D_MUSIC,
	TENSOR_A_MUSIC,
	TENSOR_B_MUSIC,
	TENSOR_C_MUSIC,
	TRIUMPH_MUSIC,
	DEATH_MUSIC,
	BATTLE_A_MUSIC,
	BATTLE_B_MUSIC, //same as tensor B
	CREEPY_MUSIC,
	CREATURE_BATTLE_MUSIC,
	NUM_MUSIC
};

extern const char* const szMusicList[NUM_MUSIC];

extern UINT8		gubMusicMode;
extern BOOLEAN	gfForceMusicToTense;


void SetMusicMode(UINT8 ubMusicMode);

/* Starts up one of the tunes in the music list. */
void MusicPlay(UINT32 uiNum);

void MusicSetVolume(UINT32 uiVolume);
UINT32 MusicGetVolume(void);

/* Handles any maintenance the music system needs done. Should be polled from
 * the main loop, or somewhere with a high frequency of calls. */
void MusicPoll(void);

void SetMusicFadeSpeed( INT8 bFadeSpeed );

#endif
