#include "Directories.h"
#include "IMPVideoObjects.h"
#include "VObject.h"
#include "Laptop.h"
#include "GameRes.h"
#include "IMP_Attribute_Selection.h"
#include "Button_System.h"
#include "Object_Cache.h"
#include "Video.h"
#include "VSurface.h"
#include "UILayout.h"

// video object handles
SGPVObject* guiANALYSE;
SGPVObject* guiATTRIBUTEGRAPH;
SGPVObject* guiSMALLSILHOUETTE;

namespace {
cache_key_t const guiBACKGROUND{ LAPTOPDIR "/metalbackground.sti" };
cache_key_t const guiBEGININDENT{ LAPTOPDIR "/beginscreenindent.sti" };
cache_key_t const guiACTIVATIONINDENT{ LAPTOPDIR "/activationindent.sti" };
cache_key_t const guiFRONTPAGEINDENT{ LAPTOPDIR "/frontpageindent.sti" };
cache_key_t const guiNAMEINDENT{ LAPTOPDIR "/nameindent.sti" };
cache_key_t const guiNICKNAMEINDENT{ LAPTOPDIR "/nickname.sti" };
cache_key_t const guiGENDERINDENT{ LAPTOPDIR "/genderindent.sti" };
cache_key_t const guiLARGESILHOUETTE{ LAPTOPDIR "/largesilhouette.sti" };
cache_key_t const guiPORTRAITFRAME{ LAPTOPDIR "/voice_portraitframe.sti" };
cache_key_t const guiSLIDERBAR{ LAPTOPDIR "/attributeslider.sti" };
cache_key_t const guiATTRIBUTEFRAME{ LAPTOPDIR "/attributeframe.sti" };
cache_key_t const guiBUTTON2IMAGE{ LAPTOPDIR "/button_2.sti" };
cache_key_t const guiBUTTON4IMAGE{ LAPTOPDIR "/button_4.sti" };
cache_key_t const guiMAININDENT{ LAPTOPDIR "/mainprofilepageindent.sti" };
cache_key_t const guiLONGINDENT{ LAPTOPDIR "/longindent.sti" };
cache_key_t const guiLONGHINDENT{ LAPTOPDIR "/longindenthigh.sti" };
cache_key_t const guiSHORTINDENT{ LAPTOPDIR "/shortindent.sti" };
cache_key_t const guiSHORTHINDENT{ LAPTOPDIR "/shortindenthigh.sti" };
cache_key_t const guiSHORT2INDENT{ LAPTOPDIR "/shortindent2.sti"};
cache_key_t const guiSHORT2HINDENT{ LAPTOPDIR "/shortindent2high.sti" };
cache_key_t const guiQINDENT{ LAPTOPDIR "/questionindent.sti" };
cache_key_t const guiA1INDENT{ LAPTOPDIR "/attributescreenindent_1.sti" };
cache_key_t const guiA2INDENT{ LAPTOPDIR "/attributescreenindent_2.sti" };
cache_key_t const guiAVGMERCINDENT{ LAPTOPDIR "/anaveragemercindent.sti" };
cache_key_t const guiABOUTUSINDENT{ LAPTOPDIR "/aboutusindent.sti" };
}


// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining( void );


void RemoveProfileBackGround( void )
{
	// remove background
	RemoveVObject(guiBACKGROUND);
}


void RenderProfileBackGround( void )
{
	INT32 iCounter = 0;

	// this procedure will render the generic backgound to the screen

	// render each row 5 times wide, 5 tiles high
	auto * const hHandle = guiBACKGROUND;
	for(iCounter = 0; iCounter < 4; iCounter++)
	{
		// blt background to screen from left to right
		const INT32 x = LAPTOP_SCREEN_UL_X;
		const INT32 y = LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT;
		BltVideoObject(FRAME_BUFFER, hHandle, 0, x + 0 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, y);
		BltVideoObject(FRAME_BUFFER, hHandle, 0, x + 1 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, y);
		BltVideoObject(FRAME_BUFFER, hHandle, 0, x + 2 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, y);
		BltVideoObject(FRAME_BUFFER, hHandle, 0, x + 3 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, y);
 	}

	// dirty buttons
	MarkButtonsDirty( );

	// force refresh of screen
	InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
}


void DeleteIMPSymbol( void )
{
	// remove IMP symbol
	RemoveVObject(MLG_IMPSYMBOL);
}


void RenderIMPSymbol(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, MLG_IMPSYMBOL, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteBeginIndent( void )
{
	// remove indent symbol
	RemoveVObject(guiBEGININDENT);
}


void RenderBeginIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBEGININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteActivationIndent( void )
{
	// remove activation indent symbol
	RemoveVObject(guiACTIVATIONINDENT);
}


void RenderActivationIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiACTIVATIONINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteFrontPageIndent( void )
{
	// remove activation indent symbol
	RemoveVObject(guiFRONTPAGEINDENT);
}


void RenderFrontPageIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiFRONTPAGEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAnalyse(void)
{
	// this procedure will load the activation indent into memory
	guiANALYSE = AddVideoObjectFromFile(LAPTOPDIR "/analyze.sti");
}


void DeleteAnalyse( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiANALYSE);
}


void LoadAttributeGraph(void)
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEGRAPH = AddVideoObjectFromFile(LAPTOPDIR "/attributegraph.sti");
}


void DeleteAttributeGraph( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiATTRIBUTEGRAPH);
}


void DeleteNickNameIndent( void )
{
	// remove activation indent symbol
	RemoveVObject(guiNICKNAMEINDENT);
}


void RenderNickNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNICKNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteNameIndent( void )
{
	// remove activation indent symbol
	RemoveVObject(guiNAMEINDENT);
}


void RenderNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteGenderIndent( void )
{
	// remove activation indent symbol
	RemoveVObject(guiGENDERINDENT);
}


void RenderGenderIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiGENDERINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadSmallSilhouette(void)
{
	// this procedure will load the activation indent into memory
	guiSMALLSILHOUETTE = AddVideoObjectFromFile(LAPTOPDIR "/smallsilhouette.sti");
}


void DeleteSmallSilhouette( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSMALLSILHOUETTE);
}


void DeleteLargeSilhouette( void )
{
	// remove activation indent symbol
	RemoveVObject(guiLARGESILHOUETTE);
}


void RenderLargeSilhouette(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLARGESILHOUETTE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteAttributeFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiATTRIBUTEFRAME);
}


void RenderAttributeFrame(INT16 sX, INT16 sY)
{
	INT32 iCounter = 0;
	INT16 sCurrentY = 0;

	auto * const hHandle = guiATTRIBUTEFRAME;

	// blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

	sCurrentY += 10;
	for( iCounter = 0; iCounter < 10; iCounter++ )
	{
		// blt to sX, sY relative to upper left corner
		BltVideoObject(FRAME_BUFFER, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);
		BltVideoObject(FRAME_BUFFER, hHandle, 1, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);
		BltVideoObject(FRAME_BUFFER, hHandle, 3, LAPTOP_SCREEN_UL_X + sX + 368, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

		sCurrentY += 20;
	}

	BltVideoObject(FRAME_BUFFER, hHandle, 4, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);
}

void RenderAttributeFrameForIndex( INT16 sX, INT16 sY, INT32 iIndex )
{
	INT16 sCurrentY = 0;

	// valid index?
	if( iIndex == -1 )
	{
		return;
	}

	sCurrentY = ( INT16 )( 10 + ( iIndex * 20 ) );

	BltVideoObject(FRAME_BUFFER, guiATTRIBUTEFRAME, 2, LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

	RenderAttrib2IndentFrame(350, 42 );

	// amt of bonus pts
	DrawBonusPointsRemaining( );

	// render attribute boxes
	RenderAttributeBoxes( );

	InvalidateRegion( LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, LAPTOP_SCREEN_UL_X + sX + 400, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY + 21 );
}


void DeleteSliderBar( void )
{
	// remove activation indent symbol
	RemoveVObject(guiSLIDERBAR);
}


void RenderSliderBar(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSLIDERBAR, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteButton2Image( void )
{
	// remove activation indent symbol
	RemoveVObject(guiBUTTON2IMAGE);
}


void RenderButton2Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON2IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteButton4Image( void )
{
	// remove activation indent symbol
	RemoveVObject(guiBUTTON4IMAGE);
}


void RenderButton4Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON4IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeletePortraitFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiPORTRAITFRAME);
}


void RenderPortraitFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiPORTRAITFRAME, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteMainIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiMAININDENT);
}


void RenderMainIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiMAININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnLongIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiLONGINDENT);
}


void RenderQtnLongIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnShortIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiSHORTINDENT);
}


void RenderQtnShortIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnLongIndentHighFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiLONGHINDENT);
}


void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnShortIndentHighFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiSHORTHINDENT);
}


void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiQINDENT);
}


void RenderQtnIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiQINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteAttrib1IndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiA1INDENT);
}


void RenderAttrib1IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA1INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteAttrib2IndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiA2INDENT);
}


void RenderAttrib2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteAvgMercIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiAVGMERCINDENT);
}


void RenderAvgMercIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiAVGMERCINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteAboutUsIndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiABOUTUSINDENT);
}


void RenderAboutUsIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiABOUTUSINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnShort2IndentFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiSHORT2INDENT);
}


void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void DeleteQtnShort2IndentHighFrame( void )
{
	// remove activation indent symbol
	RemoveVObject(guiSHORT2HINDENT);
}


void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2HINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}
