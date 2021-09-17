#include "Debug.h"
#include "FileMan.h"
#include "Gap.h"
#include "LoadSaveData.h"
#include "MemMan.h"
#include "SoundMan.h"
#include "Sound_Control.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include "Logger.h"

static void AudioGapListInit(const char* zSoundFile, AudioGapList* pGapList)
{
	// This procedure will load in the appropriate .gap file, corresponding to
	// the .wav file in szSoundEffects indexed by uiSampleNum.  The procedure
	// will then allocate and load in the AUDIO_GAP information, while counting
	// the number of elements loaded

	SLOGD("File is %s", zSoundFile);

	// strip .wav and change to .gap
	ST::string sFileName(FileMan::replaceExtension(zSoundFile, "gap"));

	try
	{
		AutoSGPFile f(GCM->openGameResForReading(sFileName));

		// gap file exists
		// now read in the AUDIO_GAPs
		const UINT32 size = f->size();

		const UINT32 count = size / 8;
		if (count > 0)
		{
			BYTE *data = new BYTE[size]{};
			f->read(data, size);

			AUDIO_GAP* const gaps  = new AUDIO_GAP[count]{};

			pGapList->gaps = gaps;
			pGapList->end  = gaps + count;

			DataReader d{data};
			for (UINT32 i = 0; i < count; ++i)
			{
				UINT32 start;
				UINT32 end;

				EXTR_U32(d, start);
				EXTR_U32(d, end);

				gaps[i].start = start;
				gaps[i].end   = end;

				SLOGD("Gap Start %d and Ends %d", start, end);
			}
			Assert(d.getConsumed() == size);

			SLOGD("gap list started from file %s", sFileName.c_str());

			delete[] data;
			return;
		}
	}
	catch (...) { /* Handled below */ }

	pGapList->gaps = NULL;
	pGapList->end  = NULL;
}


void AudioGapListDone(AudioGapList* pGapList)
{
	// Free the array and nullify the pointers in the AudioGapList
	delete[] pGapList->gaps;
	pGapList->gaps = NULL;
	pGapList->end  = NULL;
	SLOGD("Audio Gap List Deleted");
}


BOOLEAN PollAudioGap(UINT32 uiSampleNum, AudioGapList* pGapList)
{
	// This procedure will access the AudioGapList pertaining to the .wav about
	// to be played and returns whether there is a gap currently.  This is done
	// by going to the current AUDIO_GAP element in the AudioGapList, comparing
	// to see if the current time is between the start and end. If so, return
	// TRUE..if not and the start of the next element is not greater than
	// current time, set current to next and repeat ...if next elements start
	// is larger than current time, or no more elements..  return FALSE

	if (!pGapList)
	{
		// no gap list, return
		return FALSE;
	}

	const AUDIO_GAP* i = pGapList->gaps;
	if (i == NULL) return FALSE;

	const UINT32 time = SoundGetPosition(uiSampleNum);
	SLOGD("Sound Sample Time is %d", time);

	// Check to see if we have fallen behind.  If so, catch up
	const AUDIO_GAP* const end = pGapList->end;
	while (time > i->end)
	{
		if (++i == end) return FALSE;
	}

	// check to see if time is within the next AUDIO_GAPs start time
	if (i->start < time && time < i->end)
	{
		// we are within the time frame
		SLOGD("Gap Started at %d", time);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


UINT32 PlayJA2GapSample(const char* zSoundFile, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData)
{
	// Setup Gap Detection, if it is not null
	if (pData != NULL) AudioGapListInit(zSoundFile, pData);

	const UINT32 vol = CalculateSpeechVolume(ubVolume);
	return SoundPlay(zSoundFile, vol, uiPan, ubLoops, NULL, NULL);
}
