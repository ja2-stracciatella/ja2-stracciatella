#ifndef _CINEMATICS_H_
#define _CINEMATICS_H_

#include "smack.h"

typedef struct {

		CHAR8										*cFilename;
//		HFILE										hFileHandle;
		HWFILE									hFileHandle;
		Smack										*SmackHandle;
		SmackBuf								*SmackBuffer;
		UINT32									uiFlags;
		LPDIRECTDRAWSURFACE2		lpDDS;
		HWND										hWindow;
		UINT32									uiFrame;
		UINT32									uiLeft, uiTop;
//		LPDIRECTDRAW2						lpDD;
//		UINT32									uiNumFrames;
//		UINT8										*pAudioData;
//		UINT8										*pCueData;
							}	SMKFLIC;

void				SmkInitialize(HWND hWindow, UINT32 uiWidth, UINT32 uiHeight);
void				SmkShutdown(void);
SMKFLIC			*SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fAutoClose);
BOOLEAN			SmkPollFlics(void);
SMKFLIC			*SmkOpenFlic(CHAR8 *cFilename);
void				SmkSetBlitPosition(SMKFLIC *pSmack, UINT32 uiLeft, UINT32 uiTop);
void				SmkCloseFlic(SMKFLIC *pSmack);
SMKFLIC			*SmkGetFreeFlic(void);

/*
//--------------------------------------------------------------------------
// Prototypes etc. for our functions that make use of the Smacker library.
//
// Written by Derek Beland, Jan 11, 1995

#define FLICSOUNDID		"BLAH"	// ID for smack flic w/ sound :)

typedef struct {
	unsigned long offset;
	unsigned long length;
} SMPLARRAY;

extern SmackBuf *sbuf;
extern Smack *smk;
extern int smktag;
extern HANDLE smkhandle;
extern int SmackFlicIsOpened;
extern int SmackFlicIsPlaying;


HANDLE OpenSmackFlic(char *fname,Smack **s,u32);
int GetNextCue(void);
void PlayCueSamples(void);
void InitFlicSamples(HANDLE fhandle);
void FreeFlicSamples(void);

int SmackPlayFlic(char *,u32);
void SmackShowNextFrame(void);
void CloseSmackFlic(void);
void InitPal(HWND wh);
*/
#endif
