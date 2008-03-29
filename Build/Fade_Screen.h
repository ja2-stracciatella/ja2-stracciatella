#ifndef FADE_SCREEN_H
#define FADE_SCREEN_H

#define	FADE_OUT_REALFADE					5

#define FADE_IN_REALFADE					12

typedef void (*FADE_HOOK)( void );

extern FADE_HOOK		gFadeInDoneCallback;
extern FADE_HOOK		gFadeOutDoneCallback;


typedef void (*FADE_FUNCTION)( void );


BOOLEAN						gfFadeInitialized;
BOOLEAN						gfFadeIn;
FADE_FUNCTION			gFadeFunction;
BOOLEAN						gfFadeInVideo;

BOOLEAN HandleBeginFadeIn( UINT32 uiScreenExit );
BOOLEAN HandleBeginFadeOut( UINT32 uiScreenExit );

BOOLEAN HandleFadeOutCallback(void);
BOOLEAN HandleFadeInCallback(void);

void FadeInNextFrame(void);
void FadeOutNextFrame(void);

#endif
