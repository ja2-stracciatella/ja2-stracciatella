#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include "Debug.h"
	#include "Types.h"
	//#include "mssw.h"
	#include "Gap.h"
	#include "Sound_Control.h"
	#include "SoundMan.h"
	#include "Timer_Control.h"
	#include <stdio.h>
	#include "FileMan.h"
	#include <string.h>
#endif

#if 0
static void AILCALLBACK timer_func( UINT32 user )
{
	AudioGapList	*pGapList;

	pGapList = (AudioGapList*)user;

  pGapList->gap_monitor_timer += GAP_TIMER_INTERVAL;

  if ( pGapList->audio_gap_active )
  {
   if ( (pGapList->gap_monitor_timer / 1000) > pGapList->end[ pGapList->gap_monitor_current] )
    {
      pGapList->audio_gap_active = 0;
      pGapList->gap_monitor_current++;
    }
  }
  else
	{
    if ( pGapList->gap_monitor_current < pGapList->count )
		{
      if ( ( pGapList->gap_monitor_timer / 1000) >= pGapList->start[ pGapList->gap_monitor_current ])
      {
				pGapList->audio_gap_active = 1;
      }
		}
	}
}
#endif


void AudioGapListInit( CHAR8 *zSoundFile, AudioGapList	*pGapList )
{
	// This procedure will load in the appropriate .gap file, corresponding
	// to the .wav file in szSoundEffects indexed by uiSampleNum
	// The procedure will then allocate and load in the AUDIO_GAP information,
	// while counting the number of elements loaded

//	FILE *pFile;
	HWFILE pFile;
	STR pSourceFileName;
	STR pDestFileName;
	char sFileName[256];
	UINT8 counter=0;
	AUDIO_GAP *pCurrentGap, *pPreviousGap;
	UINT32 Start;
	UINT32	uiNumBytesRead;

	UINT32 End;


	pSourceFileName= zSoundFile;
	pDestFileName=sFileName;
	//Initialize GapList
	pGapList->size=0;
	pGapList->current_time=0;
	pGapList->pHead=0;
	pGapList->pCurrent=0;
	pGapList->audio_gap_active=FALSE;
	pPreviousGap=pCurrentGap=0;
	//DebugMsg(TOPIC_JA2, DBG_LEVEL_3,String("File is %s", szSoundEffects[uiSampleNum]));
	// Get filename
	strcpy(pDestFileName, pSourceFileName);
	// strip .wav and change to .gap

	while(pDestFileName[counter] !='.')
	{
   		counter++;
	}

	pDestFileName[counter+1]='g';
	pDestFileName[counter+2]='a';
	pDestFileName[counter+3]='p';
	pDestFileName[counter+4]='\0';

	pFile = FileOpen(pDestFileName, FILE_ACCESS_READ, FALSE );
	if( pFile )
	{
			counter=0;
		 // gap file exists
		 // now read in the AUDIO_GAPs

		 //fread(&Start,sizeof(UINT32), 1, pFile);
		 FileRead( pFile, &Start,sizeof(UINT32), &uiNumBytesRead );


		//	while ( !feof(pFile) )
			while ( !FileCheckEndOfFile( pFile ) )
			{
				 // can read the first element, there exists a second
					//fread(&End, sizeof(UINT32),1,pFile);
				 FileRead( pFile, &End, sizeof(UINT32), &uiNumBytesRead );

				 // allocate space for AUDIO_GAP
				 pCurrentGap = MemAlloc( sizeof(AUDIO_GAP) );
				 if (pPreviousGap !=0)
						 pPreviousGap->pNext=pCurrentGap;
				 else
						{
						 // Start of list
						 pGapList->pCurrent=pCurrentGap;
						 pGapList->pHead=pCurrentGap;
						}

				 pGapList->size++;
				 pCurrentGap->pNext=0;
				 pCurrentGap->uiStart=Start;
				 pCurrentGap->uiEnd=End;
				 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Gap Start %d and Ends %d", Start, End) );

				 // Increment pointer
				 pPreviousGap=pCurrentGap;

				//	fread(&Start,sizeof(UINT32), 1, pFile);
				 FileRead( pFile, &Start, sizeof(UINT32), &uiNumBytesRead );
			}

			pGapList->audio_gap_active=FALSE;
			pGapList->current_time=0;


	//fclose(pFile);
			 FileClose( pFile );
		 }
		 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Gap List Started From File %s and has %d gaps", pDestFileName,  pGapList->size) );

}

void AudioGapListDone( AudioGapList	*pGapList )
{
 // This procedure will go through the  AudioGapList and free space/nullify pointers
 // for any allocated elements

	AUDIO_GAP *pCurrent, *pNext;
 if (pGapList->pHead !=0)
		{

	   pCurrent=pGapList->pHead;
     pNext=pCurrent->pNext;
	   // There are elements in the list
	    while(pNext !=0)
						{
             // kill pCurrent
				     MemFree(pCurrent);
						 pCurrent=pNext;
						 pNext=pNext->pNext;
						}
			// now kill the last element
			MemFree(pCurrent);
      pCurrent=0;

		}
 pGapList->pHead=0;
 pGapList->pCurrent=0;
 pGapList->size = 0;
 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Audio Gap List Deleted") );
}



void PollAudioGap( UINT32 uiSampleNum, AudioGapList *pGapList )
{
 // This procedure will access the AudioGapList pertaining to the .wav about
 // to be played and sets the audio_gap_active flag. This is done by
 // going to the current AUDIO_GAP element in the AudioGapList, comparing to see if
 //	the current time is between the uiStart and uiEnd. If so, set flag..if not and
 // the uiStart of the next element is not greater than current time, set current to next and repeat
 // ...if next elements uiStart is larger than current_time, or no more elements..
 // set flag FALSE

	UINT32 time;
 AUDIO_GAP *pCurrent;

 if(!pGapList)
 {
	 // no gap list, return
	 return;
 }

 if (pGapList->size > 0)
 {
  time=SoundGetPosition(uiSampleNum);
//  DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Sound Sample Time is %d", time) );
 }
 else
 {
    pGapList->audio_gap_active=(FALSE);
		return;
 }

 // set current ot head of gap list for this sound
 pCurrent = pGapList -> pHead;

 // check to see if we have fallen behind
 if( ( time > pCurrent-> uiEnd ) )
 {
	 // fallen behind
	 // catchup
	 while( time > pCurrent -> uiEnd )
	 {
		 pCurrent = pCurrent -> pNext;
		 if( ! pCurrent )
		 {
       pGapList->audio_gap_active=(FALSE);
			 return;
		 }
	 }
 }




 // check to see if time is within the next AUDIO_GAPs start time
 if ( ( time > pCurrent ->uiStart ) && ( time < pCurrent->uiEnd ) )
			{

	     if ((time >pCurrent->uiStart)&&(time < pCurrent->uiEnd))
			 {
				 // we are within the time frame
		  	 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Gap Started at %d", time) );
				 pGapList->audio_gap_active=(TRUE);

			 }
       else if ( (time > pCurrent->uiEnd)&&(pGapList->audio_gap_active == TRUE) )
			 {
				 // reset if already set
				 pGapList->audio_gap_active=(FALSE);
			   DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Gap Ended at %d", time) );
			 }
			}
  else
	{
    pGapList->audio_gap_active=(FALSE);
	}

}


UINT32 PlayJA2GapSample( CHAR8 *zSoundFile, UINT32 usRate, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan, AudioGapList* pData )
{
	SOUNDPARMS spParms;

	memset(&spParms, 0xff, sizeof(SOUNDPARMS));

	spParms.uiSpeed = usRate;
	spParms.uiVolume =  (UINT32) (  ( ubVolume / (FLOAT) HIGHVOLUME ) * GetSpeechVolume( ) );
	spParms.uiLoop = ubLoops;
	spParms.uiPan = uiPan;
	spParms.uiPriority=GROUP_PLAYER;

	// Setup Gap Detection, if it is not null
	if( pData != NULL )
		AudioGapListInit( zSoundFile, pData );

	return(SoundPlayStreamedFile( zSoundFile, &spParms));
}
