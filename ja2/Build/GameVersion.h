#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_


#ifdef __cplusplus
extern "C" {
#endif


//
//	Keeps track of the game version
//

extern	INT16		zVersionLabel[16];
extern	INT8		czVersionNumber[16];
extern	INT16		zTrackingNumber[16];



//#define RUSSIANGOLD

//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file
//

const extern	UINT32	guiSavedGameVersion;

#ifdef __cplusplus
}
#endif


#endif
