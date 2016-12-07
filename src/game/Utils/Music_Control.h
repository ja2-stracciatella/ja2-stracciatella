#ifndef _MUSIC_CONTROL_H_
#define _MUSIC_CONTROL_H_

#include "SoundMan.h"
#include "UTF8String.h"
#include "ContentMusic.h"

extern MusicMode gubMusicMode;
extern BOOLEAN	gfForceMusicToTense;


void SetMusicMode(MusicMode ubMusicMode);

/* Starts up one of the tunes in the music list. */
void MusicPlay(const UTF8String* pFilename);

void MusicSetVolume(UINT32 uiVolume);
UINT32 MusicGetVolume(void);

/* Handles any maintenance the music system needs done. Should be polled from
 * the main loop, or somewhere with a high frequency of calls. */
void MusicPoll(void);

void SetMusicFadeSpeed( INT8 bFadeSpeed );

#endif
