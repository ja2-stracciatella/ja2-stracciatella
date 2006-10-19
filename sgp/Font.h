#ifndef FONT_H
#define FONT_H

#include "Types.h"
#include "VObject.h"


#define		DEFAULT_SHADOW			2
#define		MILITARY_SHADOW			67
#define		NO_SHADOW						0

#ifdef JA2

// these are bogus! No palette is set yet!
// font foreground color symbols
#define		FONT_FCOLOR_WHITE			208
#define		FONT_FCOLOR_RED				162
#define		FONT_FCOLOR_NICERED		164
#define		FONT_FCOLOR_BLUE			203
#define		FONT_FCOLOR_GREEN			184
#define		FONT_FCOLOR_YELLOW		144
#define		FONT_FCOLOR_BROWN			184
#define		FONT_FCOLOR_ORANGE		76
#define		FONT_FCOLOR_PURPLE		160

// font background color symbols
#define		FONT_BCOLOR_WHITE			208
#define		FONT_BCOLOR_RED				162
#define		FONT_BCOLOR_BLUE			203
#define		FONT_BCOLOR_GREEN			184
#define		FONT_BCOLOR_YELLOW		144
#define		FONT_BCOLOR_BROWN			80
#define		FONT_BCOLOR_ORANGE		76
#define		FONT_BCOLOR_PURPLE		160

#else

// font foreground color symbols
#define		FONT_FCOLOR_WHITE			0x0000
#define		FONT_FCOLOR_RED				0x0000
#define		FONT_FCOLOR_BLUE			0x0000
#define		FONT_FCOLOR_GREEN			0x0000
#define		FONT_FCOLOR_YELLOW		0x0000
#define		FONT_FCOLOR_BROWN			0x0000
#define		FONT_FCOLOR_ORANGE		0x0000
#define		FONT_FCOLOR_PURPLE		0x0000

// font background color symbols
#define		FONT_BCOLOR_WHITE			0x0000
#define		FONT_BCOLOR_RED				0x0000
#define		FONT_BCOLOR_BLUE			0x0000
#define		FONT_BCOLOR_GREEN			0x0000
#define		FONT_BCOLOR_YELLOW		0x0000
#define		FONT_BCOLOR_BROWN			0x0000
#define		FONT_BCOLOR_ORANGE		0x0000
#define		FONT_BCOLOR_PURPLE		0x0000

// font glyphs for spell targeting types
#define		FONT_GLYPH_TARGET_POINT		0xFFF0
#define		FONT_GLYPH_TARGET_CONE		0xFFF1
#define		FONT_GLYPH_TARGET_SINGLE	0xFFF2
#define		FONT_GLYPH_TARGET_GROUP		0xFFF3
#define		FONT_GLYPH_TARGET_NONE		0xFFF4

#endif


typedef struct
{
	UINT16  usNumberOfSymbols;
	UINT16 *DynamicArrayOf16BitValues;
} FontTranslationTable;

#ifdef __cplusplus
extern "C" {
#endif

extern INT32		FontDefault;


void SetFontColors(UINT16 usColors);
void SetFontForeground(UINT8 ubForeground);
void SetFontBackground(UINT8 ubBackground);
void SetFontShadow(UINT8 ubBackground);

void DestroyEnglishTransTable( void );

extern HVOBJECT	 GetFontObject(INT32 iFont);
extern UINT32		 gprintf(INT32 x, INT32 y, const wchar_t *pFontString, ...);
extern UINT32		 mprintf(INT32 x, INT32 y, const wchar_t *pFontString, ...);
extern UINT32		 mprintf_buffer( UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y, const wchar_t *pFontString, ...);

UINT32 mprintf_buffer_coded( UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y, wchar_t *pFontString, ...);
UINT32 mprintf_coded( INT32 x, INT32 y, wchar_t *pFontString, ...);


extern BOOLEAN	 SetFontDestBuffer(UINT32 DestBuffer, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOLEAN wrap);
extern BOOLEAN	 SetFont(INT32 iFontIndex);

extern INT32		 LoadFontFile(const char *pFileName);
extern UINT16    GetFontHeight(INT32 FontNum);
extern BOOLEAN   InitializeFontManager(UINT16 usDefaultPixDepth, FontTranslationTable *pTransTable);
extern void      ShutdownFontManager(void);
extern void			 UnloadFont(UINT32 FontIndex);

extern FontTranslationTable *CreateEnglishTransTable(  );

extern INT16 GetIndex(UINT16 siChar);
extern UINT32 GetWidth(HVOBJECT hSrcVObject, INT16 ssIndex);

extern INT16 StringPixLengthArgFastHelp( INT32 usUseFont, INT32 usBoldFont, UINT32 uiCharCount, wchar_t *pFontString );
extern INT16 StringPixLengthArg(INT32 usUseFont, UINT32 uiCharCount, wchar_t *pFontString, ...);
extern INT16 StringPixLength(const wchar_t *string,INT32 UseFont);
extern void SaveFontSettings(void);
extern void RestoreFontSettings(void);

void VarFindFontRightCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, const wchar_t *pFontString, ... );
void VarFindFontCenterCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, const wchar_t *pFontString, ... );
void FindFontRightCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t *pStr, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY );
void FindFontCenterCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t *pStr, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY );

#ifdef __cplusplus
}
#endif

#endif
