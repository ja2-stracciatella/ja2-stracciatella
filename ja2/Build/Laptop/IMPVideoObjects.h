#ifndef __IMP_VIDEO_H
#define __IMP_VIDEO_H

#include "types.h"

// the functions

// metal background
void RenderProfileBackGround( void );
void RemoveProfileBackGround( void );
BOOLEAN LoadProfileBackGround( void );

// imp symbol
void RenderIMPSymbol( INT16 sX, INT16 sY );
void DeleteIMPSymbol( void );
BOOLEAN LoadIMPSymbol( void );

BOOLEAN LoadBeginIndent( void );
void DeleteBeginIndent( void );
void RenderBeginIndent(INT16 sX, INT16 sY);

BOOLEAN LoadActivationIndent( void );
void DeleteActivationIndent( void );
void RenderActivationIndent(INT16 sX, INT16 sY);

BOOLEAN LoadFrontPageIndent( void );
void DeleteFrontPageIndent( void );
void RenderFrontPageIndent(INT16 sX, INT16 sY);

BOOLEAN LoadAnalyse( void );
void DeleteAnalyse( void );
void RenderAnalyse(INT16 sX, INT16 sY, INT8 bImageNumber);

BOOLEAN LoadAttributeGraph( void );
void DeleteAttributeGraph( void );
void RenderAttributeGraph(INT16 sX, INT16 sY);

BOOLEAN LoadAttributeGraphBar( void );
void DeleteAttributeBarGraph( void );
void RenderAttributeBarGraph(INT16 sX, INT16 sY);

BOOLEAN LoadFullNameIndent( void );
void DeleteFullNameIndent( void );
void RenderFullNameIndent(INT16 sX, INT16 sY);

BOOLEAN LoadNameIndent( void );
void DeleteNameIndent( void );
void RenderNameIndent(INT16 sX, INT16 sY);

BOOLEAN LoadNickNameIndent( void );
void DeleteNickNameIndent( void );
void RenderNickNameIndent(INT16 sX, INT16 sY);

BOOLEAN LoadGenderIndent( void );
void DeleteGenderIndent( void );
void RenderGenderIndent(INT16 sX, INT16 sY);

BOOLEAN LoadSmallFrame( void );
void DeleteSmallFrame( void );
void RenderSmallFrame(INT16 sX, INT16 sY);

BOOLEAN LoadSmallSilhouette( void );
void DeleteSmallSilhouette( void );
void RenderSmallSilhouette(INT16 sX, INT16 sY);

BOOLEAN LoadLargeSilhouette( void );
void DeleteLargeSilhouette( void );
void RenderLargeSilhouette(INT16 sX, INT16 sY);

BOOLEAN LoadAttributeFrame( void );
void DeleteAttributeFrame( void );
void RenderAttributeFrame(INT16 sX, INT16 sY);

BOOLEAN LoadSliderBar( void );
void DeleteSliderBar( void );
void RenderSliderBar(INT16 sX, INT16 sY);


BOOLEAN LoadButton2Image( void );
void DeleteButton2Image( void );
void RenderButton2Image(INT16 sX, INT16 sY);

BOOLEAN LoadButton4Image( void );
void DeleteButton4Image( void );
void RenderButton4Image(INT16 sX, INT16 sY);

BOOLEAN LoadButton1Image( void );
void DeleteButton1Image( void );
void RenderButton1Image(INT16 sX, INT16 sY);

BOOLEAN LoadPortraitFrame( void );
void DeletePortraitFrame( void );
void RenderPortraitFrame(INT16 sX, INT16 sY);

BOOLEAN LoadMainIndentFrame( void );
void DeleteMainIndentFrame( void );
void RenderMainIndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnLongIndentFrame( void );
void DeleteQtnLongIndentFrame( void );
void RenderQtnLongIndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnShortIndentFrame( void );
void DeleteQtnShortIndentFrame( void );
void RenderQtnShortIndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnLongIndentHighFrame( void );
void DeleteQtnLongIndentHighFrame( void );
void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnShortIndentHighFrame( void );
void DeleteQtnShortIndentHighFrame( void );
void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnShort2IndentFrame( void );
void DeleteQtnShort2IndentFrame( void );
void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnShort2IndentHighFrame( void );
void DeleteQtnShort2IndentHighFrame( void );
void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY);

BOOLEAN LoadQtnIndentFrame( void );
void DeleteQtnIndentFrame( void );
void RenderQtnIndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadAttrib1IndentFrame( void );
void DeleteAttrib1IndentFrame( void );
void RenderAttrib1IndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadAttrib2IndentFrame( void );
void DeleteAttrib2IndentFrame( void );
void RenderAttrib2IndentFrame(INT16 sX, INT16 sY);

BOOLEAN LoadAvgMercIndentFrame( void );
void DeleteAvgMercIndentFrame( void );
void RenderAvgMercIndentFrame(INT16 sX, INT16 sY);


BOOLEAN LoadAboutUsIndentFrame( void );
void DeleteAboutUsIndentFrame( void );
void RenderAboutUsIndentFrame(INT16 sX, INT16 sY);


void RenderAttributeFrameForIndex( INT16 sX, INT16 sY, INT32 iIndex );


// graphical handles

extern UINT32 guiBACKGROUND;
extern UINT32 guiIMPSYMBOL;
extern UINT32 guiBEGININDENT;
extern UINT32 guiACTIVATIONINDENT;
extern UINT32 guiFRONTPAGEINDENT;
extern UINT32 guiFULLNAMEINDENT;
extern UINT32 guiGENDERINDENT;
extern UINT32 guiSMALLFRAME;
extern UINT32 guiANALYSE;
extern UINT32 guiATTRIBUTEGRAPH;
extern UINT32 guiATTRIBUTEGRAPHBAR;
extern UINT32 guiSMALLSILHOUETTE;
extern UINT32 guiLARGESILHOUETTE;
extern UINT32 guiPORTRAITFRAME;
extern UINT32 guiATTRIBUTEFRAME;
extern UINT32 guiATTRIBUTESCREENINDENT1;
extern UINT32 guiATTRIBUTESCREENINDENT2;
extern UINT32 guiSLIDERBAR;

#endif
