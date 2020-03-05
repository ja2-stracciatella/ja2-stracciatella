#include <stdarg.h>
#include "HImage.h"
#include "Local.h"
#include "Types.h"
#include "MemMan.h"
#include "Font.h"
#include "Font_Control.h"
#include "Debug.h"
#include "TranslationTable.h"
#include "VSurface.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "UILayout.h"
#include "GameRes.h"
#include "Logger.h"

typedef UINT8 GlyphIdx;


// Destination printing parameters
SGPFont             FontDefault      = 0;
static SGPVSurface* FontDestBuffer;
static SGPRect      FontDestRegion   = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
static UINT32       FontForeground32 = 0;
static UINT32       FontBackground32 = 0;
static UINT32       FontShadow32     = DEFAULT_SHADOW;

// Temp, for saving printing parameters
static SGPFont      SaveFontDefault      = 0;
static SGPVSurface* SaveFontDestBuffer   = NULL;
static SGPRect      SaveFontDestRegion   = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
static UINT32       SaveFontForeground16 = 0;
static UINT32       SaveFontShadow16     = 0;
static UINT32       SaveFontBackground16 = 0;


/* Sets both the foreground and the background colors of the current font. The
 * top byte of the parameter word is the background color, and the bottom byte
 * is the foreground. */
void SetFontColors(UINT32 uForeground, UINT32 uBackground)
{
	SetFontForeground(uForeground);
	SetFontBackground(uBackground);
}


/* Sets the foreground color of the currently selected font. The parameter is
 * the index into the 8-bit palette. In 16BPP mode, the RGB values from the
 * palette are used to create the pixel color. Note that if you change fonts,
 * the selected foreground/background colors will stay at what they are
 * currently set to. */
void SetFontForeground(UINT32 uForeground)
{
	FontForeground32 = uForeground;

//	if (!FontDefault) return;
//	const SGPPaletteEntry *c = FontDefault->Palette();

//	printf("const UINT32 gColorFromPalette[256] = {\n");
//	for(int i = 0; i < 256; i++)
//		printf("\tRGB(%3d, %3d, %3d),\n", c[i].r, c[i].g, c[i].b);
//	printf("};\n\n");

//	for(int i = 0; i < 256; i++)
//		printf("#define FONT_COLOR_P%-3d RGB(%3d, %3d, %3d)\n", i, c[i].r, c[i].g, c[i].b);
}


void SetFontShadow(UINT32 uShadow)
{
	FontShadow32 = uShadow;
}


/* Sets the Background color of the currently selected font. The parameter is
 * the index into the 8-bit palette. In 16BPP mode, the RGB values from the
 * palette are used to create the pixel color. If the background value is zero,
 * the background of the font will be transparent.  Note that if you change
 * fonts, the selected foreground/background colors will stay at what they are
 * currently set to. */
void SetFontBackground(UINT32 uBackground)
{
	FontBackground32 = uBackground;
}


/* Loads a font from an ETRLE file */
SGPFont LoadFontFile(const char *filename)
{
	SGPFont const font = AddScaledAlphaVideoObjectFromFile(filename);
	if (!FontDefault) FontDefault = font;
	return font;
}


/* Deletes the video object of a particular font. Frees up the memory and
 * resources allocated for it. */
void UnloadFont(SGPFont const font)
{
	Assert(font);
	DeleteVideoObject(font);
}


/* Returns the width of a given character in the font. */
static UINT32 GetWidth(HVOBJECT const hSrcVObject, GlyphIdx const ssIndex)
{
	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(ssIndex);
	return pTrav.usWidth + pTrav.sOffsetX;
}


/* Returns the length of a string in pixels, depending on the font given. */
INT16 StringPixLength(wchar_t const* const string, SGPFont const font)
{
	if (!string) return 0;

	UINT32 w = 0;
	for (wchar_t const* c = string; *c != L'\0'; ++c)
	{
		w += GetCharWidth(font, *c);
	}
	return w;
}


/* Saves the current font printing settings into temporary locations. */
void SaveFontSettings(void)
{
	SaveFontDefault      = FontDefault;
	SaveFontDestBuffer   = FontDestBuffer;
	SaveFontDestRegion   = FontDestRegion;
	SaveFontForeground16 = FontForeground32;
	SaveFontShadow16     = FontShadow32;
	SaveFontBackground16 = FontBackground32;
}


/* Restores the last saved font printing settings from the temporary lactions */
void RestoreFontSettings(void)
{
	FontDefault      = SaveFontDefault;
	FontDestBuffer   = SaveFontDestBuffer;
	FontDestRegion   = SaveFontDestRegion;
	FontForeground32 = SaveFontForeground16;
	FontShadow32     = SaveFontShadow16;
	FontBackground32 = SaveFontBackground16;
}


/* Returns the height of a given character in the font. */
static UINT32 GetHeight(HVOBJECT hSrcVObject, INT16 ssIndex)
{
	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(ssIndex);
	return pTrav.usHeight + pTrav.sOffsetY;
}


/* Returns the height of the first character in a font. */
UINT16 GetFontHeight(SGPFont const font)
{
	return GetHeight(font, 0);
}


bool IsPrintableChar(wchar_t const c)
{
	if (TRANSLATION_TABLE_SIZE <= c) return false;
	return TranslationTable[c] != 0 || c == getZeroGlyphChar();
}


/* Given a wide char, this function returns the index of the glyph. If no glyph
 * exists for the requested wide char, the glyph index of '?' is returned. */
static GlyphIdx GetGlyphIndex(wchar_t const c)
{
	if ((0 <= c) && (c < TRANSLATION_TABLE_SIZE))
	{
		GlyphIdx const idx = TranslationTable[c];
		if (idx != 0 || c == getZeroGlyphChar()) return idx;
	}
	SLOGE("Invalid character given U+%04X", c);
	return TranslationTable[L'?'];
}


UINT32 GetCharWidth(HVOBJECT SGPFont, wchar_t c)
{
	return GetWidth(SGPFont, GetGlyphIndex(c));
}


/* Sets the current font number. */
void SetFont(SGPFont const font)
{
	Assert(font);
	FontDefault = font;
}


void SetFontAttributes(SGPFont const font, const UINT32 foreground, const UINT32 shadow, const UINT32 background)
{
	SetFont(font);
	SetFontForeground(foreground);
	SetFontShadow(shadow);
	SetFontBackground(background);
}


void SetFontDestBuffer(SGPVSurface *dst, const INT32 x1, const INT32 y1, const INT32 x2, const INT32 y2)
{
	Assert(x2 > x1);
	Assert(y2 > y1);

	FontDestBuffer         = dst;
	FontDestRegion.iLeft   = x1;
	FontDestRegion.iTop    = y1;
	FontDestRegion.iRight  = x2;
	FontDestRegion.iBottom = y2;
}


void SetFontDestBuffer(SGPVSurface *dst)
{
	SetFontDestBuffer(dst, 0, 0, dst->Width(), dst->Height());
}

/** Replace backbuffer if it is used by the font system. */
void ReplaceFontBackBuffer(SGPVSurface* oldBackbuffer, SGPVSurface* newBackbuffer)
{
	if(FontDestBuffer == oldBackbuffer)
	{
		FontDestBuffer = newBackbuffer;
	}

	if(SaveFontDestBuffer == oldBackbuffer)
	{
		SaveFontDestBuffer = newBackbuffer;
	}
}

void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, SGPFont const font, INT16* psNewX, INT16* psNewY)
{
	// Compute the coordinates to right justify the text
	INT16 xp = sWidth - StringPixLength(pStr, font) + sLeft;
	INT16 yp = (sHeight - GetFontHeight(font)) / 2 + sTop;

	*psNewX = xp;
	*psNewY = yp;
}


void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, SGPFont const font, INT16* psNewX, INT16* psNewY)
{
	// Compute the coordinates to center the text
	INT16 xp = (sWidth - StringPixLength(pStr, font) + 1) / 2 + sLeft;
	INT16 yp = (sHeight - GetFontHeight(font)) / 2 + sTop;

	*psNewX = xp;
	*psNewY = yp;
}


void gprintf(INT32 x, INT32 const y, wchar_t const* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), fmt, ap);
	va_end(ap);

	SGPVSurface::Lock l(FontDestBuffer);
	UINT32* const buf   = l.Buffer<UINT32>();
	UINT32  const pitch = l.Pitch();
	SGPFont const font  = FontDefault;
	for (wchar_t const* i = string; *i != L'\0'; ++i)
	{
		GlyphIdx const glyph = GetGlyphIndex(*i);
		Blt32BPPDataTo32BPPBufferTransparentClip(buf, pitch, font, x, y, glyph, &FontDestRegion);
		x += GetWidth(font, glyph);
	}
}


UINT32 MPrintChar(INT32 const x, INT32 const y, wchar_t const c)
{
	GlyphIdx const glyph = GetGlyphIndex(c);
	SGPFont  const font  = FontDefault;
	{
		SGPVSurface::Lock l(FontDestBuffer);
		Blt8BPPDataTo32BPPBufferMonoShadowClip(l.Buffer<UINT32>(), l.Pitch(), font, x, y, glyph, &FontDestRegion, FontForeground32, FontBackground32, FontShadow32);
	}
	return GetWidth(font, glyph);
}


void MPrintBuffer(UINT32 *pDestBuf, const UINT32 uiDestPitchBYTES, INT32 x, const INT32 y, const wchar_t *str)
{
	SGPFont const font = FontDefault;
	for (; *str != L'\0'; ++str)
	{
		GlyphIdx const glyph = GetGlyphIndex(*str);
		Blt8BPPDataTo32BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, font, x, y, glyph, &FontDestRegion, FontForeground32, FontBackground32, FontShadow32);
		x += GetWidth(font, glyph);
	}
}


void MPrint(const INT32 x, const INT32 y, const wchar_t *str)
{
	SGPVSurface::Lock l(FontDestBuffer);
	MPrintBuffer(l.Buffer<UINT32>(), l.Pitch(), x, y, str);
}


/* Prints to the currently selected destination buffer, at the X/Y coordinates
 * specified, using the currently selected font. Other than the X/Y coordinates,
 * the parameters are identical to printf. The resulting string may be no longer
 * than 512 word-characters. Uses monochrome font color settings */
void mprintf(INT32 const x, INT32 const y, wchar_t const* const fmt, ...)
{
	wchar_t str[512];
	va_list ap;
	va_start(ap, fmt);
	vswprintf(str, lengthof(str), fmt, ap);
	va_end(ap);
	MPrint(x, y, str);
}


void mprintf_buffer(UINT32 *pDestBuf, const UINT32 uiDestPitchBYTES, const INT32 x, const INT32 y, const wchar_t *fmt, ...)
{
	wchar_t str[512];
	va_list ap;
	va_start(ap, fmt);
	vswprintf(str, lengthof(str), fmt, ap);
	va_end(ap);
	MPrintBuffer(pDestBuf, uiDestPitchBYTES, x, y, str);
}


void InitializeFontManager(void)
{
	FontDefault    = 0;
	SetFontDestBuffer(BACKBUFFER);
}
