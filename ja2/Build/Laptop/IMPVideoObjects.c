#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "IMPVideoObjects.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render_Dirty.h"
	#include "Encrypted_File.h"
	#include "Cursors.h"
	#include "IMP_HomePage.h"
	#include "Laptop.h"
	#include "Multi_Language_Graphic_Utils.h"
	#include "IMP_Attribute_Selection.h"
	#include "Button_System.h"
	#include "Video.h"
	#include "VSurface.h"
#endif

// globals


// video object handles
UINT32 guiBACKGROUND;
UINT32 guiIMPSYMBOL;
UINT32 guiBEGININDENT;
UINT32 guiACTIVATIONINDENT;
UINT32 guiFRONTPAGEINDENT;
UINT32 guiFULLNAMEINDENT;
UINT32 guiNAMEINDENT;
UINT32 guiNICKNAMEINDENT;
UINT32 guiGENDERINDENT;
UINT32 guiSMALLFRAME;
UINT32 guiANALYSE;
UINT32 guiATTRIBUTEGRAPH;
UINT32 guiATTRIBUTEGRAPHBAR;
UINT32 guiSMALLSILHOUETTE;
UINT32 guiLARGESILHOUETTE;
UINT32 guiPORTRAITFRAME;
UINT32 guiSLIDERBAR;
UINT32 guiATTRIBUTEFRAME;
UINT32 guiATTRIBUTESCREENINDENT1;
UINT32 guiATTRIBUTESCREENINDENT2;
UINT32 guiATTRIBUTEBAR;
UINT32 guiBUTTON2IMAGE;
UINT32 guiBUTTON1IMAGE;
UINT32 guiBUTTON4IMAGE;
UINT32 guiPORTRAITFRAME;
UINT32 guiMAININDENT;
UINT32 guiLONGINDENT;
UINT32 guiSHORTINDENT;
UINT32 guiSHORTHINDENT;
UINT32 guiSHORT2INDENT;
UINT32 guiLONGHINDENT;
UINT32 guiQINDENT;
UINT32 guiA1INDENT;
UINT32 guiA2INDENT;
UINT32 guiAVGMERCINDENT;
UINT32 guiABOUTUSINDENT;
UINT32 guiSHORT2HINDENT;


// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining( void );


BOOLEAN LoadProfileBackGround( void )
{
	// this procedure will load in the graphics for the generic background
	CHECKF(AddVideoObjectFromFile("LAPTOP\\MetalBackGround.sti", &guiBACKGROUND));
	return (TRUE) ;
}

void RemoveProfileBackGround( void )
{

  // remove background
	DeleteVideoObjectFromIndex( guiBACKGROUND );
}


void RenderProfileBackGround( void )
{

	HVOBJECT hHandle;
  INT32 iCounter = 0;

  // this procedure will render the generic backgound to the screen

	// get the video object
  GetVideoObject(&hHandle, guiBACKGROUND);

	// render each row 5 times wide, 5 tiles high
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
	CHECKF(AddVideoObjectFromFile(ImageFile, &guiIMPSYMBOL));

	return (TRUE) ;
}


void DeleteIMPSymbol( void )
{

  // remove IMP symbol
	DeleteVideoObjectFromIndex( guiIMPSYMBOL );
}

void RenderIMPSymbol(INT16 sX, INT16 sY)
{
  HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiIMPSYMBOL);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadBeginIndent( void )
{
	// this procedure will load the indent main symbol into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\BeginScreenIndent.sti", &guiBEGININDENT));
	return (TRUE) ;
}


void DeleteBeginIndent( void )
{

  // remove indent symbol

	DeleteVideoObjectFromIndex( guiBEGININDENT );
}

void RenderBeginIndent(INT16 sX, INT16 sY)
{
  HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBEGININDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}







BOOLEAN LoadActivationIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\ActivationIndent.sti", &guiACTIVATIONINDENT));
	return (TRUE) ;
}


void DeleteActivationIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiACTIVATIONINDENT );
}

void RenderActivationIndent(INT16 sX, INT16 sY)
{
  HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiACTIVATIONINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadFrontPageIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\FrontPageIndent.sti", &guiFRONTPAGEINDENT));
	return (TRUE) ;
}


void DeleteFrontPageIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiFRONTPAGEINDENT );
}

void RenderFrontPageIndent(INT16 sX, INT16 sY)
{
  HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFRONTPAGEINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadAnalyse( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\Analyze.sti", &guiANALYSE));
	return (TRUE) ;
}


void DeleteAnalyse( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiANALYSE );
}

void RenderAnalyse(INT16 sX, INT16 sY, INT8 bImageNumber)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiANALYSE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, bImageNumber, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}





BOOLEAN LoadAttributeGraph( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\Attributegraph.sti", &guiATTRIBUTEGRAPH));
	return (TRUE) ;
}


void DeleteAttributeGraph( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiATTRIBUTEGRAPH );
}

void RenderAttributeGraph(INT16 sX, INT16 sY)
{


	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPH);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadAttributeGraphBar( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\AttributegraphBar.sti", &guiATTRIBUTEGRAPHBAR));
	return (TRUE) ;
}


void DeleteAttributeBarGraph( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiATTRIBUTEGRAPHBAR );
}

void RenderAttributeBarGraph(INT16 sX, INT16 sY)
{
  HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPHBAR);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}






BOOLEAN LoadFullNameIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\FullNameIndent.sti", &guiFULLNAMEINDENT));
	return (TRUE);
}


void DeleteFullNameIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiFULLNAMEINDENT );
}

void RenderFullNameIndent(INT16 sX, INT16 sY)
{


	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFULLNAMEINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadNickNameIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\NickName.sti", &guiNICKNAMEINDENT));
	return (TRUE);
}


void DeleteNickNameIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiNICKNAMEINDENT );
}

void RenderNickNameIndent(INT16 sX, INT16 sY)
{


	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNICKNAMEINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadNameIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\NameIndent.sti", &guiNAMEINDENT));
	return (TRUE);
}


void DeleteNameIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiNAMEINDENT );
}

void RenderNameIndent(INT16 sX, INT16 sY)
{


	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNAMEINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadGenderIndent( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\GenderIndent.sti", &guiGENDERINDENT));
	return (TRUE);
}


void DeleteGenderIndent( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiGENDERINDENT );
}

void RenderGenderIndent(INT16 sX, INT16 sY)
{
	HVOBJECT hHandle;


  // get the video object
  GetVideoObject(&hHandle, guiGENDERINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadSmallFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\SmallFrame.sti", &guiSMALLFRAME));
	return (TRUE);
}


void DeleteSmallFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSMALLFRAME );
}

void RenderSmallFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLFRAME);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadSmallSilhouette( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\SmallSilhouette.sti", &guiSMALLSILHOUETTE));
	return (TRUE);
}


void DeleteSmallSilhouette( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSMALLSILHOUETTE );
}

void RenderSmallSilhouette(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLSILHOUETTE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadLargeSilhouette( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\largesilhouette.sti", &guiLARGESILHOUETTE));
	return (TRUE);
}


void DeleteLargeSilhouette( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLARGESILHOUETTE );
}

void RenderLargeSilhouette(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLARGESILHOUETTE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttributeFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\attributeframe.sti", &guiATTRIBUTEFRAME));
	return (TRUE);
}


void DeleteAttributeFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiATTRIBUTEFRAME );
}

void RenderAttributeFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;
	INT32 iCounter = 0;
	INT16 sCurrentY = 0;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

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
	HVOBJECT hHandle;

	// valid index?
	if( iIndex == -1 )
	{
		return;
	}

	sCurrentY = ( INT16 )( 10 + ( iIndex * 20 ) );

	 // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

	// blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

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
	CHECKF(AddVideoObjectFromFile("LAPTOP\\attributeslider.sti", &guiSLIDERBAR));
	return (TRUE);
}


void DeleteSliderBar( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSLIDERBAR );
}

void RenderSliderBar(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSLIDERBAR);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}




BOOLEAN LoadButton2Image( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\button_2.sti", &guiBUTTON2IMAGE));
	return (TRUE);
}


void DeleteButton2Image( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiBUTTON2IMAGE );
}

void RenderButton2Image(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON2IMAGE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadButton4Image( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\button_4.sti", &guiBUTTON4IMAGE));
	return (TRUE);
}


void DeleteButton4Image( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiBUTTON4IMAGE );
}

void RenderButton4Image(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON4IMAGE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadButton1Image( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\button_1.sti", &guiBUTTON1IMAGE));
	return (TRUE);
}


void DeleteButton1Image( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiBUTTON1IMAGE );
}

void RenderButton1Image(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON1IMAGE);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadPortraitFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\Voice_PortraitFrame.sti", &guiPORTRAITFRAME));
	return (TRUE);
}


void DeletePortraitFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiPORTRAITFRAME );
}

void RenderPortraitFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiPORTRAITFRAME);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadMainIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\mainprofilepageindent.sti", &guiMAININDENT));
	return (TRUE);
}


void DeleteMainIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiMAININDENT );
}

void RenderMainIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiMAININDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnLongIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\longindent.sti", &guiLONGINDENT));
	return (TRUE);
}


void DeleteQtnLongIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLONGINDENT );
}

void RenderQtnLongIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\shortindent.sti", &guiSHORTINDENT));
	return (TRUE);
}


void DeleteQtnShortIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORTINDENT );
}

void RenderQtnShortIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnLongIndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\longindenthigh.sti", &guiLONGHINDENT));
	return (TRUE);
}


void DeleteQtnLongIndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiLONGHINDENT );
}

void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGHINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShortIndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\shortindenthigh.sti", &guiSHORTHINDENT));
	return (TRUE);
}


void DeleteQtnShortIndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORTHINDENT );
}

void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTHINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\questionindent.sti", &guiQINDENT));
	return (TRUE);
}


void DeleteQtnIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiQINDENT );
}

void RenderQtnIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiQINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadAttrib1IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\attributescreenindent_1.sti", &guiA1INDENT));
	return (TRUE);
}


void DeleteAttrib1IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiA1INDENT );
}

void RenderAttrib1IndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA1INDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAttrib2IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\attributescreenindent_2.sti", &guiA2INDENT));
	return (TRUE);
}


void DeleteAttrib2IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiA2INDENT );
}

void RenderAttrib2IndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA2INDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadAvgMercIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\anaveragemercindent.sti", &guiAVGMERCINDENT));
	return (TRUE);
}


void DeleteAvgMercIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiAVGMERCINDENT );
}

void RenderAvgMercIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiAVGMERCINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



BOOLEAN LoadAboutUsIndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\aboutusindent.sti", &guiABOUTUSINDENT));
	return (TRUE);
}


void DeleteAboutUsIndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiABOUTUSINDENT );
}

void RenderAboutUsIndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiABOUTUSINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


BOOLEAN LoadQtnShort2IndentFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\shortindent2.sti", &guiSHORT2INDENT));
	return (TRUE);
}


void DeleteQtnShort2IndentFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORT2INDENT );
}

void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2INDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}

BOOLEAN LoadQtnShort2IndentHighFrame( void )
{
	// this procedure will load the activation indent into memory
	CHECKF(AddVideoObjectFromFile("LAPTOP\\shortindent2High.sti", &guiSHORT2HINDENT));
	return (TRUE);
}


void DeleteQtnShort2IndentHighFrame( void )
{

  // remove activation indent symbol
	DeleteVideoObjectFromIndex( guiSHORT2HINDENT );
}

void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY)
{

	HVOBJECT hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2HINDENT);

  // blt to sX, sY relative to upper left corner
	BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}
