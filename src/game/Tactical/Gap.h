#ifndef GAP_H
#define GAP_H

#include "Faces.h"


void    AudioGapListDone(AudioGapList* pGapList);
BOOLEAN PollAudioGap(UINT32 uiSampleNum, AudioGapList* pGapList);
UINT32  PlayJA2GapSample(const ST::string& zSoundFile, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData);

#endif
