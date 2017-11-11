#ifndef FADE_SCREEN_H
#define FADE_SCREEN_H

#include "ScreenIDs.h"
#include "Types.h"

#define FADE_OUT_REALFADE	5

#define FADE_IN_REALFADE	12

typedef void (*FADE_HOOK)( void );

extern FADE_HOOK gFadeInDoneCallback;
extern FADE_HOOK gFadeOutDoneCallback;


typedef void (*FADE_FUNCTION)( void );


extern BOOLEAN       gfFadeInitialized;
extern BOOLEAN       gfFadeIn;
extern FADE_FUNCTION gFadeFunction;
extern BOOLEAN       gfFadeInVideo;

BOOLEAN HandleBeginFadeIn(ScreenID uiScreenExit);
BOOLEAN HandleBeginFadeOut(ScreenID uiScreenExit);

BOOLEAN HandleFadeOutCallback(void);
BOOLEAN HandleFadeInCallback(void);

void FadeInNextFrame(void);
void FadeOutNextFrame(void);

ScreenID FadeScreenHandle(void);

#endif
