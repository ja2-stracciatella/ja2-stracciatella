#ifndef _BUILDDEFINES_H
#define _BUILDDEFINES_H


#if defined DUTCH + defined ENGLISH + defined FRENCH + defined GERMAN + defined ITALIAN + defined POLISH + defined RUSSIAN + defined RUSSIAN_GOLD != 1
#	error Excactly one of DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN or RUSSIAN_GOLD must be defined.
#endif

// Beta version
// #define	JA2BETAVERSION

// Normal test version
// #define JA2TESTVERSION

// If we want to include the editor
// #define JA2EDITOR

#ifdef _DEBUG
	#ifndef JA2TESTVERSION
		#define JA2TESTVERSION
	#endif
#endif

// Do combinations
#ifdef JA2TESTVERSION
	#define JA2BETAVERSION
	//#define JA2EDITOR
#endif

#ifdef JA2BETAVERSION
	#define SGP_DEBUG
	#define	FORCE_ASSERTS_ON
	#define SGP_VIDEO_DEBUGGING
#endif

// Huge speed and memory hog, but thorough -- will work with release builds.
// #define EXTREME_MEMORY_DEBUGGING

#endif
