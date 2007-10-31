#include "Debug.h"
#include "FileMan.h"
#include "Gap.h"
#include "MemMan.h"
#include "SoundMan.h"
#include "Sound_Control.h"


static void AudioGapListInit(const char* zSoundFile, AudioGapList* pGapList)
{
	/* This procedure will load in the appropriate .gap file, corresponding to
	 * the .wav file in szSoundEffects indexed by uiSampleNum.  The procedure
	 * will then allocate and load in the AUDIO_GAP information, while counting
	 * the number of elements loaded */

	//Initialize GapList
	pGapList->pHead            = NULL;
	//DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("File is %s", szSoundEffects[uiSampleNum]));

	// strip .wav and change to .gap
	char sFileName[256];
	strcpy(sFileName, zSoundFile);
	char* const dot = strchr(sFileName, '.');
	dot[1] = 'g';
	dot[2] = 'a';
	dot[3] = 'p';
	dot[4] = '\0';

	const HWFILE pFile = FileOpen(sFileName, FILE_ACCESS_READ);
	if (pFile)
	{
		// gap file exists
		// now read in the AUDIO_GAPs
		AUDIO_GAP* pPreviousGap = NULL;
		for (;;)
		{
			UINT32 Start;
			FileRead(pFile, &Start, sizeof(UINT32));

			if (FileCheckEndOfFile(pFile)) break;

			// can read the first element, there exists a second
			UINT32 End;
			FileRead(pFile, &End, sizeof(UINT32));

			// allocate space for AUDIO_GAP
			AUDIO_GAP* const pCurrentGap = MemAlloc(sizeof(*pCurrentGap));
			if (pPreviousGap != NULL)
			{
				pPreviousGap->pNext = pCurrentGap;
			}
			else
			{
				// Start of list
				pGapList->pHead = pCurrentGap;
			}

			pCurrentGap->pNext   = 0;
			pCurrentGap->uiStart = Start;
			pCurrentGap->uiEnd   = End;
			DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap Start %d and Ends %d", Start, End));

			// Increment pointer
			pPreviousGap = pCurrentGap;
		}

		FileClose(pFile);
	}
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap List Started From File %s", sFileName));
}


void AudioGapListDone(AudioGapList* pGapList)
{
	/* This procedure will go through the AudioGapList and free space/nullify
	 * pointers for all allocated elements */
	for (AUDIO_GAP* i = pGapList->pHead; i != NULL;)
	{
		AUDIO_GAP* const next = i->pNext;
		MemFree(i);
		i = next;
	}
	pGapList->pHead = NULL;
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Audio Gap List Deleted");
}


BOOLEAN PollAudioGap(UINT32 uiSampleNum, AudioGapList* pGapList)
{
	/* This procedure will access the AudioGapList pertaining to the .wav about
	 * to be played and returns whether there is a gap currently.  This is done
	 * by going to the current AUDIO_GAP element in the AudioGapList, comparing
	 * to see if the current time is between the uiStart and uiEnd. If so, return
	 * TRUE..if not and the uiStart of the next element is not greater than
	 * current time, set current to next and repeat ...if next elements uiStart
	 * is larger than current time, or no more elements..  return FALSE */

	if (!pGapList)
	{
		// no gap list, return
		return FALSE;
	}

	const AUDIO_GAP* pCurrent = pGapList->pHead;
	if (pCurrent == NULL) return FALSE;

	const UINT32 time = SoundGetPosition(uiSampleNum);
	//DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Sound Sample Time is %d", time));

	// Check to see if we have fallen behind.  If so, catch up
	while (time > pCurrent->uiEnd)
	{
		pCurrent = pCurrent->pNext;
		if (!pCurrent) return FALSE;
	}

	// check to see if time is within the next AUDIO_GAPs start time
	if (pCurrent->uiStart < time && time < pCurrent->uiEnd)
	{
		// we are within the time frame
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Gap Started at %d", time));
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
	return SoundPlayStreamedFile(zSoundFile, vol, uiPan, ubLoops, NULL, NULL);
}
