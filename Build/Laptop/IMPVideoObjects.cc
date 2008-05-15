#include "IMPVideoObjects.h"
#include "Local.h"
#include "VObject.h"
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
static SGPVObject* guiBACKGROUND;
static SGPVObject* guiIMPSYMBOL;
static SGPVObject* guiBEGININDENT;
static SGPVObject* guiACTIVATIONINDENT;
static SGPVObject* guiFRONTPAGEINDENT;
static SGPVObject* guiNAMEINDENT;
static SGPVObject* guiNICKNAMEINDENT;
static SGPVObject* guiGENDERINDENT;
SGPVObject* guiANALYSE;
SGPVObject* guiATTRIBUTEGRAPH;
SGPVObject* guiSMALLSILHOUETTE;
static SGPVObject* guiLARGESILHOUETTE;
static SGPVObject* guiPORTRAITFRAME;
static SGPVObject* guiSLIDERBAR;
static SGPVObject* guiATTRIBUTEFRAME;
static SGPVObject* guiBUTTON2IMAGE;
static SGPVObject* guiBUTTON4IMAGE;
static SGPVObject* guiMAININDENT;
static SGPVObject* guiLONGINDENT;
static SGPVObject* guiSHORTINDENT;
static SGPVObject* guiSHORTHINDENT;
static SGPVObject* guiSHORT2INDENT;
static SGPVObject* guiLONGHINDENT;
static SGPVObject* guiQINDENT;
static SGPVObject* guiA1INDENT;
static SGPVObject* guiA2INDENT;
static SGPVObject* guiAVGMERCINDENT;
static SGPVObject* guiABOUTUSINDENT;
static SGPVObject* guiSHORT2HINDENT;


// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining( void );


BOOLEAN LoadProfileBackGround( void )
try
{
	// this procedure will load in the graphics for the generic background
	guiBACKGROUND = AddVideoObjectFromFile("LAPTOP/MetalBackGround.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void RemoveProfileBackGround( void )
{
  // remove background
	DeleteVideoObject(guiBACKGROUND);
}


void RenderProfileBackGround( void )
{
  INT32 iCounter = 0;

  // this procedure will render the generic backgound to the screen

	// render each row 5 times wide, 5 tiles high
	const SGPVObject* hHandle = guiBACKGROUND;
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


BOOLEAN LoadIMPSymbol( void )
try
{
	// this procedure will load the IMP main symbol into memory
	const char* const ImageFile = GetMLGFilename(MLG_IMPSYMBOL);
	guiIMPSYMBOL = AddVideoObjectFromFile(ImageFile);
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteIMPSymbol( void )
{
  // remove IMP symbol
	DeleteVideoObject(guiIMPSYMBOL);
}


void RenderIMPSymbol(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiIMPSYMBOL, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadBeginIndent( void )
try
{
	// this procedure will load the indent main symbol into memory
	guiBEGININDENT = AddVideoObjectFromFile("LAPTOP/BeginScreenIndent.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteBeginIndent( void )
{
  // remove indent symbol
	DeleteVideoObject(guiBEGININDENT);
}


void RenderBeginIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBEGININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}







BOOLEAN LoadActivationIndent( void )
try
{
	// this procedure will load the activation indent into memory
	guiACTIVATIONINDENT = AddVideoObjectFromFile("LAPTOP/ActivationIndent.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteActivationIndent( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiACTIVATIONINDENT);
}


void RenderActivationIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiACTIVATIONINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadFrontPageIndent( void )
try
{
	// this procedure will load the activation indent into memory
	guiFRONTPAGEINDENT = AddVideoObjectFromFile("LAPTOP/FrontPageIndent.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteFrontPageIndent( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiFRONTPAGEINDENT);
}


void RenderFrontPageIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiFRONTPAGEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadAnalyse( void )
try
{
	// this procedure will load the activation indent into memory
	guiANALYSE = AddVideoObjectFromFile("LAPTOP/Analyze.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteAnalyse( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiANALYSE);
}


BOOLEAN LoadAttributeGraph( void )
try
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEGRAPH = AddVideoObjectFromFile("LAPTOP/Attributegraph.sti");
	return (TRUE) ;
}
catch (...) { return FALSE; }


void DeleteAttributeGraph( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiATTRIBUTEGRAPH);
}


BOOLEAN LoadNickNameIndent( void )
try
{
	// this procedure will load the activation indent into memory
	guiNICKNAMEINDENT = AddVideoObjectFromFile("LAPTOP/NickName.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteNickNameIndent( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiNICKNAMEINDENT);
}


void RenderNickNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNICKNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadNameIndent( void )
try
{
	// this procedure will load the activation indent into memory
	guiNAMEINDENT = AddVideoObjectFromFile("LAPTOP/NameIndent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteNameIndent( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiNAMEINDENT);
}


void RenderNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadGenderIndent( void )
try
{
	// this procedure will load the activation indent into memory
	guiGENDERINDENT = AddVideoObjectFromFile("LAPTOP/GenderIndent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteGenderIndent( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiGENDERINDENT);
}


void RenderGenderIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiGENDERINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadSmallSilhouette( void )
try
{
	// this procedure will load the activation indent into memory
	guiSMALLSILHOUETTE = AddVideoObjectFromFile("LAPTOP/SmallSilhouette.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteSmallSilhouette( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSMALLSILHOUETTE);
}


BOOLEAN LoadLargeSilhouette( void )
try
{
	// this procedure will load the activation indent into memory
	guiLARGESILHOUETTE = AddVideoObjectFromFile("LAPTOP/largesilhouette.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteLargeSilhouette( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiLARGESILHOUETTE);
}


void RenderLargeSilhouette(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLARGESILHOUETTE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttributeFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEFRAME = AddVideoObjectFromFile("LAPTOP/attributeframe.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteAttributeFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiATTRIBUTEFRAME);
}


void RenderAttributeFrame(INT16 sX, INT16 sY)
{
	INT32 iCounter = 0;
	INT16 sCurrentY = 0;

	const SGPVObject* const hHandle = guiATTRIBUTEFRAME;

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


BOOLEAN LoadSliderBar( void )
try
{
	// this procedure will load the activation indent into memory
	guiSLIDERBAR = AddVideoObjectFromFile("LAPTOP/attributeslider.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteSliderBar( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSLIDERBAR);
}


void RenderSliderBar(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSLIDERBAR, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadButton2Image( void )
try
{
	// this procedure will load the activation indent into memory
	guiBUTTON2IMAGE = AddVideoObjectFromFile("LAPTOP/button_2.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteButton2Image( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiBUTTON2IMAGE);
}


void RenderButton2Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON2IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadButton4Image( void )
try
{
	// this procedure will load the activation indent into memory
	guiBUTTON4IMAGE = AddVideoObjectFromFile("LAPTOP/button_4.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteButton4Image( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiBUTTON4IMAGE);
}


void RenderButton4Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON4IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadPortraitFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiPORTRAITFRAME = AddVideoObjectFromFile("LAPTOP/Voice_PortraitFrame.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeletePortraitFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiPORTRAITFRAME);
}


void RenderPortraitFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiPORTRAITFRAME, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadMainIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiMAININDENT = AddVideoObjectFromFile("LAPTOP/mainprofilepageindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteMainIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiMAININDENT);
}


void RenderMainIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiMAININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnLongIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiLONGINDENT = AddVideoObjectFromFile("LAPTOP/longindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnLongIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiLONGINDENT);
}


void RenderQtnLongIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiSHORTINDENT = AddVideoObjectFromFile("LAPTOP/shortindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnShortIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSHORTINDENT);
}


void RenderQtnShortIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnLongIndentHighFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiLONGHINDENT = AddVideoObjectFromFile("LAPTOP/longindenthigh.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnLongIndentHighFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiLONGHINDENT);
}


void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentHighFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiSHORTHINDENT = AddVideoObjectFromFile("LAPTOP/shortindenthigh.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnShortIndentHighFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSHORTHINDENT);
}


void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiQINDENT = AddVideoObjectFromFile("LAPTOP/questionindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiQINDENT);
}


void RenderQtnIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiQINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttrib1IndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiA1INDENT = AddVideoObjectFromFile("LAPTOP/attributescreenindent_1.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteAttrib1IndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiA1INDENT);
}


void RenderAttrib1IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA1INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAttrib2IndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiA2INDENT = AddVideoObjectFromFile("LAPTOP/attributescreenindent_2.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteAttrib2IndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiA2INDENT);
}


void RenderAttrib2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAvgMercIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiAVGMERCINDENT = AddVideoObjectFromFile("LAPTOP/anaveragemercindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteAvgMercIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiAVGMERCINDENT);
}


void RenderAvgMercIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiAVGMERCINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadAboutUsIndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiABOUTUSINDENT = AddVideoObjectFromFile("LAPTOP/aboutusindent.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteAboutUsIndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiABOUTUSINDENT);
}


void RenderAboutUsIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiABOUTUSINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnShort2IndentFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiSHORT2INDENT = AddVideoObjectFromFile("LAPTOP/shortindent2.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnShort2IndentFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSHORT2INDENT);
}


void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShort2IndentHighFrame( void )
try
{
	// this procedure will load the activation indent into memory
	guiSHORT2HINDENT = AddVideoObjectFromFile("LAPTOP/shortindent2High.sti");
	return (TRUE);
}
catch (...) { return FALSE; }


void DeleteQtnShort2IndentHighFrame( void )
{
  // remove activation indent symbol
	DeleteVideoObject(guiSHORT2HINDENT);
}


void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2HINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}
