#include "Directories.h"
#include "IMPVideoObjects.h"
#include "VObject.h"
#include "Laptop.h"
#include "GameRes.h"
#include "IMP_Attribute_Selection.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "UILayout.h"


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


void LoadProfileBackGround(void)
{
	// this procedure will load in the graphics for the generic background
	guiBACKGROUND = AddVideoObjectFromFile(LAPTOPDIR "/metalbackground.sti");
}


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


void LoadIMPSymbol(void)
{
	// this procedure will load the IMP main symbol into memory
	guiIMPSYMBOL = AddVideoObjectFromFile(MLG_IMPSYMBOL);
}


void DeleteIMPSymbol( void )
{
	// remove IMP symbol
	DeleteVideoObject(guiIMPSYMBOL);
}


void RenderIMPSymbol(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiIMPSYMBOL, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadBeginIndent(void)
{
	// this procedure will load the indent main symbol into memory
	guiBEGININDENT = AddVideoObjectFromFile(LAPTOPDIR "/beginscreenindent.sti");
}


void DeleteBeginIndent( void )
{
	// remove indent symbol
	DeleteVideoObject(guiBEGININDENT);
}


void RenderBeginIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBEGININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadActivationIndent(void)
{
	// this procedure will load the activation indent into memory
	guiACTIVATIONINDENT = AddVideoObjectFromFile(LAPTOPDIR "/activationindent.sti");
}


void DeleteActivationIndent( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiACTIVATIONINDENT);
}


void RenderActivationIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiACTIVATIONINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadFrontPageIndent(void)
{
	// this procedure will load the activation indent into memory
	guiFRONTPAGEINDENT = AddVideoObjectFromFile(LAPTOPDIR "/frontpageindent.sti");
}


void DeleteFrontPageIndent( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiFRONTPAGEINDENT);
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


void LoadNickNameIndent(void)
{
	// this procedure will load the activation indent into memory
	guiNICKNAMEINDENT = AddVideoObjectFromFile(LAPTOPDIR "/nickname.sti");
}


void DeleteNickNameIndent( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiNICKNAMEINDENT);
}


void RenderNickNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNICKNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadNameIndent(void)
{
	// this procedure will load the activation indent into memory
	guiNAMEINDENT = AddVideoObjectFromFile(LAPTOPDIR "/nameindent.sti");
}


void DeleteNameIndent( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiNAMEINDENT);
}


void RenderNameIndent(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiNAMEINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadGenderIndent(void)
{
	// this procedure will load the activation indent into memory
	guiGENDERINDENT = AddVideoObjectFromFile(LAPTOPDIR "/genderindent.sti");
}


void DeleteGenderIndent( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiGENDERINDENT);
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


void LoadLargeSilhouette(void)
{
	// this procedure will load the activation indent into memory
	guiLARGESILHOUETTE = AddVideoObjectFromFile(LAPTOPDIR "/largesilhouette.sti");
}


void DeleteLargeSilhouette( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiLARGESILHOUETTE);
}


void RenderLargeSilhouette(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLARGESILHOUETTE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAttributeFrame(void)
{
	// this procedure will load the activation indent into memory
	guiATTRIBUTEFRAME = AddVideoObjectFromFile(LAPTOPDIR "/attributeframe.sti");
}


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


void LoadSliderBar(void)
{
	// this procedure will load the activation indent into memory
	guiSLIDERBAR = AddVideoObjectFromFile(LAPTOPDIR "/attributeslider.sti");
}


void DeleteSliderBar( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSLIDERBAR);
}


void RenderSliderBar(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSLIDERBAR, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadButton2Image(void)
{
	// this procedure will load the activation indent into memory
	guiBUTTON2IMAGE = AddVideoObjectFromFile(LAPTOPDIR "/button_2.sti");
}


void DeleteButton2Image( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiBUTTON2IMAGE);
}


void RenderButton2Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON2IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadButton4Image(void)
{
	// this procedure will load the activation indent into memory
	guiBUTTON4IMAGE = AddVideoObjectFromFile(LAPTOPDIR "/button_4.sti");
}


void DeleteButton4Image( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiBUTTON4IMAGE);
}


void RenderButton4Image(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiBUTTON4IMAGE, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadPortraitFrame(void)
{
	// this procedure will load the activation indent into memory
	guiPORTRAITFRAME = AddVideoObjectFromFile(LAPTOPDIR "/voice_portraitframe.sti");
}


void DeletePortraitFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiPORTRAITFRAME);
}


void RenderPortraitFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiPORTRAITFRAME, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}



void LoadMainIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiMAININDENT = AddVideoObjectFromFile(LAPTOPDIR "/mainprofilepageindent.sti");
}


void DeleteMainIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiMAININDENT);
}


void RenderMainIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiMAININDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnLongIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiLONGINDENT = AddVideoObjectFromFile(LAPTOPDIR "/longindent.sti");
}


void DeleteQtnLongIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiLONGINDENT);
}


void RenderQtnLongIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnShortIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiSHORTINDENT = AddVideoObjectFromFile(LAPTOPDIR "/shortindent.sti");
}


void DeleteQtnShortIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSHORTINDENT);
}


void RenderQtnShortIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnLongIndentHighFrame(void)
{
	// this procedure will load the activation indent into memory
	guiLONGHINDENT = AddVideoObjectFromFile(LAPTOPDIR "/longindenthigh.sti");
}


void DeleteQtnLongIndentHighFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiLONGHINDENT);
}


void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiLONGHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnShortIndentHighFrame(void)
{
	// this procedure will load the activation indent into memory
	guiSHORTHINDENT = AddVideoObjectFromFile(LAPTOPDIR "/shortindenthigh.sti");
}


void DeleteQtnShortIndentHighFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSHORTHINDENT);
}


void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORTHINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiQINDENT = AddVideoObjectFromFile(LAPTOPDIR "/questionindent.sti");
}


void DeleteQtnIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiQINDENT);
}


void RenderQtnIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiQINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAttrib1IndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiA1INDENT = AddVideoObjectFromFile(LAPTOPDIR "/attributescreenindent_1.sti");
}


void DeleteAttrib1IndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiA1INDENT);
}


void RenderAttrib1IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA1INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAttrib2IndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiA2INDENT = AddVideoObjectFromFile(LAPTOPDIR "/attributescreenindent_2.sti");
}


void DeleteAttrib2IndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiA2INDENT);
}


void RenderAttrib2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiA2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAvgMercIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiAVGMERCINDENT = AddVideoObjectFromFile(LAPTOPDIR "/anaveragemercindent.sti");
}


void DeleteAvgMercIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiAVGMERCINDENT);
}


void RenderAvgMercIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiAVGMERCINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadAboutUsIndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiABOUTUSINDENT = AddVideoObjectFromFile(LAPTOPDIR "/aboutusindent.sti");
}


void DeleteAboutUsIndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiABOUTUSINDENT);
}


void RenderAboutUsIndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiABOUTUSINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnShort2IndentFrame(void)
{
	// this procedure will load the activation indent into memory
	guiSHORT2INDENT = AddVideoObjectFromFile(LAPTOPDIR "/shortindent2.sti");
}


void DeleteQtnShort2IndentFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSHORT2INDENT);
}


void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2INDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}


void LoadQtnShort2IndentHighFrame(void)
{
	// this procedure will load the activation indent into memory
	guiSHORT2HINDENT = AddVideoObjectFromFile(LAPTOPDIR "/shortindent2high.sti");
}


void DeleteQtnShort2IndentHighFrame( void )
{
	// remove activation indent symbol
	DeleteVideoObject(guiSHORT2HINDENT);
}


void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY)
{
	BltVideoObject(FRAME_BUFFER, guiSHORT2HINDENT, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
}
