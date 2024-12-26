#ifndef __SOUNDMAN_
#define __SOUNDMAN_

#include "Types.h"

#include <vector>


#define MAXVOLUME 127


// Sound error values (they're all the same)
#define NO_SAMPLE	0xffffffff
#define SOUND_ERROR	0xffffffff


// Zeros out the structs for the system info, and initializes the cache.
void InitializeSoundManager(void);

/* Silences all currently playing sound, deallocates any memory allocated, and
 * releases the sound hardware. */
void ShutdownSoundManager(void);


UINT32 SoundPlayFromSmackBuff(const char* name, UINT8 channels, UINT8 depth, UINT32 rate, std::vector<UINT8>& buf, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data);


/* Starts a sample playing. If the sample is not loaded in the cache, it will
 * be found and loaded.
 *
 * Returns: If the sound was started, it returns a sound ID unique to that
 *          instance of the sound If an error occured, SOUND_ERROR will be
 *          returned
 */
UINT32 SoundPlay(const char* pFilename, UINT32 volume, UINT32 pan, UINT32 loop, void (*end_callback)(void*), void* data);

/* Registers a sample to be played randomly within the specified parameters.
 *
 * * Samples designated "random" are ALWAYS loaded into the cache, and locked
 * in place. They are never double-buffered, and this call will fail if they
 * cannot be loaded. *
 *
 * Returns: If successful, it returns the sample index it is loaded to, else
 *          SOUND_ERROR is returned. */
UINT32 SoundPlayRandom(const char* pFilename, UINT32 time_min, UINT32 time_max, UINT32 vol_min, UINT32 vol_max, UINT32 pan_min, UINT32 pan_max, UINT32 max_instances);

/* Can be polled in tight loops where sound buffers might starve due to heavy
 * hardware use, etc. Streams DO NOT normally need to be serviced manually, but
 * in some cases (heavy file loading) it might be desirable.
 * If you are using the end of sample callbacks, you must call this function
 * periodically to check the sample's status. */
void SoundServiceStreams(void);

/* This function should be polled by the application if random samples are
 * used. The time marks on each are checked and if it is time to spawn a new
 * instance of the sound, the number already in existance are checked, and if
 * there is room, a new one is made and the count updated.
 * If random samples are not being used, there is no purpose in polling this
 * function. */
void SoundServiceRandom(void);

// Stops all currently playing sounds.
void SoundStopAll(void);

void SoundStopAllRandom(void);

/* Stops the playing of a sound instance, if still playing.
 *
 * Returns: true if the sample was actually stopped, false if it could not be
 *          found, or was not playing. */
BOOLEAN SoundStop(UINT32 uiSoundID);

void SoundStopRandom(UINT32 uiSample);

// Returns true/false that an instance of a sound is still playing.
BOOLEAN SoundIsPlaying(UINT32 uiSoundID);

/* Sets the volume on a currently playing sound.
 *
 * Returns: true if the volume was actually set on the sample, false if the
 *          sample had already expired or couldn't be found */
BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume);

/* Sets the pan on a currently playing sound.
 *
 * Returns: true if the pan was actually set on the sample, false if the sample
 *          had already expired or couldn't be found */
BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan);

/* Returns the current volume setting of a sound that is playing. If the sound
 * has expired, or could not be found, SOUND_ERROR is returned. */
UINT32 SoundGetVolume(UINT32 uiSoundID);

/* Reports the current time position of the sample.
 * Note: You should be checking SoundIsPlaying very carefully while calling
 * this function.
 *
 * Returns: The current time of the sample in milliseconds. */
UINT32 SoundGetPosition(UINT32 uiSoundID);

// Allows or disallows the startup of the sound hardware.
void SoundEnableSound(BOOLEAN fEnable);
bool IsSoundEnabled();

#endif
