#ifndef __SOUNDMAN_
#define __SOUNDMAN_

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Sample status flags
#define		SAMPLE_ALLOCATED			0x00000001
#define		SAMPLE_LOCKED					0x00000002
#define		SAMPLE_RANDOM					0x00000004


// Sound error values (they're all the same)
#define		NO_SAMPLE							0xffffffff
#define		SOUND_ERROR						0xffffffff

// Maximum allowable priority value
#define		PRIORITY_MAX					0xfffffffe
#define		PRIORITY_RANDOM				PRIORITY_MAX-1


// Structure definition for sound parameters being passed down to
//		the sample playing function
typedef struct {
				UINT32			uiVolume;
				UINT32			uiPan;
				UINT32			uiLoop;
				UINT32			uiPriority;
				void				(*EOSCallback)(void *);
				void				*pCallbackData;
				} SOUNDPARMS;


// Structure definition for parameters to the random sample playing
//		function
typedef struct {
				UINT32			uiTimeMin, uiTimeMax;
				UINT32			uiVolMin, uiVolMax;
				UINT32			uiPanMin, uiPanMax;
				UINT32			uiPriority;
				UINT32			uiMaxInstances;
				} RANDOMPARMS;


// Global startup/shutdown functions
extern BOOLEAN	InitializeSoundManager(void);
extern void			ShutdownSoundManager(void);

// Cache control functions
extern UINT32		SoundLoadSample(const char *pFilename);

// Play/service sample functions
extern UINT32		SoundPlay(const char *pFilename, SOUNDPARMS *pParms);
extern UINT32		SoundPlayStreamedFile(const char *pFilename, SOUNDPARMS *pParms );

extern UINT32		SoundPlayRandom(STR pFilename, RANDOMPARMS *pParms);
extern BOOLEAN	SoundServiceStreams(void);

// Sound instance manipulation functions
extern BOOLEAN	SoundStopAll(void);
extern BOOLEAN	SoundStopAllRandom(void);
extern BOOLEAN	SoundStop(UINT32 uiSoundID);
extern BOOLEAN	SoundIsPlaying(UINT32 uiSoundID);
extern BOOLEAN	SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume);
extern BOOLEAN	SoundSetPan(UINT32 uiSoundID, UINT32 uiPan);
extern UINT32		SoundGetVolume(UINT32 uiSoundID);
extern UINT32		SoundGetPosition(UINT32 uiSoundID);

extern void SoundRemoveSampleFlags( UINT32 uiSample, UINT32 uiFlags );

extern void SoundEnableSound(BOOLEAN fEnable);

#ifdef __cplusplus
}
#endif

#endif
