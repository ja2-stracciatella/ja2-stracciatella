#ifndef __IMP_VIDEO_H
#define __IMP_VIDEO_H

#include "Types.h"

// the functions

// metal background
void RenderProfileBackGround( void );
void RemoveProfileBackGround( void );

// imp symbol
void RenderIMPSymbol( INT16 sX, INT16 sY );
void DeleteIMPSymbol( void );

void DeleteBeginIndent( void );
void RenderBeginIndent(INT16 sX, INT16 sY);

void DeleteActivationIndent( void );
void RenderActivationIndent(INT16 sX, INT16 sY);

void DeleteFrontPageIndent( void );
void RenderFrontPageIndent(INT16 sX, INT16 sY);

void LoadAnalyse(void);
void DeleteAnalyse( void );

void LoadAttributeGraph(void);
void DeleteAttributeGraph( void );

void DeleteNameIndent( void );
void RenderNameIndent(INT16 sX, INT16 sY);

void DeleteNickNameIndent( void );
void RenderNickNameIndent(INT16 sX, INT16 sY);

void DeleteGenderIndent( void );
void RenderGenderIndent(INT16 sX, INT16 sY);

void LoadSmallSilhouette(void);
void DeleteSmallSilhouette( void );

void DeleteLargeSilhouette( void );
void RenderLargeSilhouette(INT16 sX, INT16 sY);

void DeleteAttributeFrame( void );
void RenderAttributeFrame(INT16 sX, INT16 sY);

void DeleteSliderBar( void );
void RenderSliderBar(INT16 sX, INT16 sY);

void DeleteButton2Image( void );
void RenderButton2Image(INT16 sX, INT16 sY);

void DeleteButton4Image( void );
void RenderButton4Image(INT16 sX, INT16 sY);

void DeletePortraitFrame( void );
void RenderPortraitFrame(INT16 sX, INT16 sY);

void DeleteMainIndentFrame( void );
void RenderMainIndentFrame(INT16 sX, INT16 sY);

void DeleteQtnLongIndentFrame( void );
void RenderQtnLongIndentFrame(INT16 sX, INT16 sY);

void DeleteQtnShortIndentFrame( void );
void RenderQtnShortIndentFrame(INT16 sX, INT16 sY);

void DeleteQtnLongIndentHighFrame( void );
void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY);

void DeleteQtnShortIndentHighFrame( void );
void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY);

void DeleteQtnShort2IndentFrame( void );
void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY);

void DeleteQtnShort2IndentHighFrame( void );
void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY);

void DeleteQtnIndentFrame( void );
void RenderQtnIndentFrame(INT16 sX, INT16 sY);

void DeleteAttrib1IndentFrame( void );
void RenderAttrib1IndentFrame(INT16 sX, INT16 sY);

void DeleteAttrib2IndentFrame( void );
void RenderAttrib2IndentFrame(INT16 sX, INT16 sY);

void DeleteAvgMercIndentFrame( void );
void RenderAvgMercIndentFrame(INT16 sX, INT16 sY);

void DeleteAboutUsIndentFrame( void );
void RenderAboutUsIndentFrame(INT16 sX, INT16 sY);


void RenderAttributeFrameForIndex( INT16 sX, INT16 sY, INT32 iIndex );


// graphical handles

extern SGPVObject* guiANALYSE;
extern SGPVObject* guiATTRIBUTEGRAPH;
extern SGPVObject* guiSMALLSILHOUETTE;

#endif
