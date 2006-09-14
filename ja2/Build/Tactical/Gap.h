#ifndef __GAP_H
#define __GAP_H

#include "Faces.h"

typedef UINT8		AudioSample8;
typedef INT16		AudioSample16;



void AudioGapListInit(const char *zSoundFile, AudioGapList	*pGapList );
void AudioGapListDone( AudioGapList	*pGapList );

void PollAudioGap( UINT32 uiSampleNum, AudioGapList *pGapList );


UINT32 PlayJA2GapSample( const char *zSoundFile, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData );



#endif
