#include "IMPVideoObjects.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Laptop.h"
#include "Multi_Language_Graphic_Utils.h"
#include "IMP_Attribute_Selection.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"


// video object handles
static UINT32 guiBACKGROUND;
static UINT32 guiIMPSYMBOL;
static UINT32 guiBEGININDENT;
static UINT32 guiACTIVATIONINDENT;
static UINT32 guiFRONTPAGEINDENT;
static UINT32 guiNAMEINDENT;
static UINT32 guiNICKNAMEINDENT;
static UINT32 guiGENDERINDENT;
UINT32 guiANALYSE;
UINT32 guiATTRIBUTEGRAPH;
UINT32 guiSMALLSILHOUETTE;
static UINT32 guiLARGESILHOUETTE;
static UINT32 guiPORTRAITFRAME;
static UINT32 guiSLIDERBAR;
static UINT32 guiATTRIBUTEFRAME;
static UINT32 guiBUTTON2IMAGE;
static UINT32 guiBUTTON4IMAGE;
static UINT32 guiMAININDENT;
static UINT32 guiLONGINDENT;
static UINT32 guiSHORTINDENT;
static UINT32 guiSHORTHINDENT;
static UINT32 guiSHORT2INDENT;
static UINT32 guiLONGHINDENT;
static UINT32 guiQINDENT;
static UINT32 guiA1INDENT;
static UINT32 guiA2INDENT;
static UINT32 guiAVGMERCINDENT;
static UINT32 guiABOUTUSINDENT;
static UINT32 guiSHORT2HINDENT;


// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining( void );


BOOLEAN LoadProfileBackGround( void )
{
	// this procedure will load in the graphics for the generic background
	guiBACKGROUND = AddVideoObjectFromFile("LAPTOP/MetalBackGround.sti");
	CHECKF(guiBACKGROUND != NO_VOBJECT);
	return (TRUE) ;
}

void RemoveProfileBackGround( void )
{

  // remove background
	DeleteVideoObjectFromIndex( guiBACKGROUND );
}


void RenderProfileBackGround( void )
{
  INT32 iCounter = 0;

  // this procedure will render the generic backgound to the screen

	// render each row 5 times wide, 5 tiles high
	HVOBJECT hHandle = GetVideoObject(guiBACKGROUND);
  for(iCounter = 0; iCounter < 4; iCounter++)
	{

	  // blt background to screen from left to right
	  BltVideoObject(FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X + 0 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X + 1 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X + 2 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X + 3 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH, LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
 	}

	// dirty buttons
	MarkButtonsDirty( );

	// force refresh of screen
  InvalidateRegion( LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, 640, 480 );
}

BOOLEAN LoadIMPSymbol( void )
{

	// this procedure will load the IMP main symbol into memory
	SGPFILENAME ImageFile;
	GetMLGFilename(ImageFile, MLG_IMPSYMBOL);
	guiIMPSYMBOL = AddVideoObjectFromFile(ImageFile);
	CHECKF(guiIMPSYMBOL != NO_VOBJECT);

	return (TRUE) ;
}


void DeleteIMPSymbol( void )
{

  // remove IMP symbol
	DeleteVideoObjectFromIndex( guiIMPSYMBOL );
}

void RenderIMPSymbol(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiIMPSYMBOL, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadBeginIndent( void )
{
	// this procedure will load the indent main symbol into memory
	guiBEGININDENT = AddVideoObjectFromFile("LAPTOP/BeginScreenIndent.sti");
	CHECKF(guiBEGININDENT != NO_VOBJECT);
	return (TRUE) ;
}


void DeleteBeginIndent( void )
{

  // remove indent symbol

	DeleteVideoObjectFromIndex( guiBEGININDENT );
}

void RenderBeginIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiBEGININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}







BOOLEAN LoadActivationIndent( void )
{
	// this procedure will load the activation indent into memory
	guiACTIVATIONINDENT = AddVideoObjectFromFile("LAPTOP/ActivationIndent.sti");
	CHECKF(guiACTIVATIONINDENT != NO_VOBJECT);
	return (TRUE) ;
}


void DeleteActivationIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiACTIVATIONINDENT );
}

void RenderActivationIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiACTIVATIONINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadFrontPageIndent( void )
{
	// this procedure will load the activation indent into memory
	guiFRONTPAGEINDENT = AddVideoObjectFromFile("LAPTOP/FrontPageIndent.sti");
	CHECKF(guiFRONTPAGEINDENT != NO_VOBJECT);
	return (TRUE) ;
}


void DeleteFrontPageIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiFRONTPAGEINDENT );
}

void RenderFrontPageIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiFRONTPAGEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadAnalyse( void )
{
	// this procedure will load the activation indent into memory
	guiANALYSE = AddVideoObjectFromFile("LAPTOP/Analyze.sti");
	CHECKF(guiANALYSE != NO_VOBJECT);
	return (TRUE) ;
}


void DeleteAnalyse( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiANALYSE );
}


BOOLEAN LoadAttributeGraph( void )
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEGRAPH = AddVideoObjectFromFile("LAPTOP/Attributegraph.sti");
	CHECKF(guiATTRIBUTEGRAPH != NO_VOBJECT);
	return (TRUE) ;
}


void DeleteAttributeGraph( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiATTRIBUTEGRAPH );
}


BOOLEAN LoadNickNameIndent( void )
{
	// this procedure will load the activation indent into memory
	guiNICKNAMEINDENT = AddVideoObjectFromFile("LAPTOP/NickName.sti");
	CHECKF(guiNICKNAMEINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteNickNameIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiNICKNAMEINDENT );
}

void RenderNickNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiNICKNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadNameIndent( void )
{
	// this procedure will load the activation indent into memory
	guiNAMEINDENT = AddVideoObjectFromFile("LAPTOP/NameIndent.sti");
	CHECKF(guiNAMEINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteNameIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiNAMEINDENT );
}

void RenderNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadGenderIndent( void )
{
	// this procedure will load the activation indent into memory
	guiGENDERINDENT = AddVideoObjectFromFile("LAPTOP/GenderIndent.sti");
	CHECKF(guiGENDERINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteGenderIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiGENDERINDENT );
}

void RenderGenderIndent(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiGENDERINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadSmallSilhouette( void )
{
	// this procedure will load the activation indent into memory
	guiSMALLSILHOUETTE = AddVideoObjectFromFile("LAPTOP/SmallSilhouette.sti");
	CHECKF(guiSMALLSILHOUETTE != NO_VOBJECT);
	return (TRUE);
}


void DeleteSmallSilhouette( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSMALLSILHOUETTE );
}


BOOLEAN LoadLargeSilhouette( void )
{
	// this procedure will load the activation indent into memory
	guiLARGESILHOUETTE = AddVideoObjectFromFile("LAPTOP/largesilhouette.sti");
	CHECKF(guiLARGESILHOUETTE != NO_VOBJECT);
	return (TRUE);
}


void DeleteLargeSilhouette( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLARGESILHOUETTE );
}

void RenderLargeSilhouette(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLARGESILHOUETTE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttributeFrame( void )
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEFRAME = AddVideoObjectFromFile("LAPTOP/attributeframe.sti");
	CHECKF(guiATTRIBUTEFRAME != NO_VOBJECT);
	return (TRUE);
}


void DeleteAttributeFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiATTRIBUTEFRAME );
}

void RenderAttributeFrame(INT16 sX, INT16 sY)
{
	INT32 iCounter = 0;
	INT16 sCurrentY = 0;

	HVOBJECT hHandle = GetVideoObject(guiATTRIBUTEFRAME);

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

	BltVideoObjectFromIndex(FRAME_BUFFER, guiATTRIBUTEFRAME, 2, LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

	RenderAttrib2IndentFrame(350, 42 );

	// amt of bonus pts
	DrawBonusPointsRemaining( );

	// render attribute boxes
	RenderAttributeBoxes( );

	InvalidateRegion( LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, LAPTOP_SCREEN_UL_X + sX + 400, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY + 21 );
}


BOOLEAN LoadSliderBar( void )
{
	// this procedure will load the activation indent into memory
	guiSLIDERBAR = AddVideoObjectFromFile("LAPTOP/attributeslider.sti");
	CHECKF(guiSLIDERBAR != NO_VOBJECT);
	return (TRUE);
}


void DeleteSliderBar( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSLIDERBAR );
}

void RenderSliderBar(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSLIDERBAR, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadButton2Image( void )
{
	// this procedure will load the activation indent into memory
	guiBUTTON2IMAGE = AddVideoObjectFromFile("LAPTOP/button_2.sti");
	CHECKF(guiBUTTON2IMAGE != NO_VOBJECT);
	return (TRUE);
}


void DeleteButton2Image( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiBUTTON2IMAGE );
}

void RenderButton2Image(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiBUTTON2IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadButton4Image( void )
{
	// this procedure will load the activation indent into memory
	guiBUTTON4IMAGE = AddVideoObjectFromFile("LAPTOP/button_4.sti");
	CHECKF(guiBUTTON4IMAGE != NO_VOBJECT);
	return (TRUE);
}


void DeleteButton4Image( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiBUTTON4IMAGE );
}

void RenderButton4Image(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiBUTTON4IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadPortraitFrame( void )
{
	// this procedure will load the activation indent into memory
	guiPORTRAITFRAME = AddVideoObjectFromFile("LAPTOP/Voice_PortraitFrame.sti");
	CHECKF(guiPORTRAITFRAME != NO_VOBJECT);
	return (TRUE);
}


void DeletePortraitFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiPORTRAITFRAME );
}

void RenderPortraitFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiPORTRAITFRAME, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadMainIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiMAININDENT = AddVideoObjectFromFile("LAPTOP/mainprofilepageindent.sti");
	CHECKF(guiMAININDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteMainIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiMAININDENT );
}

void RenderMainIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiMAININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnLongIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiLONGINDENT = AddVideoObjectFromFile("LAPTOP/longindent.sti");
	CHECKF(guiLONGINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnLongIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLONGINDENT );
}

void RenderQtnLongIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLONGINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiSHORTINDENT = AddVideoObjectFromFile("LAPTOP/shortindent.sti");
	CHECKF(guiSHORTINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnShortIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORTINDENT );
}

void RenderQtnShortIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSHORTINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnLongIndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	guiLONGHINDENT = AddVideoObjectFromFile("LAPTOP/longindenthigh.sti");
	CHECKF(guiLONGHINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnLongIndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLONGHINDENT );
}

void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLONGHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	guiSHORTHINDENT = AddVideoObjectFromFile("LAPTOP/shortindenthigh.sti");
	CHECKF(guiSHORTHINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnShortIndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORTHINDENT );
}

void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSHORTHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiQINDENT = AddVideoObjectFromFile("LAPTOP/questionindent.sti");
	CHECKF(guiQINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiQINDENT );
}

void RenderQtnIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiQINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttrib1IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiA1INDENT = AddVideoObjectFromFile("LAPTOP/attributescreenindent_1.sti");
	CHECKF(guiA1INDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteAttrib1IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiA1INDENT );
}

void RenderAttrib1IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiA1INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAttrib2IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiA2INDENT = AddVideoObjectFromFile("LAPTOP/attributescreenindent_2.sti");
	CHECKF(guiA2INDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteAttrib2IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiA2INDENT );
}

void RenderAttrib2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiA2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAvgMercIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiAVGMERCINDENT = AddVideoObjectFromFile("LAPTOP/anaveragemercindent.sti");
	CHECKF(guiAVGMERCINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteAvgMercIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiAVGMERCINDENT );
}

void RenderAvgMercIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiAVGMERCINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadAboutUsIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiABOUTUSINDENT = AddVideoObjectFromFile("LAPTOP/aboutusindent.sti");
	CHECKF(guiABOUTUSINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteAboutUsIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiABOUTUSINDENT );
}

void RenderAboutUsIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiABOUTUSINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnShort2IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	guiSHORT2INDENT = AddVideoObjectFromFile("LAPTOP/shortindent2.sti");
	CHECKF(guiSHORT2INDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnShort2IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORT2INDENT );
}

void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSHORT2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShort2IndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	guiSHORT2HINDENT = AddVideoObjectFromFile("LAPTOP/shortindent2High.sti");
	CHECKF(guiSHORT2HINDENT != NO_VOBJECT);
	return (TRUE);
}


void DeleteQtnShort2IndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORT2HINDENT );
}

void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSHORT2HINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}
