/*********************************************************************************
 * vtuneapi.h 11-05-97
 * Intel Corporation
 *
 * To use VTune's programatic method to start and stop sampling:
 * include this header file with your project when using the VTune pasue/resume APIs,
 * then link the vtuneapi.lib file to your module or load the vtuneapi.dll during runtime
 *
 * When running your application under VTune, if you want to start the session with the
 * sampler paused, make sure the "Start the new session
 * in Pause mode" option is enabled under View Sampling Options/Advanced tab
 *
 * The sampler is controlled by a flag. When the flag is set, sampling is stopped. When the
 * flag is reset, sampling is running. If you start VTune without enabling the "Start the
 * new session in Pause mode" option, the flag is reset and the sampler is running. If you
 * start VTune and enable the "Start the new session in Pause mode" option, the flag is set
 * and the sampler is not running.
 *
 * To set the flag, call VtPauseSampling(). To rest the flag, call
 * VtResumeSampling().
 *
 * Be aware that the flag is global. If one thread pauses sampling, any other thread
 * in the system can resume the sampler.
 *
\*********************************************************************************/

#ifndef _VTUNEAPI_H_
#define _VTUNEAPI_H_

#ifdef _VTUNE_PROFILING //Preprocessor directive that is set when using debug,
												//release with debug info, and boundschecker builds.
												//(all VTune code vanishes in release builds where this preprocessor directive isn't set)

#include <windows.h>

#if !defined(_VTUNEAPI_)
#define VTUNEAPI __declspec(dllimport)
#else
#define VTUNEAPI __declspec(dllexport)
#endif


VTUNEAPI
BOOL VtPauseSampling(void);

VTUNEAPI
BOOL VtResumeSampling(void);

#else

//Replace all calls with empty statements so release builds don't waste any time!
#define VtPauseSampling()
#define VtResumeSampling()

#endif  // _VTUNE_PROFILING

#endif  // _VTUNEAPI_H_

