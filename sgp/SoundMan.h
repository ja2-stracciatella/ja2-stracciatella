#ifndef __SOUNDMAN_
#define __SOUNDMAN_

#include "Types.h"
#include "Mss.h"

#ifdef __cplusplus
extern "C" {
#endif

	// Sample status flags
#define		SAMPLE_ALLOCATED			0x00000001
#define		SAMPLE_LOCKED					0x00000002
#define		SAMPLE_RANDOM					0x00000004
#define		SAMPLE_RANDOM_MANUAL	0x00000008
#define		SAMPLE_3D							0x00000010


// Sound error values (they're all the same)
#define		NO_SAMPLE							0xffffffff
#define		SOUND_ERROR						0xffffffff

// Maximum allowable priority value
#define		PRIORITY_MAX					0xfffffffe
#define		PRIORITY_RANDOM				PRIORITY_MAX-1

// Structure definition for 3D sound positional information used by
// various other structs and functions
typedef struct {
				FLOAT				flX, flY, flZ;
				FLOAT				flVelX, flVelY, flVelZ;
				FLOAT				flFaceX, flFaceY, flFaceZ;
				FLOAT				flUpX, flUpY, flUpZ;
				FLOAT				flFalloffMin, flFalloffMax;
				UINT32			uiVolume;
				} SOUND3DPOS;

// Struct definition for sample slots in the cache
//		Holds the regular sample data, as well as the
//		data for the random samples

typedef struct {
				CHAR8				pName[128];						// Path to sample data
				UINT32			uiSize;								// Size of sample data
				UINT32			uiSoundSize;					// Playable sound size
				UINT32			uiFlags;							// Status flags
				UINT32			uiSpeed;							// Playback frequency
				BOOLEAN			fStereo;							// Stereo/Mono
				UINT8				ubBits;								// 8/16 bits
				PTR					pData;								// pointer to sample data memory
				PTR					pSoundStart;					// pointer to start of sound data
				UINT32			uiCacheHits;

				UINT32			uiTimeNext;						// Random sound data
				UINT32			uiTimeMin, uiTimeMax;
				UINT32			uiSpeedMin, uiSpeedMax;
				UINT32			uiVolMin, uiVolMax;
				UINT32			uiPanMin, uiPanMax;
				UINT32			uiPriority;
				UINT32			uiInstances;
				UINT32			uiMaxInstances;

				UINT32			uiAilWaveFormat;			// AIL wave sample type
				UINT32			uiADPCMBlockSize;			// Block size for compressed files

				} SAMPLETAG;

// Structure definition for slots in the sound output
//		These are used for both the cached and double-buffered
//		streams
typedef struct {
				SAMPLETAG		*pSample;
				UINT32			uiSample;
				HSAMPLE			hMSS;
				HSTREAM			hMSSStream;
				H3DSAMPLE		hM3D;
				UINT32			uiFlags;
				UINT32			uiSoundID;
				UINT32			uiPriority;
				void				(*pCallback)(UINT8*, UINT32, UINT32, UINT32, void *);
				void				*pData;
				void				(*EOSCallback)(void *);
				void				*pCallbackData;
				UINT32			uiTimeStamp;
				BOOLEAN			fLooping;
				HWFILE			hFile;
				BOOLEAN			fMusic;
				BOOLEAN			fStopAtZero;
				UINT32			uiFadeVolume;
				UINT32			uiFadeRate;
				UINT32			uiFadeTime;
				} SOUNDTAG;

// Structure definition for sound parameters being passed down to
//		the sample playing function
typedef struct {
				UINT32			uiSpeed;
				UINT32			uiPitchBend;					// Random pitch bend range +/-
				UINT32			uiVolume;
				UINT32			uiPan;
				UINT32			uiLoop;
				UINT32			uiPriority;
				void				(*EOSCallback)(void *);
				void				*pCallbackData;
				} SOUNDPARMS;

// Structure definition for 3D sound parameters being passed down to
//		the sample playing function
typedef struct {
				UINT32			uiSpeed;
				UINT32			uiPitchBend;					// Random pitch bend range +/-
				UINT32			uiVolume;							// volume at distance zero
				UINT32			uiLoop;
				UINT32			uiPriority;
				void				(*EOSCallback)(void *);
				void				*pCallbackData;

				SOUND3DPOS	Pos; // NOT optional, MUST be set
				} SOUND3DPARMS;


// Structure definition for parameters to the random sample playing
//		function
typedef struct {
				UINT32			uiTimeMin, uiTimeMax;
				UINT32			uiSpeedMin, uiSpeedMax;
				UINT32			uiVolMin, uiVolMax;
				UINT32			uiPanMin, uiPanMax;
				UINT32			uiPriority;
				UINT32			uiMaxInstances;
				} RANDOMPARMS;

// Structure definition for parameters to the random 3D sample playing
//		function
typedef struct {
				UINT32			uiTimeMin, uiTimeMax;
				UINT32			uiSpeedMin, uiSpeedMax;
				UINT32			uiVolMin, uiVolMax;
				UINT32			uiPriority;
				UINT32			uiMaxInstances;

				SOUND3DPOS	Pos; // NOT optional, MUST be set
				} RANDOM3DPARMS;


	enum e_EAXRoomTypes
	{
		EAXROOMTYPE_NONE=0,
		EAXROOMTYPE_SMALL_CAVE,
		EAXROOMTYPE_MEDIUM_CAVE,
		EAXROOMTYPE_LARGE_CAVE,
		EAXROOMTYPE_SMALL_ROOM,
		EAXROOMTYPE_MEDIUM_ROOM,
		EAXROOMTYPE_LARGE_ROOM,
		EAXROOMTYPE_OUTDOORS_FLAT,
		EAXROOMTYPE_OUTDOORS_CANYON,
		EAXROOMTYPE_UNDERWATER,

		EAXROOMTYPE_NUM_TYPES
	};

// Global startup/shutdown functions
extern BOOLEAN	InitializeSoundManager(void);
extern void			ShutdownSoundManager(void);

// Configuration functions
extern BOOLEAN	SoundSetMemoryLimit(UINT32 uiLimit);
extern BOOLEAN	SoundSetCacheThreshhold(UINT32 uiThreshold);
extern HDIGDRIVER SoundGetDriverHandle(void);

// Master volume control functions
extern BOOLEAN	SoundSetDigitalVolume(UINT32 uiVolume);
extern UINT32		SoundGetDigitalVolume(UINT32 uiVolume);
extern void			SoundSetDefaultVolume(UINT32 uiVolume);
extern UINT32		SoundGetDefaultVolume(void);


// Cache control functions
extern UINT32		SoundLoadSample(const char *pFilename);
extern UINT32		SoundFreeSample(STR pFilename);
extern UINT32		SoundLockSample(STR pFilename);
extern UINT32		SoundUnlockSample(STR pFilename);
extern BOOLEAN	SoundEmptyCache(void);
extern BOOLEAN	SoundSampleIsInUse(UINT32 uiSample);

// Play/service sample functions
extern UINT32		SoundPlay(const char *pFilename, SOUNDPARMS *pParms);
extern UINT32		SoundPlayStreamedFile(const char *pFilename, SOUNDPARMS *pParms );

extern UINT32		SoundPlayRandom(STR pFilename, RANDOMPARMS *pParms);
extern BOOLEAN	SoundRandomShouldPlay(UINT32 uiSample);
extern UINT32		SoundStartRandom(UINT32 uiSample);
extern UINT32		SoundStreamCallback(STR pFilename, SOUNDPARMS *pParms, void (*pCallback)(UINT8 *, UINT32, UINT32, UINT32, void *), void *);
extern BOOLEAN	SoundServiceStreams(void);
extern BOOLEAN	SoundServiceRandom(void);
extern void			SoundSampleSetVolumeRange(UINT32 uiSample, UINT32 uiVolMin, UINT32 uiVolMax);
extern void			SoundSampleSetPanRange(UINT32 uiSample, UINT32 uiPanMin, UINT32 uiPanMax);

// Sound instance manipulation functions
extern void			SoundSetMusic(UINT32 uiSound);
extern BOOLEAN	SoundStopMusic(void);
extern BOOLEAN	SoundStopAll(void);
extern BOOLEAN	SoundStopAllRandom(void);
extern BOOLEAN	SoundStop(UINT32 uiSoundID);
extern BOOLEAN	SoundIsPlaying(UINT32 uiSoundID);
extern BOOLEAN	SoundFileIsPlaying(CHAR8 *pFilename);
extern BOOLEAN	SoundSetFadeVolume(UINT32 uiSoundID, UINT32 uiVolume, UINT32 uiRate, BOOLEAN fStopAtZero);
extern BOOLEAN	SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume);
extern BOOLEAN	SoundSetPan(UINT32 uiSoundID, UINT32 uiPan);
extern BOOLEAN	SoundSetFrequency(UINT32 uiSoundID, UINT32 uiFreq);
extern BOOLEAN	SoundSetLoop(UINT32 uiSoundID, UINT32 uiLoop);
extern UINT32		SoundGetVolume(UINT32 uiSoundID);
extern UINT32		SoundGetPan(UINT32 uiSoundID);
extern UINT32		SoundGetFrequency(UINT32 uiSoundID);
extern UINT32		SoundGetLoop(UINT32 uiSoundID);
extern UINT32		SoundGetPosition(UINT32 uiSoundID);
extern BOOLEAN	SoundGetMilliSecondPosition(UINT32 uiSoundID, UINT32 *puiTotalMilliseconds, UINT32 *puiCurrentMilliseconds);

// Sound instance group functions
extern BOOLEAN	SoundStopGroup(UINT32 uiPriority);
extern BOOLEAN	SoundFreeGroup(UINT32 uiPriority);

extern void SoundSetSampleFlags( UINT32 uiSample, UINT32 uiFlags );
extern void SoundRemoveSampleFlags( UINT32 uiSample, UINT32 uiFlags );

extern void SoundEnableSound(BOOLEAN fEnable);


// New 3D sound priovider
extern void				Sound3DSetProvider(CHAR8 *pProviderName);
extern BOOLEAN		Sound3DInitProvider(CHAR8 *pProviderName);
extern void				Sound3DShutdownProvider(void);

// 3D sound control
extern void				Sound3DSetPosition(UINT32 uiSample, FLOAT flX, FLOAT flY, FLOAT flZ);
extern void				Sound3DSetVelocity(UINT32 uiSample, FLOAT flX, FLOAT flY, FLOAT flZ);
extern void				Sound3DSetListener(FLOAT flX, FLOAT flY, FLOAT flZ);
extern void				Sound3DSetFacing(FLOAT flXFace, FLOAT flYFace, FLOAT flZFace, FLOAT flXUp, FLOAT flYUp, FLOAT flZUp);
extern void				Sound3DSetDirection(UINT32 uiSample, FLOAT flXFace, FLOAT flYFace, FLOAT flZFace, FLOAT flXUp, FLOAT flYUp, FLOAT flZUp);
extern void				Sound3DSetFalloff(UINT32 uiSample, FLOAT flMax, FLOAT flMin);
extern void				Sound3DSetEnvironment(INT32 iEnvironment);
extern UINT32			Sound3DPlay(STR pFilename, SOUND3DPARMS *pParms);
extern UINT32			Sound3DStartSample(UINT32 uiSample, UINT32 uiChannel, SOUND3DPARMS *pParms);
extern void				Sound3DStopAll(void);
extern UINT32			Sound3DPlayRandom(STR pFilename, RANDOM3DPARMS *pParms);
extern UINT32			Sound3DStartRandom(UINT32 uiSample, SOUND3DPOS *Pos);
extern void				Sound3DSetRoomType(UINT32 uiRoomType);

// Status query functions
extern UINT32			Sound3DChannelsInUse(void);
extern UINT32			SoundStreamsInUse(void);
extern UINT32			Sound2DChannelsInUse(void);
extern UINT32			SoundTotalChannelsInUse(void);

#ifdef __cplusplus
}
#endif

#endif
