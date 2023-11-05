#include "HImage.h"
#include "Timer_Control.h"
#include "Sys_Globals.h"
#include "Fade_Screen.h"
#include "SysUtil.h"
#include "Cursor_Control.h"
#include "Music_Control.h"
#include "Render_Dirty.h"
#include "GameLoop.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include "UILayout.h"


static ScreenID guiExitScreen;
BOOLEAN gfFadeInitialized = FALSE;
INT16   gsFadeLimit;
UINT32  guiTime;
UINT32  guiFadeDelay;
BOOLEAN gfFirstTimeInFade = FALSE;
INT16   gsFadeCount;
static INT8 gbFadeType;
INT16   gsFadeRealCount;
BOOLEAN gfFadeInVideo;


FADE_FUNCTION gFadeFunction = NULL;

FADE_HOOK gFadeInDoneCallback  = NULL;
FADE_HOOK gFadeOutDoneCallback = NULL;


BOOLEAN gfFadeIn      = FALSE;
BOOLEAN gfFadeOut     = FALSE;
BOOLEAN gfFadeOutDone = FALSE;
BOOLEAN gfFadeInDone  = FALSE;


void FadeInNextFrame( )
{
	gfFadeIn = TRUE;
	gfFadeInDone = FALSE;
}

void FadeOutNextFrame( )
{
	gfFadeOut = TRUE;
	gfFadeOutDone = FALSE;
}


static void BeginFade(ScreenID uiExitScreen, INT8 bFadeValue, INT8 bType, UINT32 uiDelay);


BOOLEAN HandleBeginFadeIn(ScreenID const uiScreenExit)
{
	if ( gfFadeIn )
	{
		BeginFade( uiScreenExit, 35, FADE_IN_REALFADE, 5 );

		gfFadeIn = FALSE;

		gfFadeInDone = TRUE;

		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN HandleBeginFadeOut(ScreenID const uiScreenExit)
{
	if ( gfFadeOut )
	{
		BeginFade( uiScreenExit, 35, FADE_OUT_REALFADE, 5 );

		gfFadeOut = FALSE;

		gfFadeOutDone = TRUE;

		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN HandleFadeOutCallback( )
{
	if ( gfFadeOutDone )
	{
		gfFadeOutDone = FALSE;

		if ( gFadeOutDoneCallback != NULL )
		{
			gFadeOutDoneCallback( );

			gFadeOutDoneCallback = NULL;

			return( TRUE );
		}
	}

	return( FALSE );
}


BOOLEAN HandleFadeInCallback( )
{
	if ( gfFadeInDone )
	{
		gfFadeInDone = FALSE;

		if ( gFadeInDoneCallback != NULL )
		{
			gFadeInDoneCallback( );
		}

		gFadeInDoneCallback = NULL;

		return( TRUE );
	}

	return( FALSE );
}


static void FadeFrameBufferRealFade(void);
static void FadeInFrameBufferRealFade(void);


static void BeginFade(ScreenID const uiExitScreen, INT8 const bFadeValue, INT8 const bType, UINT32 const uiDelay)
{
	//Init some paramters
	guiExitScreen	= uiExitScreen;
	guiFadeDelay			= uiDelay;
	gfFadeIn = FALSE;
	gfFadeInVideo = TRUE;


	// Calculate step;
	switch (bType)
	{
		case FADE_IN_REALFADE:
			gsFadeRealCount = -1;
			gsFadeLimit			= 8;
			gFadeFunction = FadeInFrameBufferRealFade;
			gfFadeInVideo   = FALSE;

			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			FRAME_BUFFER->Fill(0x000000FF);
			break;

		case FADE_OUT_REALFADE:
			gsFadeRealCount = -1;
			gsFadeLimit			= 10;
			gFadeFunction = FadeFrameBufferRealFade;
			gfFadeInVideo   = FALSE;
			break;
	}

	gfFadeInitialized = TRUE;
	gfFirstTimeInFade = TRUE;
	gsFadeCount				= 0;
	gbFadeType						= bType;

	SetPendingNewScreen(FADE_SCREEN);
}


ScreenID FadeScreenHandle()
{
	UINT32 uiTime;

	if ( !gfFadeInitialized )
	{
		SET_ERROR( "Fade Screen called but not intialized " );
		return( ERROR_SCREEN );
	}

	// ATE: Remove cursor
	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );


	if ( gfFirstTimeInFade )
	{
		gfFirstTimeInFade = FALSE;

		// Calcuate delay
		guiTime = GetJA2Clock( );
	}

	// Get time
	uiTime = GetJA2Clock( );

	MusicPoll();

	if ( ( uiTime - guiTime ) > guiFadeDelay )
	{
		// Fade!
		if ( !gfFadeIn )
		{
			//gFadeFunction( );
		}

		InvalidateScreen();

		if ( !gfFadeInVideo )
		{
			gFadeFunction( );
		}

		gsFadeCount++;

		if ( gsFadeCount > gsFadeLimit )
		{
			switch( gbFadeType )
			{
				case FADE_OUT_REALFADE:
					FRAME_BUFFER->Fill(0x000000FF);
					break;
			}

			//End!
			gfFadeInitialized = FALSE;
			gfFadeIn = FALSE;

			return( guiExitScreen );
		}
	}

	return( FADE_SCREEN );
}


static void FadeFrameBufferRealFade(void)
{
	if ( gsFadeRealCount != gsFadeCount )
	{
		FRAME_BUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		gsFadeRealCount = gsFadeCount;
	}

}


static void FadeInFrameBufferRealFade(void)
{
	INT32 cnt;

	if ( gsFadeRealCount != gsFadeCount )
	{

		for ( cnt = 0; cnt < ( gsFadeLimit - gsFadeCount ); cnt++ )
		{
			FRAME_BUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}

		RefreshScreen();

		// Copy save buffer back
		RestoreExternBackgroundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		gsFadeRealCount = gsFadeCount;
	}

}
