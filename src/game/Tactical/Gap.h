#ifndef GAP_H
#define GAP_H

#include "Types.h"
#include <vector>

struct AUDIO_GAP
{
	// Times in milliseconds since the start of the underlying speech sample.
	UINT32 start;
	UINT32 end;
};

using AudioGapList = std::vector<AUDIO_GAP>;


void AudioGapListDone(AudioGapList* pGapList);
bool PollAudioGap(UINT32 uiSampleNum, AudioGapList* pGapList);
UINT32 PlayJA2GapSample(const ST::string& zSoundFile, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData);

#endif
