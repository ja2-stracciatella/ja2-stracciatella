#ifndef __GAP_H
#define __GAP_H

#include "faces.h"

typedef UINT8		AudioSample8;
typedef INT16		AudioSample16;



void AudioGapListInit( CHAR8 *zSoundFile, AudioGapList	*pGapList );
void AudioGapListDone( AudioGapList	*pGapList );

void PollAudioGap( UINT32 uiSampleNum, AudioGapList *pGapList );


UINT32 PlayJA2GapSample( CHAR8 *zSoundFile, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData );



#endif
