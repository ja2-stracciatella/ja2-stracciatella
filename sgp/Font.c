#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include "Types.h"
#include "MemMan.h"
#include "Font.h"
#include "Debug.h"
#include "Video.h"
#include "VObject.h"
#include "VObject_Blitters.h"


#define MAX_FONTS 25


struct FontTranslationTable
{
	UINT16  usNumberOfSymbols;
	UINT16* DynamicArrayOf16BitValues;
};
typedef struct FontTranslationTable FontTranslationTable;


static FontTranslationTable TranslationTable;
static HVOBJECT FontObjs[MAX_FONTS];

// Destination printing parameters
INT32 FontDefault = -1;
static UINT32  FontDestBuffer   = BACKBUFFER;
static SGPRect FontDestRegion   = { 0, 0, 640, 480 };
static BOOLEAN FontDestWrap     = FALSE;
static UINT16  FontForeground16 = 0;
static UINT16  FontBackground16 = 0;
static UINT16  FontShadow16     = DEFAULT_SHADOW;

// Temp, for saving printing parameters
static INT32   SaveFontDefault      = -1;
static UINT32  SaveFontDestBuffer   = BACKBUFFER;
static SGPRect SaveFontDestRegion   = { 0, 0, 640, 480};
static BOOLEAN SaveFontDestWrap     = FALSE;
static UINT16  SaveFontForeground16 = 0;
static UINT16  SaveFontShadow16     = 0;
static UINT16  SaveFontBackground16 = 0;


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
	if (FontDefault < 0 || (FontDefault > MAX_FONTS)) return;

	UINT32 uiRed   = FontObjs[FontDefault]->pPaletteEntry[ubForeground].peRed;
	UINT32 uiGreen = FontObjs[FontDefault]->pPaletteEntry[ubForeground].peGreen;
	UINT32 uiBlue  = FontObjs[FontDefault]->pPaletteEntry[ubForeground].peBlue;

	FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}


void SetFontShadow(UINT8 ubShadow)
{
	if (FontDefault < 0 || FontDefault > MAX_FONTS) return;

	UINT32 uiRed   = FontObjs[FontDefault]->pPaletteEntry[ubShadow].peRed;
	UINT32 uiGreen = FontObjs[FontDefault]->pPaletteEntry[ubShadow].peGreen;
	UINT32 uiBlue  = FontObjs[FontDefault]->pPaletteEntry[ubShadow].peBlue;

	FontShadow16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));

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
	if (FontDefault < 0 || FontDefault > MAX_FONTS) return;

	UINT32 uiRed   = FontObjs[FontDefault]->pPaletteEntry[ubBackground].peRed;
	UINT32 uiGreen = FontObjs[FontDefault]->pPaletteEntry[ubBackground].peGreen;
	UINT32 uiBlue  = FontObjs[FontDefault]->pPaletteEntry[ubBackground].peBlue;

	FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}


/* Returns the VOBJECT pointer of a font. */
HVOBJECT GetFontObject(INT32 iFont)
{
	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] != NULL);

	return(FontObjs[iFont]);
}


/* Locates an empty slot in the font table. */
static INT32 FindFreeFont(void)
{
	for (int count = 0; count < MAX_FONTS; count++)
		if (FontObjs[count] == NULL) return count;

	return -1;
}


/* Loads a font from an ETRLE file, and inserts it into one of the font slots.
 * This function returns (-1) if it fails, and debug msgs for a reason.
 * Otherwise the font number is returned. */
INT32 LoadFontFile(const char *filename)
{
	Assert(filename != NULL);
	Assert(strlen(filename));

	UINT32 LoadIndex = FindFreeFont();
	if (LoadIndex == -1)
	{
		DebugMsg(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Out of font slots (%s)", filename));
#ifdef JA2
		FatalError("Cannot init FONT file %s", filename);
#endif
		return -1;
	}

	FontObjs[LoadIndex] = CreateVideoObjectFromFile(filename);
	if (FontObjs[LoadIndex] == NULL)
	{
		DebugMsg(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error creating VOBJECT (%s)", filename));
#ifdef JA2
		FatalError("Cannot init FONT file %s", filename);
#endif
		return -1;
	}

	if (FontDefault == -1) FontDefault = LoadIndex;

	return LoadIndex;
}


/* Deletes the video object of a particular font. Frees up the memory and
 * resources allocated for it. */
void UnloadFont(UINT32 FontIndex)
{
	Assert(FontIndex >= 0);
	Assert(FontIndex <= MAX_FONTS);
	Assert(FontObjs[FontIndex] != NULL);

	DeleteVideoObject(FontObjs[FontIndex]);
	FontObjs[FontIndex] = NULL;
}


/* Returns the width of a given character in the font. */
UINT32 GetWidth(HVOBJECT hSrcVObject, INT16 ssIndex)
{
	Assert(hSrcVObject != NULL);

	if (ssIndex < 0 || ssIndex > 92)
	{
		int i=0;
	}

	// Get Offsets from Index into structure
	const ETRLEObject* pTrav = &hSrcVObject->pETRLEObject[ssIndex];
	return pTrav->usWidth + pTrav->sOffsetX;
}


/* Returns the length of a string with a variable number of arguments, in
 * pixels, using the current font. Maximum length in characters the string can
 * evaluate to is 512.
 * 'uiCharCount' specifies how many characters of the string are counted. */
INT16 StringPixLengthArg(INT32 usUseFont, UINT32 uiCharCount, const wchar_t* pFontString, ...)
{
	Assert(pFontString != NULL);

	va_list argptr;
	va_start(argptr, pFontString);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);
	va_end(argptr);

	// make sure the character count is legal
	if (uiCharCount > wcslen(string))
	{
		uiCharCount = wcslen(string);
	}
	else
	{
		if (uiCharCount < wcslen(string))
		{
			// less than the full string, so whack off the end of it (it's temporary anyway)
			string[uiCharCount] = '\0';
		}
	}

	return(StringPixLength(string, usUseFont));
}


/* Returns the length of a string with a variable number of arguments, in
 * pixels, using the current font. Maximum length in characters the string can
 * evaluate to is 512.  Because this is for fast help text, all '|' characters
 * are ignored for the width calculation.
 * 'uiCharCount' specifies how many characters of the string are counted.
 * YOU HAVE TO PREBUILD THE FAST HELP STRING! */
INT16 StringPixLengthArgFastHelp(INT32 usUseFont, INT32 usBoldFont, UINT32 uiCharCount, const wchar_t* pFontString)
{
	Assert(pFontString != NULL);

	wchar_t	string[512];
	wcscpy(string, pFontString);

	// make sure the character count is legal
	if (uiCharCount > wcslen(string))
	{
		uiCharCount = wcslen(string);
	}
	else
	{
		if (uiCharCount < wcslen(string))
		{
			// less than the full string, so whack off the end of it (it's temporary anyway)
			string[uiCharCount] = '\0';
		}
	}

	//now eliminate all '|' characters from the string.
	INT16 sBoldDiff = 0;
	for (UINT32 i = 0; i < uiCharCount; i++)
	{
		if (string[i] == '|')
		{
			for (UINT32 index = i; index < uiCharCount; index++)
			{
				string[index] = string[index + 1];
			}
			uiCharCount--;
			//now we have eliminated the '|' character, so now calculate the size difference of the
			//bolded character.
			wchar_t str[2];
			str[0] = string[i];
			str[1] = L'\0';
			sBoldDiff += StringPixLength(str, usBoldFont) - StringPixLength(str, usUseFont);
		}
	}
	return StringPixLength(string, usUseFont) + sBoldDiff;
}


/* Returns the length of a string in pixels, depending on the font given. */
INT16 StringPixLength(const wchar_t *string, INT32 UseFont)
{
	if (string == NULL) return 0;

	UINT32 Cur = 0;
	for (const wchar_t* curletter = string; *curletter != L'\0'; curletter++)
	{
		wchar_t transletter = GetIndex(*curletter);
		Cur += GetWidth(FontObjs[UseFont], transletter);
	}
	return Cur;
}


/* Saves the current font printing settings into temporary locations. */
void SaveFontSettings(void)
{
	SaveFontDefault      = FontDefault;
	SaveFontDestBuffer   = FontDestBuffer;
	SaveFontDestRegion   = FontDestRegion;
	SaveFontDestWrap     = FontDestWrap;
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
	FontDestWrap     = SaveFontDestWrap;
	FontForeground16 = SaveFontForeground16;
	FontShadow16     = SaveFontShadow16;
	FontBackground16 = SaveFontBackground16;
}


/* Returns the height of a given character in the font. */
static UINT32 GetHeight(HVOBJECT hSrcVObject, INT16 ssIndex)
{
	Assert(hSrcVObject != NULL);

	// Get Offsets from Index into structure
	const ETRLEObject* pTrav = &hSrcVObject->pETRLEObject[ssIndex];
	return pTrav->usHeight + pTrav->sOffsetY;
}


/* Returns the height of the first character in a font. */
UINT16 GetFontHeight(INT32 FontNum)
{
	Assert(FontNum >= 0);
	Assert(FontNum <= MAX_FONTS);
	Assert(FontObjs[FontNum] != NULL);

	return GetHeight(FontObjs[FontNum], 0);
}


/* Given a word-sized character, this function returns the index of the cell in
 * the font to print to the screen. The conversion table is built by
 *	CreateEnglishTransTable() */
INT16 GetIndex(UINT16 siChar)
{
	UINT16 usNumberOfSymbols = TranslationTable.usNumberOfSymbols;

	// search the Translation Table and return the index for the font
	const UINT16* pTrav = TranslationTable.DynamicArrayOf16BitValues;
	for (UINT16 ssCount = 0; ssCount < usNumberOfSymbols; ssCount++)
	{
		if (siChar == pTrav[ssCount]) return ssCount;
	}

	// If here, present warning and give the first index
	DebugMsg(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error: Invalid character given %d", siChar));

	// Return 0 here, NOT -1 - we should see A's here now...
	return 0;
}


/* Sets the current font number. */
BOOLEAN SetFont(INT32 iFontIndex)
{
	Assert(iFontIndex >= 0);
	Assert(iFontIndex <= MAX_FONTS);
	Assert(FontObjs[iFontIndex] != NULL);

	FontDefault = iFontIndex;
	return TRUE;
}


/* Sets the destination buffer for printing to, the clipping rectangle, and
 * sets the line wrap on/off. DestBuffer is a VOBJECT handle, not a pointer. */
BOOLEAN SetFontDestBuffer(UINT32 DestBuffer, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOLEAN wrap)
{
	Assert(x2 > x1);
	Assert(y2 > y1);

	FontDestBuffer         = DestBuffer;
	FontDestRegion.iLeft   = x1;
	FontDestRegion.iTop    = y1;
	FontDestRegion.iRight  = x2;
	FontDestRegion.iBottom = y2;
	FontDestWrap           = wrap;

	return TRUE;
}


/* Prints to the currently selected destination buffer, at the X/Y coordinates
 * specified, using the currently selected font. Other than the X/Y coordinates,
 * the parameters are identical to printf. The resulting string may be no longer
 * than 512 word-characters. Uses monochrome font color settings */
UINT32 mprintf(INT32 x, INT32 y, const wchar_t* pFontString, ...)
{
	Assert(pFontString != NULL);

	va_list argptr;
	va_start(argptr, pFontString);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);
	va_end(argptr);

	INT32 destx = x;
	INT32 desty = y;

	UINT32 uiDestPitchBYTES;
	UINT8* pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

	for (const wchar_t* curletter = string; *curletter != L'\0'; curletter++)
	{
		wchar_t transletter = GetIndex(*curletter);

		if (FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx = x;
			desty += GetHeight(FontObjs[FontDefault], transletter);
		}

		Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
		destx += GetWidth(FontObjs[FontDefault], transletter);
	}

	UnLockVideoSurface(FontDestBuffer);
	return 0;
}


void VarFindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16* psNewX, INT16* psNewY, const wchar_t* pFontString, ...)
{
	va_list argptr;
	va_start(argptr, pFontString);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);
	va_end(argptr);

	FindFontRightCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}


void VarFindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16* psNewX, INT16* psNewY, const wchar_t* pFontString, ...)
{
	va_list argptr;
	va_start(argptr, pFontString);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);
	va_end(argptr);

	FindFontCenterCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}


void FindFontRightCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, INT32 iFontIndex, INT16* psNewX, INT16* psNewY)
{
	// Compute the coordinates to right justify the text
	INT16 xp = sWidth - StringPixLength(pStr, iFontIndex) + sLeft;
	INT16 yp = (sHeight - GetFontHeight(iFontIndex)) / 2 + sTop;

	*psNewX = xp;
	*psNewY = yp;
}


void FindFontCenterCoordinates(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, const wchar_t* pStr, INT32 iFontIndex, INT16* psNewX, INT16* psNewY)
{
	// Compute the coordinates to center the text
	INT16 xp = (sWidth - StringPixLength(pStr, iFontIndex) + 1) / 2 + sLeft;
	INT16 yp = (sHeight - GetFontHeight(iFontIndex)) / 2 + sTop;

	*psNewX = xp;
	*psNewY = yp;
}


/* Prints to the currently selected destination buffer, at the X/Y coordinates
 * specified, using the currently selected font. Other than the X/Y coordinates,
 * the parameters are identical to printf. The resulting string may be no longer
 * than 512 word-characters. */
UINT32 gprintf(INT32 x, INT32 y, const wchar_t* pFontString, ...)
{
	Assert(pFontString != NULL);

	va_list argptr;
	va_start(argptr, pFontString);
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);
	va_end(argptr);

	INT32 destx = x;
	INT32 desty = y;

	// Lock the dest buffer
	UINT32 uiDestPitchBYTES;
	UINT8* pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

	for (const wchar_t* curletter = string; *curletter != L'\0'; curletter++)
	{
		wchar_t transletter = GetIndex(*curletter);

		if (FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx = x;
			desty += GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion);
		destx += GetWidth(FontObjs[FontDefault], transletter);
	}

	UnLockVideoSurface(FontDestBuffer);
	return 0;
}


UINT32 mprintf_buffer(UINT8* pDestBuf, UINT32 uiDestPitchBYTES, INT32 x, INT32 y, const wchar_t* pFontString, ...)
{
	Assert(pFontString != NULL);

	va_list argptr;
	va_start(argptr, pFontString);       	// Set up variable argument pointer
	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	INT32 destx = x;
	INT32 desty = y;

	for (const wchar_t* curletter = string; *curletter != L'\0'; curletter++)
	{
		wchar_t transletter=GetIndex(*curletter);

		if (FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx = x;
			desty += GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
		destx += GetWidth(FontObjs[FontDefault], transletter);
	}

	return 0;
}


static UINT32 vmprintf_buffer_coded(UINT8* pDestBuf, UINT32 uiDestPitchBYTES, INT32 x, INT32 y, const wchar_t* pFontString, va_list ArgPtr)
{
	Assert(pFontString != NULL);

	wchar_t	string[512];
	vswprintf(string, lengthof(string), pFontString, ArgPtr);

	INT32 destx = x;
	INT32 desty = y;

	UINT16 usOldForeColor = FontForeground16;

	for (const wchar_t* curletter = string; *curletter != 0; curletter++)
	{
		if (*curletter == 180)
		{
			curletter++;
			SetFontForeground(*curletter);
			curletter++;
		}
		else if (*curletter == 181)
		{
			FontForeground16 = usOldForeColor;
			curletter++;
		}

		wchar_t transletter = GetIndex(*curletter);


		if (FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx = x;
			desty += GetHeight(FontObjs[FontDefault], transletter);
		}

		Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
		destx += GetWidth(FontObjs[FontDefault], transletter);
	}

	return 0;
}


UINT32 mprintf_buffer_coded(UINT8* pDestBuf, UINT32 uiDestPitchBYTES, INT32 x, INT32 y, const wchar_t* pFontString, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, pFontString);
	UINT32 Ret = vmprintf_buffer_coded(pDestBuf, uiDestPitchBYTES, x, y, pFontString, ArgPtr);
	va_end(ArgPtr);

	return Ret;
}


UINT32 mprintf_coded(INT32 x, INT32 y, const wchar_t* pFontString, ...)
{
	UINT32 uiDestPitchBYTES;
	UINT8* pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

	va_list ArgPtr;
	va_start(ArgPtr, pFontString);
	UINT32 Ret = vmprintf_buffer_coded(pDestBuf, uiDestPitchBYTES, x, y, pFontString, ArgPtr);
	va_end(ArgPtr);

	UnLockVideoSurface(FontDestBuffer);
	return Ret;
}


static void CreateEnglishTransTable(void);


/* Starts up the font manager system with the appropriate translation table. */
BOOLEAN InitializeFontManager(void)
{
	FontDefault = -1;
	FontDestBuffer = BACKBUFFER;

	UINT16 uiRight;
	UINT16 uiBottom;
	GetCurrentVideoSettings(&uiRight, &uiBottom);
	FontDestRegion.iLeft = 0;
	FontDestRegion.iTop = 0;
	FontDestRegion.iRight=(INT32)uiRight;
	FontDestRegion.iBottom=(INT32)uiBottom;

	FontDestWrap=FALSE;

	CreateEnglishTransTable();

	// Mark all font slots as empty
	for (int count = 0; count < MAX_FONTS; count++) FontObjs[count] = NULL;

	return TRUE;
}


/* Shuts down, and deallocates all fonts. */
void ShutdownFontManager(void)
{
	for (INT32 count = 0; count < MAX_FONTS; count++)
	{
		if (FontObjs[count] != NULL) UnloadFont(count);
	}
}


/* Destroys the English text->font map table. */
void DestroyEnglishTransTable(void)
{
	if (TranslationTable.DynamicArrayOf16BitValues != NULL)
	{
		MemFree(TranslationTable.DynamicArrayOf16BitValues);
	}
}


/* Creates the English text->font map table. */
static void CreateEnglishTransTable(void)
{
#ifdef JA2
	// ha ha, we have more than Wizardry now (again)
	TranslationTable.usNumberOfSymbols = 172;
#else
	TranslationTable.usNumberOfSymbols = 155;
#endif
	UINT16* temp = MemAlloc(TranslationTable.usNumberOfSymbols * sizeof(*temp));
	TranslationTable.DynamicArrayOf16BitValues = temp;

	*temp++ = 'A';
	*temp++ = 'B';
	*temp++ = 'C';
	*temp++ = 'D';
	*temp++ = 'E';
	*temp++ = 'F';
	*temp++ = 'G';
	*temp++ = 'H';
	*temp++ = 'I';
	*temp++ = 'J';
	*temp++ = 'K';
	*temp++ = 'L';
	*temp++ = 'M';
	*temp++ = 'N';
	*temp++ = 'O';
	*temp++ = 'P';
	*temp++ = 'Q';
	*temp++ = 'R';
	*temp++ = 'S';
	*temp++ = 'T';
	*temp++ = 'U';
	*temp++ = 'V';
	*temp++ = 'W';
	*temp++ = 'X';
	*temp++ = 'Y';
	*temp++ = 'Z';
	*temp++ = 'a';
	*temp++ = 'b';
	*temp++ = 'c';
	*temp++ = 'd';
	*temp++ = 'e';
	*temp++ = 'f';
	*temp++ = 'g';
	*temp++ = 'h';
	*temp++ = 'i';
	*temp++ = 'j';
	*temp++ = 'k';
	*temp++ = 'l';
	*temp++ = 'm';
	*temp++ = 'n';
	*temp++ = 'o';
	*temp++ = 'p';
	*temp++ = 'q';
	*temp++ = 'r';
	*temp++ = 's';
	*temp++ = 't';
	*temp++ = 'u';
	*temp++ = 'v';
	*temp++ = 'w';
	*temp++ = 'x';
	*temp++ = 'y';
	*temp++ = 'z';
	*temp++ = '0';
	*temp++ = '1';
	*temp++ = '2';
	*temp++ = '3';
	*temp++ = '4';
	*temp++ = '5';
	*temp++ = '6';
	*temp++ = '7';
	*temp++ = '8';
	*temp++ = '9';
	*temp++ = '!';
	*temp++ = '@';
	*temp++ = '#';
	*temp++ = '$';
	*temp++ = '%';
	*temp++ = '^';
	*temp++ = '&';
	*temp++ = '*';
	*temp++ = '(';
	*temp++ = ')';
	*temp++ = '-';
	*temp++ = '_';
	*temp++ = '+';
	*temp++ = '=';
	*temp++ = '|';
	*temp++ = '\\';
	*temp++ = '{';
	*temp++ = '}';// 80
	*temp++ = '[';
	*temp++ = ']';
	*temp++ = ':';
	*temp++ = ';';
	*temp++ = '"';
	*temp++ = '\'';
	*temp++ = '<';
	*temp++ = '>';
	*temp++ = ',';
	*temp++ = '.';
	*temp++ = '?';
	*temp++ = '/';
	*temp++ = ' '; //93

#ifdef JA2
	*temp++ = 196; // "A" umlaut
	*temp++ = 214; // "O" umlaut
	*temp++ = 220; // "U" umlaut
	*temp++ = 228; // "a" umlaut
	*temp++ = 246; // "o" umlaut
	*temp++ = 252; // "u" umlaut
	*temp++ = 223; // double-s that looks like a beta/B  // 100
	// START OF FUNKY RUSSIAN STUFF
	*temp++ = 1101;
	*temp++ = 1102;
	*temp++ = 1103;
	*temp++ = 1104;
	*temp++ = 1105;
	*temp++ = 1106;
	*temp++ = 1107;
	*temp++ = 1108;
	*temp++ = 1109;
	*temp++ = 1110;
	*temp++ = 1111;
	*temp++ = 1112;
	*temp++ = 1113;
	*temp++ = 1114;
	*temp++ = 1115;
	*temp++ = 1116;
	*temp++ = 1117;
	*temp++ = 1118;
	*temp++ = 1119;
	*temp++ = 1120;
	*temp++ = 1121;
	*temp++ = 1122;
	*temp++ = 1123;
	*temp++ = 1124;
	*temp++ = 1125;
	*temp++ = 1126;
	*temp++ = 1127;
	*temp++ = 1128;
	*temp++ = 1129;
	*temp++ = 1130; // 130
	*temp++ = 1131;
	*temp++ = 1132;
	// END OF FUNKY RUSSIAN STUFF
	*temp++ = 196; // Ä
	*temp++ = 192; // À
	*temp++ = 193; // Á
	*temp++ = 194; // Â
	*temp++ = 199; // Ç
	*temp++ = 203; // Ë
	*temp++ = 200; // È
	*temp++ = 201; // É				140
	*temp++ = 202; // Ê
	*temp++ = 207; // Ï
	*temp++ = 214; // Ö
	*temp++ = 210; // Ò
	*temp++ = 211; // Ó
	*temp++ = 212; // Ô
	*temp++ = 220; // Ü
	*temp++ = 217; // Ù
	*temp++ = 218; // Ú
	*temp++ = 219; // Û				150

	*temp++ = 228; // ä
	*temp++ = 224; // à
	*temp++ = 225; // á
	*temp++ = 226; // â
	*temp++ = 231; // ç
	*temp++ = 235; // ë
	*temp++ = 232; // è
	*temp++ = 233; // é
	*temp++ = 234; // ê
	*temp++ = 239; // ï				160
	*temp++ = 246; // ö
	*temp++ = 242; // ò
	*temp++ = 243; // ó
	*temp++ = 244; // ô
	*temp++ = 252; // ü
	*temp++ = 249; // ù
	*temp++ = 250; // ú
	*temp++ = 251; // û
	*temp++ = 204; // Ì
	*temp++ = 206; // Î				170
	*temp++ = 236; // ì
	*temp++ = 238; // î

#else
// Windows Code Page 1252 Western Standard Character Set

	*temp++ = 193; // "A" acute
	*temp++ = 192; // "A" grave
	*temp++ = 193; // "A" circumflex
	*temp++ = 196; // "A" umlaut
	*temp++ = 195; // "A" tilde
	*temp++ = 197; // "A" ring
	*temp++ = 199; // "C" cedile
	*temp++ = 201; // "E" acute
	*temp++ = 200; // "E" grave
	*temp++ = 202; // "E" circumflex
	*temp++ = 203; // "E" umlaut
	*temp++ = 205; // "I" acute
	*temp++ = 204; // "I" grave
	*temp++ = 206; // "I" circumflex
	*temp++ = 207; // "I" umlaut
	*temp++ = 209; // "N" tilde
	*temp++ = 211; // "O" acute
	*temp++ = 210; // "O" grave
	*temp++ = 212; // "O" circumflex
	*temp++ = 214; // "O" umlaut
	*temp++ = 213; // "O" tilde
	*temp++ = 216; // "0" O strike-through
	*temp++ = 218; // "U" acute
	*temp++ = 217; // "U" grave
	*temp++ = 219; // "U" circumflex
	*temp++ = 220; // "U" umlaut
	*temp++ = 221; // "Y" acute
	*temp++ = 225; // "a" acute
	*temp++ = 224; // "a" grave
	*temp++ = 226; // "a" circumflex
	*temp++ = 228; // "a" umlaut
	*temp++ = 227; // "a" tilde
	*temp++ = 229; // "a" ring
	*temp++ = 231; // "c" cedile
	*temp++ = 233; // "e" acute
	*temp++ = 232; // "e" grave
	*temp++ = 234; // "e" circumflex
	*temp++ = 235; // "e" umlaut
	*temp++ = 237; // "i" acute
	*temp++ = 236; // "i" grave
	*temp++ = 238; // "i" circumflex
	*temp++ = 239; // "i" umlaut
	*temp++ = 241; // "n" tilde
	*temp++ = 243; // "o" acute
	*temp++ = 242; // "o" grave
	*temp++ = 244; // "o" circumflex
	*temp++ = 246; // "o" umlaut
	*temp++ = 245; // "o" tilde
	*temp++ = 248; // "o" strike-through
	*temp++ = 250; // "u" acute
	*temp++ = 249; // "u" grave
	*temp++ = 251; // "u" circumflex
	*temp++ = 252; // "u" umlaut
	*temp++ = 254; // "y" acute
	*temp++ = 255; // "y" umlaut
	*temp++ = 223; // beta

// Font glyphs for spell targeting icons
	//ATE: IMPORTANT! INcreate the array above if you add any new items here...
	*temp++ = FONT_GLYPH_TARGET_POINT;
	*temp++ = FONT_GLYPH_TARGET_CONE;
	*temp++ = FONT_GLYPH_TARGET_SINGLE;
	*temp++ = FONT_GLYPH_TARGET_GROUP;
	*temp++ = FONT_GLYPH_TARGET_NONE;

// 154
#endif
}
