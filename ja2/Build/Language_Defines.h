#ifndef __LANGUAGE_DEFINES_H
#define __LANGUAGE_DEFINES_H

#if defined DUTCH + defined ENGLISH + defined FRENCH + defined GERMAN + defined ITALIAN + defined POLISH + defined RUSSIAN + defined RUSSIAN_GOLD != 1
#	error Excactly one of DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN or RUSSIAN_GOLD must be defined.
#endif

#if defined RUSSIAN_GOLD
#	define RUSSIAN
#endif

#define BLOOD_N_GORE_ENABLED

#endif
