#ifndef FONT_H
#define FONT_H

#include "Types.h"
#include "VObject.h"

#include <map>
#include <string_theory/string>


#define DEFAULT_SHADOW		RGB(  0,   0,   0) // 2
#define MILITARY_SHADOW	RGB( 17, 178, 216) // 67
#define NO_SHADOW		0x00000000

// these are bogus! No palette is set yet!
// font foreground color symbols
#define FONT_FCOLOR_WHITE	RGB(255, 255, 255)
#define FONT_FCOLOR_RED	RGB(255,  64,  64)
#define FONT_FCOLOR_NICERED	RGB(205,   0,   0)
#define FONT_FCOLOR_BLUE	RGB(  0,   0, 255)
#define FONT_FCOLOR_GREEN	RGB(  0, 206,   0)
#define FONT_FCOLOR_YELLOW	RGB(255, 215,  63)
#define FONT_FCOLOR_BROWN	RGB(  0, 206,   0) // this ain't right
#define FONT_FCOLOR_ORANGE	RGB(240, 116,  45)
#define FONT_FCOLOR_PURPLE	RGB(255, 193, 193)

extern SGPFont FontDefault;

void SetFontColors(UINT32 uForeground, UINT32 uBackground);
void SetFontForeground(UINT32 uForeground);
void SetFontBackground(UINT32 uBackground);
void SetFontShadow(UINT32 uBackground);

/* Print to the currently selected destination buffer, at the X/Y coordinates
 * specified, using the currently selected font. */

void GPrint(INT32 x, INT32 y, const ST::utf32_buffer& codepoints);
inline void GPrint(INT32 x, INT32 y, const ST::string& str)
{
	GPrint(x, y, str.to_utf32());
}

/* Prints to the currently selected destination buffer, at the X/Y coordinates
 * specified, using the currently selected font. Uses monochrome font color settings */

UINT32 MPrintChar(INT32 x, INT32 y, char32_t c);
void MPrintBuffer(UINT32* pDestBuf, UINT32 uiDestPitchBYTES, INT32 x, INT32 y, const ST::utf32_buffer& codepoints);
inline void MPrintBuffer(UINT32* pDestBuf, UINT32 uiDestPitchBYTES, INT32 x, INT32 y, const ST::string& str)
{
	MPrintBuffer(pDestBuf, uiDestPitchBYTES, x, y, str.to_utf32());
}
void MPrint(INT32 x, INT32 y, const ST::utf32_buffer& codepoints);
inline void MPrint(INT32 x, INT32 y, const ST::string& str)
{
	MPrint(x, y, str.to_utf32());
}

/* Sets the destination buffer for printing to and the clipping rectangle. */
void SetFontDestBuffer(SGPVSurface *dst, const INT32 x1, const INT32 y1, const INT32 x2, const INT32 y2);

/* Set the destination buffer for printing while using the whole surface. */
void SetFontDestBuffer(SGPVSurface* dst);

void SetFont(SGPFont);

void SetFontAttributes(SGPFont, const UINT32 foreground, const UINT32 shadow = DEFAULT_SHADOW, const UINT32 background = 0);

SGPFont LoadFontFile(const char* filename);
UINT16  GetFontHeight(SGPFont);
void    InitializeFontManager(void);
void    UnloadFont(SGPFont);

UINT32 GetCharWidth(HVOBJECT SGPFont, char32_t c);

INT16 StringPixLength(const ST::utf32_buffer& codepoints, SGPFont font);
inline INT16 StringPixLength(const ST::string& str, SGPFont font)
{
	return StringPixLength(str.to_utf32(), font);
}
extern void SaveFontSettings(void);
extern void RestoreFontSettings(void);

void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const ST::utf32_buffer& codepoints, SGPFont font, INT16* psNewX, INT16* psNewY);
inline void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const ST::string& str, SGPFont font, INT16* psNewX, INT16* psNewY)
{
	FindFontRightCoordinates(sLeft, sTop, sWidth, sHeight, str.to_utf32(), font, psNewX, psNewY);
}
void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const ST::utf32_buffer& codepoints, SGPFont font, INT16* psNewX, INT16* psNewY);
inline void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const ST::string& str, SGPFont font, INT16* psNewX, INT16* psNewY)
{
	FindFontCenterCoordinates(sLeft, sTop, sWidth, sHeight, str.to_utf32(), font, psNewX, psNewY);
}

bool IsPrintableChar(char32_t c);

#endif
