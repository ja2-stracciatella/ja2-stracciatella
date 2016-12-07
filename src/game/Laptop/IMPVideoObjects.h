#ifndef __IMP_VIDEO_H
#define __IMP_VIDEO_H

#include "Types.h"

// the functions

// metal background
void RenderProfileBackGround( void );
void RemoveProfileBackGround( void );
void LoadProfileBackGround(void);

// imp symbol
void RenderIMPSymbol( INT16 sX, INT16 sY );
void DeleteIMPSymbol( void );
void LoadIMPSymbol(void);

void LoadBeginIndent(void);
void DeleteBeginIndent( void );
void RenderBeginIndent(INT16 sX, INT16 sY);

void LoadActivationIndent(void);
void DeleteActivationIndent( void );
void RenderActivationIndent(INT16 sX, INT16 sY);

void LoadFrontPageIndent(void);
void DeleteFrontPageIndent( void );
void RenderFrontPageIndent(INT16 sX, INT16 sY);

void LoadAnalyse(void);
void DeleteAnalyse( void );

void LoadAttributeGraph(void);
void DeleteAttributeGraph( void );

void LoadNameIndent(void);
void DeleteNameIndent( void );
void RenderNameIndent(INT16 sX, INT16 sY);

void LoadNickNameIndent(void);
void DeleteNickNameIndent( void );
void RenderNickNameIndent(INT16 sX, INT16 sY);

void LoadGenderIndent(void);
void DeleteGenderIndent( void );
void RenderGenderIndent(INT16 sX, INT16 sY);

void LoadSmallSilhouette(void);
void DeleteSmallSilhouette( void );

void LoadLargeSilhouette(void);
void DeleteLargeSilhouette( void );
void RenderLargeSilhouette(INT16 sX, INT16 sY);

void LoadAttributeFrame(void);
void DeleteAttributeFrame( void );
void RenderAttributeFrame(INT16 sX, INT16 sY);

void LoadSliderBar(void);
void DeleteSliderBar( void );
void RenderSliderBar(INT16 sX, INT16 sY);

void LoadButton2Image(void);
void DeleteButton2Image( void );
void RenderButton2Image(INT16 sX, INT16 sY);

void LoadButton4Image(void);
void DeleteButton4Image( void );
void RenderButton4Image(INT16 sX, INT16 sY);

void LoadPortraitFrame(void);
void DeletePortraitFrame( void );
void RenderPortraitFrame(INT16 sX, INT16 sY);

void LoadMainIndentFrame(void);
void DeleteMainIndentFrame( void );
void RenderMainIndentFrame(INT16 sX, INT16 sY);

void LoadQtnLongIndentFrame(void);
void DeleteQtnLongIndentFrame( void );
void RenderQtnLongIndentFrame(INT16 sX, INT16 sY);

void LoadQtnShortIndentFrame(void);
void DeleteQtnShortIndentFrame( void );
void RenderQtnShortIndentFrame(INT16 sX, INT16 sY);

void LoadQtnLongIndentHighFrame(void);
void DeleteQtnLongIndentHighFrame( void );
void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY);

void LoadQtnShortIndentHighFrame(void);
void DeleteQtnShortIndentHighFrame( void );
void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY);

void LoadQtnShort2IndentFrame(void);
void DeleteQtnShort2IndentFrame( void );
void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY);

void LoadQtnShort2IndentHighFrame(void);
void DeleteQtnShort2IndentHighFrame( void );
void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY);

void LoadQtnIndentFrame(void);
void DeleteQtnIndentFrame( void );
void RenderQtnIndentFrame(INT16 sX, INT16 sY);

void LoadAttrib1IndentFrame(void);
void DeleteAttrib1IndentFrame( void );
void RenderAttrib1IndentFrame(INT16 sX, INT16 sY);

void LoadAttrib2IndentFrame(void);
void DeleteAttrib2IndentFrame( void );
void RenderAttrib2IndentFrame(INT16 sX, INT16 sY);

void LoadAvgMercIndentFrame(void);
void DeleteAvgMercIndentFrame( void );
void RenderAvgMercIndentFrame(INT16 sX, INT16 sY);

void LoadAboutUsIndentFrame(void);
void DeleteAboutUsIndentFrame( void );
void RenderAboutUsIndentFrame(INT16 sX, INT16 sY);


void RenderAttributeFrameForIndex( INT16 sX, INT16 sY, INT32 iIndex );


// graphical handles

extern SGPVObject* guiANALYSE;
extern SGPVObject* guiATTRIBUTEGRAPH;
extern SGPVObject* guiSMALLSILHOUETTE;

#endif
