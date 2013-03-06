#include <stdarg.h>
#include "HImage.h"
#include "Local.h"
#include "Types.h"
#include "MemMan.h"
#include "Font.h"
#include "Debug.h"
#include "TranslationTable.h"
#include "VSurface.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "UILayout.h"
#include "GameRes.h"


typedef UINT8 GlyphIdx;


// Destination printing parameters
Font                FontDefault      = 0;
static SGPVSurface* FontDestBuffer;
static SGPRect      FontDestRegion   = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
static UINT16       FontForeground16 = 0;
static UINT16       FontBackground16 = 0;
static UINT16       FontShadow16     = DEFAULT_SHADOW;

// Temp, for saving printing parameters
static Font         SaveFontDefault      = 0;
static SGPVSurface* SaveFontDestBuffer   = NULL;
static SGPRect      SaveFontDestRegion   = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
static UINT16       SaveFontForeground16 = 0;
static UINT16       SaveFontShadow16     = 0;
static UINT16       SaveFontBackground16 = 0;


/* Sets both the foreground and the background colors of the current font. The
 * top byte of the parameter word is the background color, and the bottom byte
 * is the foreground. */
void SetFontColors(UINT16 usColors)
{
	UINT8 ubForeground = usColors & 0xFF;
	UINT8 ubBackground = (usColors >> 8) & 0xFF;

	SetFontForeground(ubForeground);
	SetFontBackground(ubBackground);
}


/* Sets the foreground color of the currently selected font. The parameter is
 * the index into the 8-bit palette. In 16BPP mode, the RGB values from the
 * palette are used to create the pixel color. Note that if you change fonts,
 * the selected foreground/background colors will stay at what they are
 * currently set to. */
void SetFontForeground(UINT8 ubForeground)
{
	if (!FontDefault) return;
	const SGPPaletteEntry* const c = &FontDefault->Palette()[ubForeground];
	FontForeground16 = Get16BPPColor(FROMRGB(c->r, c->g, c->b));
}


void SetFontShadow(UINT8 ubShadow)
{
	if (!FontDefault) return;
	const SGPPaletteEntry* const c = &FontDefault->Palette()[ubShadow];
	FontShadow16 = Get16BPPColor(FROMRGB(c->r, c->g, c->b));

	if (ubShadow != 0 && FontShadow16 == 0) FontShadow16 = 1;
}


/* Sets the Background color of the currently selected font. The parameter is
 * the index into the 8-bit palette. In 16BPP mode, the RGB values from the
 * palette are used to create the pixel color. If the background value is zero,
 * the background of the font will be transparent.  Note that if you change
 * fonts, the selected foreground/background colors will stay at what they are
 * currently set to. */
void SetFontBackground(UINT8 ubBackground)
{
	if (!FontDefault) return;
	const SGPPaletteEntry* const c = &FontDefault->Palette()[ubBackground];
	FontBackground16 = Get16BPPColor(FROMRGB(c->r, c->g, c->b));
}


/* Loads a font from an ETRLE file */
Font LoadFontFile(const char *filename)
{
	Font const font = AddVideoObjectFromFile(filename);
	if (!FontDefault) FontDefault = font;
	return font;
}


/* Deletes the video object of a particular font. Frees up the memory and
 * resources allocated for it. */
void UnloadFont(Font const font)
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
INT16 StringPixLength(wchar_t const* const string, Font const font)
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
	SaveFontForeground16 = FontForeground16;
	SaveFontShadow16     = FontShadow16;
	SaveFontBackground16 = FontBackground16;
}


/* Restores the last saved font printing settings from the temporary lactions */
void RestoreFontSettings(void)
{
	FontDefault      = SaveFontDefault;
	FontDestBuffer   = SaveFontDestBuffer;
	FontDestRegion   = SaveFontDestRegion;
	FontForeground16 = SaveFontForeground16;
	FontShadow16     = SaveFontShadow16;
	FontBackground16 = SaveFontBackground16;
}


/* Returns the height of a given character in the font. */
static UINT32 GetHeight(HVOBJECT hSrcVObject, INT16 ssIndex)
{
	// Get Offsets from Index into structure
	ETRLEObject const& pTrav = hSrcVObject->SubregionProperties(ssIndex);
	return pTrav.usHeight + pTrav.sOffsetY;
}


/* Returns the height of the first character in a font. */
UINT16 GetFontHeight(Font const font)
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
	if (c < TRANSLATION_TABLE_SIZE)
	{
		GlyphIdx const idx = TranslationTable[c];
		if (idx != 0 || c == getZeroGlyphChar()) return idx;
	}
	DebugMsg(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error: Invalid character given U+%04X", c));
	return TranslationTable[L'?'];
}


UINT32 GetCharWidth(HVOBJECT Font, wchar_t c)
{
	return GetWidth(Font, GetGlyphIndex(c));
}


/* Sets the current font number. */
void SetFont(Font const font)
{
	Assert(font);
	FontDefault = font;
}


void SetFontAttributes(Font const font, UINT8 const foreground, UINT8 const shadow, UINT8 const background)
{
	SetFont(font);
	SetFontForeground(foreground);
	SetFontShadow(shadow);
	SetFontBackground(background);
}


void SetFontDestBuffer(SGPVSurface* const dst, const INT32 x1, const INT32 y1, const INT32 x2, const INT32 y2)
{
	Assert(x2 > x1);
	Assert(y2 > y1);

	FontDestBuffer         = dst;
	FontDestRegion.iLeft   = x1;
	FontDestRegion.iTop    = y1;
	FontDestRegion.iRight  = x2;
	FontDestRegion.iBottom = y2;
}


void SetFontDestBuffer(SGPVSurface* const dst)
{
	SetFontDestBuffer(dst, 0, 0, dst->Width(), dst->Height());
}


void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, Font const font, INT16* psNewX, INT16* psNewY)
{
	// Compute the coordinates to right justify the text
	INT16 xp = sWidth - StringPixLength(pStr, font) + sLeft;
	INT16 yp = (sHeight - GetFontHeight(font)) / 2 + sTop;

	*psNewX = xp;
	*psNewY = yp;
}


void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, Font const font, INT16* psNewX, INT16* psNewY)
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
	UINT16* const buf   = l.Buffer<UINT16>();
	UINT32  const pitch = l.Pitch();
	Font    const font  = FontDefault;
	for (wchar_t const* i = string; *i != L'\0'; ++i)
	{
		GlyphIdx const glyph = GetGlyphIndex(*i);
		Blt8BPPDataTo16BPPBufferTransparentClip(buf, pitch, font, x, y, glyph, &FontDestRegion);
		x += GetWidth(font, glyph);
	}
}


UINT32 MPrintChar(INT32 const x, INT32 const y, wchar_t const c)
{
	GlyphIdx const glyph = GetGlyphIndex(c);
	Font     const font  = FontDefault;
	{ SGPVSurface::Lock l(FontDestBuffer);
		Blt8BPPDataTo16BPPBufferMonoShadowClip(l.Buffer<UINT16>(), l.Pitch(), font, x, y, glyph, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
	}
	return GetWidth(font, glyph);
}


void MPrintBuffer(UINT16* const pDestBuf, UINT32 const uiDestPitchBYTES, INT32 x, INT32 const y, wchar_t const* str)
{
	Font const font = FontDefault;
	for (; *str != L'\0'; ++str)
	{
		GlyphIdx const glyph = GetGlyphIndex(*str);
		Blt8BPPDataTo16BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, font, x, y, glyph, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
		x += GetWidth(font, glyph);
	}
}


void MPrint(INT32 const x, INT32 const y, wchar_t const* const str)
{
	SGPVSurface::Lock l(FontDestBuffer);
	MPrintBuffer(l.Buffer<UINT16>(), l.Pitch(), x, y, str);
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


void mprintf_buffer(UINT16* const pDestBuf, UINT32 const uiDestPitchBYTES, INT32 const x, INT32 const y, wchar_t const* const fmt, ...)
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
	FontDestBuffer = BACKBUFFER;

	FontDestRegion.iLeft   = 0;
	FontDestRegion.iTop    = 0;
	FontDestRegion.iRight  = SCREEN_WIDTH;
	FontDestRegion.iBottom = SCREEN_HEIGHT;
}
