// font.c
#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "types.h"
	#include <stdio.h>
	#include <stdarg.h>
	#include <malloc.h>
	#include <windows.h>
	#include <stdarg.h>
	#include <wchar.h>
	#include "sgp.h"
	#include "pcx.h"
	#include "memman.h"
	#include "fileman.h"
	#include "Font.h"
	#include "Debug.h"

	#if defined( JA2 ) || defined( UTIL )
	#include "video.h"
	#else
	#include "video2.h"
	#endif

	#include "himage.h"
	#include "vobject.h"
	#include "vobject_blitters.h"
#endif
//*******************************************************
//
//   Defines
//
//*******************************************************

#define PALETTE_SIZE     768
#define STRING_DELIMITER 0
#define ID_BLACK         0
#define MAX_FONTS				 25

//*******************************************************
//
//   Typedefs
//
//*******************************************************

SGPPaletteEntry gSgpPalette[256];

typedef struct
{
	UINT16 usDefaultPixelDepth;
	FontTranslationTable *pTranslationTable;
} FontManager;

FontManager *pFManager;
HVOBJECT	FontObjs[MAX_FONTS];
INT32		FontsLoaded=0;

// Destination printing parameters
INT32			FontDefault=(-1);
UINT32		FontDestBuffer=BACKBUFFER;
UINT32		FontDestPitch=640*2;
UINT32		FontDestBPP=16;
SGPRect		FontDestRegion={0,0,640,480};
BOOLEAN		FontDestWrap=FALSE;
UINT16		FontForeground16=0;
UINT16		FontBackground16=0;
UINT16		FontShadow16=DEFAULT_SHADOW;
UINT8			FontForeground8=0;
UINT8			FontBackground8=0;

// Temp, for saving printing parameters
INT32			SaveFontDefault=(-1);
UINT32		SaveFontDestBuffer=BACKBUFFER;
UINT32		SaveFontDestPitch=640*2;
UINT32		SaveFontDestBPP=16;
SGPRect		SaveFontDestRegion={0,0,640,480};
BOOLEAN		SaveFontDestWrap=FALSE;
UINT16		SaveFontForeground16=0;
UINT16		SaveFontShadow16=0;
UINT16		SaveFontBackground16=0;
UINT8			SaveFontForeground8=0;
UINT8			SaveFontBackground8=0;

//*****************************************************************************
// SetFontColors
//
//	Sets both the foreground and the background colors of the current font. The
// top byte of the parameter word is the background color, and the bottom byte
// is the foreground.
//
//*****************************************************************************
void SetFontColors(UINT16 usColors)
{
UINT8 ubForeground, ubBackground;

	ubForeground=(UINT8)(usColors&0xff);
	ubBackground=(UINT8)((usColors&0xff00)>>8);

	SetFontForeground(ubForeground);
	SetFontBackground(ubBackground);

}

//*****************************************************************************
// SetFontForeground
//
//	Sets the foreground color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. Note
// that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontForeground(UINT8 ubForeground)
{
UINT32 uiRed, uiGreen, uiBlue;

	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;

	FontForeground8=ubForeground;

	uiRed=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peRed;
	uiGreen=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peGreen;
	uiBlue=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peBlue;

	FontForeground16=Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));

}

void SetFontShadow(UINT8 ubShadow )
{
UINT32 uiRed, uiGreen, uiBlue;

	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;

	//FontForeground8=ubForeground;

	uiRed=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peRed;
	uiGreen=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peGreen;
	uiBlue=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peBlue;

	FontShadow16=Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));

	if ( ubShadow != 0 )
	{
		if ( FontShadow16 == 0 )
		{
			FontShadow16 = 1;
		}
	}

}

//*****************************************************************************
// SetFontBackground
//
//	Sets the Background color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. If the
// background value is zero, the background of the font will be transparent.
// Note that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontBackground(UINT8 ubBackground)
{
UINT32 uiRed, uiGreen, uiBlue;

	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;

	FontBackground8=ubBackground;

	uiRed=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peRed;
	uiGreen=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peGreen;
	uiBlue=(UINT32)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peBlue;

	FontBackground16=Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}


//Kris:  These are new counterparts to the above functions.  They won't
//			 effect an 8BPP font, only 16.
void SetRGBFontForeground( UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue )
{
	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;
	FontForeground16 = Get16BPPColor( FROMRGB( uiRed, uiGreen, uiBlue ) );
}

void SetRGBFontBackground( UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue )
{
	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;
	FontBackground16 = Get16BPPColor( FROMRGB( uiRed, uiGreen, uiBlue ) );
}

void SetRGBFontShadow( UINT32 uiRed, UINT32 uiGreen, UINT32 uiBlue )
{
	if((FontDefault < 0) || (FontDefault > MAX_FONTS))
		return;
	FontShadow16 = Get16BPPColor( FROMRGB( uiRed, uiGreen, uiBlue ) );
}
//end Kris

//*****************************************************************************
// ResetFontObjectPalette
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the HVOBJECT).
//
//*****************************************************************************
BOOLEAN ResetFontObjectPalette(INT32 iFont)
{
	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] !=NULL);

	SetFontObjectPalette8BPP(iFont, FontObjs[iFont]->pPaletteEntry);

	return(TRUE);
}


//*****************************************************************************
// SetFontObjectPalette8BPP
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the HVOBJECT).
//
//*****************************************************************************
UINT16 *SetFontObjectPalette8BPP(INT32 iFont, SGPPaletteEntry *pPal8)
{
UINT16 *pPal16;

	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] !=NULL);

	if((pPal16=Create16BPPPalette(pPal8))==NULL)
		return(NULL);

	FontObjs[iFont]->p16BPPPalette=pPal16;
	FontObjs[iFont]->pShadeCurrent=pPal16;

	return(pPal16);
}

//*****************************************************************************
// SetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
UINT16 *SetFontObjectPalette16BPP(INT32 iFont, UINT16 *pPal16)
{
	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] !=NULL);

	FontObjs[iFont]->p16BPPPalette=pPal16;
	FontObjs[iFont]->pShadeCurrent=pPal16;

	return(pPal16);

}

//*****************************************************************************
// GetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
UINT16 *GetFontObjectPalette16BPP(INT32 iFont)
{
	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] !=NULL);

	return(FontObjs[iFont]->p16BPPPalette);
}

//*****************************************************************************
// GetFontObject
//
//	Returns the VOBJECT pointer of a font.
//
//*****************************************************************************
HVOBJECT GetFontObject(INT32 iFont)
{
	Assert(iFont >= 0);
	Assert(iFont <= MAX_FONTS);
	Assert(FontObjs[iFont] !=NULL);

	return(FontObjs[iFont]);
}

//*****************************************************************************
// FindFreeFont
//
//	Locates an empty slot in the font table.
//
//*****************************************************************************
INT32 FindFreeFont(void)
{
int count;

	for(count=0; count < MAX_FONTS; count++)
		if(FontObjs[count]==NULL)
			return(count);

	return(-1);

}

//*****************************************************************************
// LoadFontFile
//
//	Loads a font from an ETRLE file, and inserts it into one of the font slots.
//  This function returns (-1) if it fails, and debug msgs for a reason.
//  Otherwise the font number is returned.
//*****************************************************************************
INT32 LoadFontFile(UINT8 *filename)
{
VOBJECT_DESC		vo_desc;
UINT32					LoadIndex;

	Assert(filename!=NULL);
	Assert(strlen(filename));

	if((LoadIndex=FindFreeFont())==(-1))
	{
		  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Out of font slots (%s)", filename));
#ifdef JA2
			FatalError( "Cannot init FONT file %s", filename );
#endif
			return(-1);
	}

	vo_desc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
	strcpy(vo_desc.ImageFile, filename);

	if((FontObjs[LoadIndex]=CreateVideoObject(&vo_desc))==NULL)
	{
		  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error creating VOBJECT (%s)", filename));
#ifdef JA2
			FatalError( "Cannot init FONT file %s", filename );
#endif
			return(-1);
	}

	if(FontDefault==(-1))
		FontDefault=LoadIndex;

	return(LoadIndex);
}

//*****************************************************************************
// UnloadFont - Delete the font structure
//
//	Deletes the video object of a particular font. Frees up the memory and
// resources allocated for it.
//
//*****************************************************************************
void UnloadFont(UINT32 FontIndex)
{
	Assert(FontIndex >= 0);
	Assert(FontIndex <= MAX_FONTS);
	Assert(FontObjs[FontIndex]!=NULL);

	DeleteVideoObject(FontObjs[FontIndex]);
	FontObjs[FontIndex]=NULL;
}

//*****************************************************************************
// GetWidth
//
//	Returns the width of a given character in the font.
//
//*****************************************************************************
UINT32 GetWidth(HVOBJECT hSrcVObject, INT16 ssIndex)
{
  ETRLEObject *pTrav;

	// Assertions
	Assert( hSrcVObject != NULL );

	if ( ssIndex < 0 || ssIndex > 92 )
	{
		int i=0;
	}

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ ssIndex ] );
	return((UINT32)(pTrav->usWidth+pTrav->sOffsetX));
}

//*****************************************************************************
// StringPixLengthArg
//
//		Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.
//    'uiCharCount' specifies how many characters of the string are counted.
//*****************************************************************************
INT16 StringPixLengthArg(INT32 usUseFont, UINT32 uiCharCount, UINT16 *pFontString, ...)
{
va_list argptr;
wchar_t	string[512];

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
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

//*****************************************************************************
// StringPixLengthArg
//
// Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.  Because this is for fast help text, all '|' characters are ignored for the
// width calculation.
// 'uiCharCount' specifies how many characters of the string are counted.
// YOU HAVE TO PREBUILD THE FAST HELP STRING!
//*****************************************************************************
INT16 StringPixLengthArgFastHelp(INT32 usUseFont, INT32 usBoldFont, UINT32 uiCharCount, UINT16 *pFontString )
{
	wchar_t	string[512];
	UINT32 i, index;
	INT16 sBoldDiff = 0;
	UINT16 str[2];

	Assert(pFontString!=NULL);

	wcscpy( string, pFontString );

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
	i = 0;
	while( i < uiCharCount )
	{
		if( string[ i ] == '|' )
		{
			for( index = i; index < uiCharCount; index++ )
			{
				string[ index ] = string[ index + 1 ];
			}
			uiCharCount--;
			//now we have eliminated the '|' character, so now calculate the size difference of the
			//bolded character.
			str[ 0 ] = string[ i ];
			str[ 1 ] = 0;
			sBoldDiff += StringPixLength( str, usBoldFont ) - StringPixLength( str, usUseFont );
		}
		i++;
	}
	return StringPixLength(string, usUseFont) + sBoldDiff;
}



//*****************************************************************************************
//
//  StringNPixLength
//
//  Return the length of the of the string or count characters in the
//  string, which ever comes first.
//
//  Returns INT16
//
//  Created by:     Gilles Beauparlant
//  Created on:     12/1/99
//
//*****************************************************************************************
INT16 StringNPixLength(UINT16 *string, UINT32 uiMaxCount, INT32 UseFont)
{
	UINT32 Cur, uiCharCount;
	UINT16 *curletter,transletter;

	Cur = 0;
	uiCharCount = 0;
	curletter = string;

	while((*curletter) != L'\0' && uiCharCount < uiMaxCount )
	{
		transletter=GetIndex(*curletter++);
		Cur+=GetWidth(FontObjs[UseFont], transletter);
		uiCharCount++;
	}
	return((INT16)Cur);
}

//*****************************************************************************
//
// StringPixLength
//
//	Returns the length of a string in pixels, depending on the font given.
//
//*****************************************************************************
INT16 StringPixLength(UINT16 *string, INT32 UseFont)
{
	UINT32 Cur;
	UINT16 *curletter,transletter;

	if (string == NULL)
	{
		return(0);
	}

	Cur=0;
	curletter=string;

	while((*curletter) != L'\0')
	{
		transletter=GetIndex(*curletter++);
		Cur+=GetWidth(FontObjs[UseFont], transletter);
	}
	return((INT16)Cur);
}


//*****************************************************************************
//
// SaveFontSettings
//
//	Saves the current font printing settings into temporary locations.
//
//*****************************************************************************
void SaveFontSettings(void)
{
	SaveFontDefault=FontDefault;
	SaveFontDestBuffer=FontDestBuffer;
	SaveFontDestPitch=FontDestPitch;
	SaveFontDestBPP=FontDestBPP;
	SaveFontDestRegion=FontDestRegion;
	SaveFontDestWrap=FontDestWrap;
	SaveFontForeground16 = FontForeground16;
	SaveFontShadow16 = FontShadow16;
	SaveFontBackground16 = FontBackground16;
	SaveFontForeground8 = FontForeground8;
	SaveFontBackground8 = FontBackground8;
}


//*****************************************************************************
//
// RestoreFontSettings
//
//	Restores the last saved font printing settings from the temporary lactions
//
//*****************************************************************************
void RestoreFontSettings(void)
{
	FontDefault=SaveFontDefault;
	FontDestBuffer=SaveFontDestBuffer;
	FontDestPitch=SaveFontDestPitch;
	FontDestBPP=SaveFontDestBPP;
	FontDestRegion=SaveFontDestRegion;
	FontDestWrap=SaveFontDestWrap;
	FontForeground16 = SaveFontForeground16;
	FontShadow16 = SaveFontShadow16;
	FontBackground16 = SaveFontBackground16;
	FontForeground8 = SaveFontForeground8;
	FontBackground8 = SaveFontBackground8;
}


//*****************************************************************************
// GetHeight
//
//	Returns the height of a given character in the font.
//
//*****************************************************************************
UINT32 GetHeight(HVOBJECT hSrcVObject, INT16 ssIndex)
{
  ETRLEObject *pTrav;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	pTrav = &(hSrcVObject->pETRLEObject[ ssIndex ] );
	return((UINT32)(pTrav->usHeight+pTrav->sOffsetY));
}

//*****************************************************************************
//
// GetFontHeight
//
//	Returns the height of the first character in a font.
//
//*****************************************************************************
UINT16 GetFontHeight(INT32 FontNum)
{
	Assert(FontNum >= 0);
	Assert(FontNum <= MAX_FONTS);
	Assert(FontObjs[FontNum]!=NULL);

	return((UINT16)GetHeight(FontObjs[FontNum], 0));
}

//*****************************************************************************
// GetIndex
//
//		Given a word-sized character, this function returns the index of the
//	cell in the font to print to the screen. The conversion table is built by
//	CreateEnglishTransTable()
//
//*****************************************************************************
INT16 GetIndex(UINT16 siChar)
{
	UINT16 *pTrav;
	UINT16 ssCount=0;
	UINT16	usNumberOfSymbols = pFManager->pTranslationTable->usNumberOfSymbols;

	// search the Translation Table and return the index for the font
	pTrav = pFManager->pTranslationTable->DynamicArrayOf16BitValues;
	while (ssCount < usNumberOfSymbols )
	{
		if (siChar == *pTrav)
		{
      return ssCount;
    }
		ssCount++;
		pTrav++;
	}

	// If here, present warning and give the first index
	DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error: Invalid character given %d", siChar));

	// Return 0 here, NOT -1 - we should see A's here now...
	return 0;
}


//*****************************************************************************
// SetFont
//
//	Sets the current font number.
//
//*****************************************************************************
BOOLEAN SetFont(INT32 iFontIndex)
{
	Assert(iFontIndex >= 0);
	Assert(iFontIndex <= MAX_FONTS);
	Assert(FontObjs[iFontIndex]!=NULL);

	FontDefault=iFontIndex;
	return(TRUE);
}

//*****************************************************************************
// SetFontDestBuffer
//
//	Sets the destination buffer for printing to, the clipping rectangle, and
// sets the line wrap on/off. DestBuffer is a VOBJECT handle, not a pointer.
//
//*****************************************************************************
BOOLEAN SetFontDestBuffer(UINT32 DestBuffer, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOLEAN wrap)
{
	Assert(x2 > x1);
	Assert(y2 > y1);

	FontDestBuffer=DestBuffer;

	FontDestRegion.iLeft=x1;
	FontDestRegion.iTop=y1;
	FontDestRegion.iRight=x2;
	FontDestRegion.iBottom=y2;
	FontDestWrap=wrap;

	return(TRUE);
}

//*****************************************************************************
// mprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters. Uses monochrome font color settings
//*****************************************************************************
UINT32 mprintf(INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];
UINT32			uiDestPitchBYTES;
UINT8				*pDestBuf;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	// Lock the dest buffer
	pDestBuf = LockVideoSurface( FontDestBuffer, &uiDestPitchBYTES );

	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter++);

		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground8, FontBackground8);
		}
		else
		{
			Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16 );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	// Unlock buffer
	UnLockVideoSurface( FontDestBuffer );

	return(0);
}


void VarFindFontRightCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, UINT16 *pFontString, ... )
{
	wchar_t	string[512];
	va_list argptr;

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	FindFontRightCoordinates( sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY );
}

void VarFindFontCenterCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY, UINT16 *pFontString, ... )
{
	wchar_t	string[512];
	va_list argptr;

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	FindFontCenterCoordinates( sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY );
}


void FindFontRightCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, UINT16 *pStr, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY )
{
	INT16 xp,yp;

	// Compute the coordinates to right justify the text
	xp = ((sWidth-StringPixLength(pStr,iFontIndex))) + sLeft;
	yp = ((sHeight-GetFontHeight(iFontIndex)) / 2) + sTop;

	*psNewX = xp;
	*psNewY = yp;
}

void FindFontCenterCoordinates( INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, UINT16 *pStr, INT32 iFontIndex, INT16 *psNewX, INT16 *psNewY )
{
	INT16 xp,yp;

	// Compute the coordinates to center the text
	xp = ((sWidth-StringPixLength(pStr,iFontIndex) + 1) / 2) + sLeft;
	yp = ((sHeight-GetFontHeight(iFontIndex)) / 2) + sTop;

	*psNewX = xp;
	*psNewY = yp;
}

//*****************************************************************************
// gprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
UINT32 gprintf(INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];
UINT32			uiDestPitchBYTES;
UINT8				*pDestBuf;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	// Lock the dest buffer
	pDestBuf = LockVideoSurface( FontDestBuffer, &uiDestPitchBYTES );

	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter++);

		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}
		else
		{
			Blt8BPPDataTo16BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	// Unlock buffer
	UnLockVideoSurface( FontDestBuffer );

	return(0);
}

UINT32 gprintfDirty(INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];
UINT32			uiDestPitchBYTES;
UINT8				*pDestBuf;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	// Lock the dest buffer
	pDestBuf = LockVideoSurface( FontDestBuffer, &uiDestPitchBYTES );

	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter++);

		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}
		else
		{
			Blt8BPPDataTo16BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	// Unlock buffer
	UnLockVideoSurface( FontDestBuffer );

#if defined ( JA2 ) || defined( UTIL )
	InvalidateRegion(x, y,
										x + StringPixLength(string, FontDefault),
										y + GetFontHeight(FontDefault));
#else
	InvalidateRegion(x, y,
										x + StringPixLength(string, FontDefault),
										y + GetFontHeight(FontDefault),
										INVAL_SRC_TRANS);
#endif

	return(0);
}
//*****************************************************************************
// gprintf_buffer
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
UINT32 gprintf_buffer( UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter++);

		if(FontDestWrap && BltIsClipped(FontObjs[FontType], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontType], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}
		else
		{
			Blt8BPPDataTo16BPPBufferTransparentClip( (UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion );
		}

		destx+=GetWidth(FontObjs[FontType], transletter);
	}

	return(0);
}


UINT32 mprintf_buffer( UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter++);

		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground8, FontBackground8);
		}
		else
		{
			Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16 );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	return(0);
}


UINT32 mprintf_buffer_coded( UINT8 *pDestBuf, UINT32 uiDestPitchBYTES, UINT32 FontType, INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];
UINT16	usOldForeColor;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	usOldForeColor = FontForeground16;

	while((*curletter)!=0)
	{
		if ( (*curletter) == 180 )
		{
			curletter++;
			SetFontForeground( (UINT8)(*curletter) );
			curletter++;
		}
		else if ( (*curletter) == 181 )
		{
			FontForeground16 = usOldForeColor;
			curletter++;
		}

		transletter=GetIndex(*curletter++);


		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground8, FontBackground8);
		}
		else
		{
			Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16 );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	return(0);
}


UINT32 mprintf_coded( INT32 x, INT32 y, UINT16 *pFontString, ...)
{
INT32		destx, desty;
UINT16	*curletter, transletter;
va_list argptr;
wchar_t	string[512];
UINT16	usOldForeColor;
UINT32			uiDestPitchBYTES;
UINT8				*pDestBuf;

	Assert(pFontString!=NULL);

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

	curletter=string;

	destx=x;
	desty=y;

	usOldForeColor = FontForeground16;

	// Lock the dest buffer
	pDestBuf = LockVideoSurface( FontDestBuffer, &uiDestPitchBYTES );

	while((*curletter)!=0)
	{
		if ( (*curletter) == 180 )
		{
			curletter++;
			SetFontForeground( (UINT8)(*curletter) );
			curletter++;
		}
		else if ( (*curletter) == 181 )
		{
			FontForeground16 = usOldForeColor;
			curletter++;
		}

		transletter=GetIndex(*curletter++);


		if(FontDestWrap && BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion))
		{
			destx=x;
			desty+=GetHeight(FontObjs[FontDefault], transletter);
		}

		// Blit directly
		if ( gbPixelDepth == 8 )
		{
			Blt8BPPDataTo8BPPBufferMonoShadowClip(pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground8, FontBackground8);
		}
		else
		{
			Blt8BPPDataTo16BPPBufferMonoShadowClip((UINT16*)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion, FontForeground16, FontBackground16, FontShadow16 );
		}
		destx+=GetWidth(FontObjs[FontDefault], transletter);
	}

	// Unlock buffer
	UnLockVideoSurface( FontDestBuffer );

	return(0);
}



//*****************************************************************************
// InitializeFontManager
//
//	Starts up the font manager system with the appropriate translation table.
//
//*****************************************************************************
BOOLEAN InitializeFontManager(UINT16 usDefaultPixelDepth, FontTranslationTable *pTransTable)
{
FontTranslationTable *pTransTab;
int count;
UINT16 uiRight, uiBottom;
UINT8 uiPixelDepth;

	FontDefault=(-1);
	FontDestBuffer=BACKBUFFER;
	FontDestPitch=0;

	//	FontDestBPP=0;

	GetCurrentVideoSettings( &uiRight, &uiBottom, &uiPixelDepth );
	FontDestRegion.iLeft = 0;
	FontDestRegion.iTop = 0;
	FontDestRegion.iRight=(INT32)uiRight;
	FontDestRegion.iBottom=(INT32)uiBottom;
	FontDestBPP=(UINT32)uiPixelDepth;


	FontDestWrap=FALSE;

	// register the appropriate debug topics
	if(pTransTable == NULL)
	{
    return FALSE;
  }
	RegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");

	if ((pFManager = (FontManager *)MemAlloc(sizeof(FontManager)))==NULL)
	{
    return FALSE;
  }

	if((pTransTab = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable)))==NULL)
	{
    return FALSE;
  }

	pFManager->pTranslationTable = pTransTab;
	pFManager->usDefaultPixelDepth = usDefaultPixelDepth;
	pTransTab->usNumberOfSymbols = pTransTable->usNumberOfSymbols;
  pTransTab->DynamicArrayOf16BitValues = pTransTable->DynamicArrayOf16BitValues;

	// Mark all font slots as empty
	for(count=0; count < MAX_FONTS; count++)
		FontObjs[count]=NULL;

	return TRUE;
}


//*****************************************************************************
// ShutdownFontManager
//
//	Shuts down, and deallocates all fonts.
//*****************************************************************************
void ShutdownFontManager(void)
{
  INT32 count;

	UnRegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");
  if(pFManager)
		MemFree(pFManager);

	for(count=0; count < MAX_FONTS; count++)
	{
		if(FontObjs[count]!=NULL)
			UnloadFont(count);
	}
}


//*****************************************************************************
// DestroyEnglishTransTable
//
// Destroys the English text->font map table.
//*****************************************************************************
void DestroyEnglishTransTable( void )
{
	if(pFManager)
	{
		if (pFManager->pTranslationTable != NULL)
		{
			if (pFManager->pTranslationTable->DynamicArrayOf16BitValues != NULL)
			{
				MemFree( pFManager->pTranslationTable->DynamicArrayOf16BitValues );
			}

			MemFree( pFManager->pTranslationTable );

			pFManager->pTranslationTable = NULL;
		}
	}
}


//*****************************************************************************
// CreateEnglishTransTable
//
// Creates the English text->font map table.
//*****************************************************************************
FontTranslationTable *CreateEnglishTransTable(  )
{
	FontTranslationTable *pTable = NULL;
	UINT16	*temp;

  pTable = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable));
	#ifdef JA2
		// ha ha, we have more than Wizardry now (again)
		pTable->usNumberOfSymbols = 172;
	#else
		pTable->usNumberOfSymbols = 155;
	#endif
	pTable->DynamicArrayOf16BitValues = (UINT16 *)MemAlloc(pTable->usNumberOfSymbols * 2);
	temp = pTable->DynamicArrayOf16BitValues;

	*temp = 'A';
	temp++;
	*temp='B';
	temp++;
	*temp ='C';
	temp++;
	*temp ='D';
	temp++;
	*temp = 'E';
	temp++;
	*temp = 'F';
	temp++;
	*temp='G';
	temp++;
	*temp ='H';
	temp++;
	*temp ='I';
	temp++;
	*temp = 'J';
	temp++;
	*temp = 'K';
	temp++;
	*temp='L';
	temp++;
	*temp ='M';
	temp++;
	*temp ='N';
	temp++;
	*temp = 'O';
	temp++;
	*temp = 'P';
	temp++;
	*temp='Q';
	temp++;
	*temp ='R';
	temp++;
	*temp ='S';
	temp++;
	*temp = 'T';
	temp++;
	*temp ='U';
	temp++;
	*temp ='V';
	temp++;
	*temp = 'W';
	temp++;
	*temp = 'X';
	temp++;
	*temp='Y';
	temp++;
	*temp ='Z';
	temp++;
	*temp ='a';
	temp++;
	*temp = 'b';
	temp++;
	*temp ='c';
	temp++;
	*temp = 'd';
	temp++;
	*temp ='e';
	temp++;
	*temp = 'f';
	temp++;
	*temp ='g';
	temp++;
	*temp = 'h';
	temp++;
	*temp ='i';
	temp++;
	*temp = 'j';
	temp++;
	*temp ='k';
	temp++;
	*temp = 'l';
	temp++;
	*temp ='m';
	temp++;
	*temp = 'n';
	temp++;
	*temp ='o';
	temp++;
	*temp = 'p';
	temp++;
	*temp ='q';
	temp++;
	*temp = 'r';
	temp++;
	*temp ='s';
	temp++;
	*temp = 't';
	temp++;
	*temp ='u';
	temp++;
	*temp = 'v';
	temp++;
	*temp ='w';
	temp++;
	*temp = 'x';
	temp++;
	*temp ='y';
	temp++;
	*temp = 'z';
	temp++;
	*temp ='0';
	temp++;
	*temp = '1';
	temp++;
	*temp ='2';
	temp++;
	*temp = '3';
	temp++;
	*temp ='4';
	temp++;
	*temp = '5';
	temp++;
	*temp ='6';
	temp++;
	*temp = '7';
	temp++;
	*temp ='8';
	temp++;
	*temp = '9';
	temp++;
	*temp ='!';
	temp++;
	*temp = '@';
	temp++;
	*temp ='#';
	temp++;
	*temp = '$';
	temp++;
	*temp ='%';
	temp++;
	*temp = '^';
	temp++;
	*temp ='&';
	temp++;
	*temp = '*';
	temp++;
	*temp ='(';
	temp++;
	*temp = ')';
	temp++;
	*temp ='-';
	temp++;
	*temp = '_';
	temp++;
	*temp ='+';
	temp++;
	*temp = '=';
	temp++;
	*temp ='|';
	temp++;
	*temp = '\\';
	temp++;
	*temp ='{';
	temp++;
	*temp = '}';// 80
	temp++;
	*temp = '[';
	temp++;
	*temp =']';
	temp++;
	*temp = ':';
	temp++;
	*temp =';';
	temp++;
	*temp = '"';
	temp++;
	*temp ='\'';
	temp++;
	*temp = '<';
	temp++;
	*temp = '>';
	temp++;
	*temp =',';
	temp++;
	*temp = '.';
	temp++;
	*temp ='?';
	temp++;
	*temp = '/';
	temp++;
	*temp = ' '; //93
	temp++;

#ifdef JA2
	*temp = 196; // "A" umlaut
	temp++;
	*temp = 214; // "O" umlaut
	temp++;
	*temp = 220; // "U" umlaut
	temp++;
	*temp = 228; // "a" umlaut
	temp++;
	*temp = 246; // "o" umlaut
	temp++;
	*temp = 252; // "u" umlaut
	temp++;
	*temp = 223; // double-s that looks like a beta/B  // 100
	temp++;
	// START OF FUNKY RUSSIAN STUFF
	*temp = 1101;
	temp++;
	*temp = 1102;
	temp++;
	*temp = 1103;
	temp++;
	*temp = 1104;
	temp++;
	*temp = 1105;
	temp++;
	*temp = 1106;
	temp++;
	*temp = 1107;
	temp++;
	*temp = 1108;
	temp++;
	*temp = 1109;
	temp++;
	*temp = 1110;
	temp++;
	*temp = 1111;
	temp++;
	*temp = 1112;
	temp++;
	*temp = 1113;
	temp++;
	*temp = 1114;
	temp++;
	*temp = 1115;
	temp++;
	*temp = 1116;
	temp++;
	*temp = 1117;
	temp++;
	*temp = 1118;
	temp++;
	*temp = 1119;
	temp++;
	*temp = 1120;
	temp++;
	*temp = 1121;
	temp++;
	*temp = 1122;
	temp++;
	*temp = 1123;
	temp++;
	*temp = 1124;
	temp++;
	*temp = 1125;
	temp++;
	*temp = 1126;
	temp++;
	*temp = 1127;
	temp++;
	*temp = 1128;
	temp++;
	*temp = 1129;
	temp++;
	*temp = 1130; // 130
	temp++;
	*temp = 1131;
	temp++;
	*temp = 1132;
	temp++;
	// END OF FUNKY RUSSIAN STUFF
	*temp = 196; // Ä
	temp++;
	*temp = 192; // À
	temp++;
	*temp = 193; // Á
	temp++;
	*temp = 194; // Â
	temp++;
	*temp = 199; // Ç
	temp++;
	*temp = 203; // Ë
	temp++;
	*temp = 200; // È
	temp++;
	*temp = 201; // É				140
	temp++;
	*temp = 202; // Ê
	temp++;
	*temp = 207; // Ï
	temp++;
	*temp = 214; // Ö
	temp++;
	*temp = 210; // Ò
	temp++;
	*temp = 211; // Ó
	temp++;
	*temp = 212; // Ô
	temp++;
	*temp = 220; // Ü
	temp++;
	*temp = 217; // Ù
	temp++;
	*temp = 218; // Ú
	temp++;
	*temp = 219; // Û				150
	temp++;

	*temp = 228; // ä
	temp++;
	*temp = 224; // à
	temp++;
	*temp = 225; // á
	temp++;
	*temp = 226; // â
	temp++;
	*temp = 231; // ç
	temp++;
	*temp = 235; // ë
	temp++;
	*temp = 232; // è
	temp++;
	*temp = 233; // é
	temp++;
	*temp = 234; // ê
	temp++;
	*temp = 239; // ï				160
	temp++;
	*temp = 246; // ö
	temp++;
	*temp = 242; // ò
	temp++;
	*temp = 243; // ó
	temp++;
	*temp = 244; // ô
	temp++;
	*temp = 252; // ü
	temp++;
	*temp = 249; // ù
	temp++;
	*temp = 250; // ú
	temp++;
	*temp = 251; // û
	temp++;
	*temp = 204; // Ì
	temp++;
	*temp = 206; // Î				170
	temp++;
	*temp = 236; // ì
	temp++;
	*temp = 238; // î
	temp++;

#else
// Windows Code Page 1252 Western Standard Character Set

	*temp = 193;	// "A" acute
	temp++;
	*temp = 192;	// "A" grave
	temp++;
	*temp = 193;	// "A" circumflex
	temp++;
	*temp = 196;	// "A" umlaut
	temp++;
	*temp = 195;	// "A" tilde
	temp++;
	*temp = 197;	// "A" ring
	temp++;
	*temp = 199;	// "C" cedile
	temp++;
	*temp = 201;	// "E" acute
	temp++;
	*temp = 200;	// "E" grave
	temp++;
	*temp = 202;	// "E" circumflex
	temp++;
	*temp = 203;	// "E" umlaut
	temp++;
	*temp = 205; // "I" acute
	temp++;
	*temp = 204;	// "I" grave
	temp++;
	*temp = 206;	// "I" circumflex
	temp++;
	*temp = 207;	// "I" umlaut
	temp++;
	*temp = 209;	// "N" tilde
	temp++;
	*temp = 211;	// "O" acute
	temp++;
	*temp = 210;	// "O" grave
	temp++;
	*temp = 212;	// "O" circumflex
	temp++;
	*temp = 214;	// "O" umlaut
	temp++;
	*temp = 213;	// "O" tilde
	temp++;
	*temp = 216;	// "0" O strike-through
	temp++;
	*temp = 218;	// "U" acute
	temp++;
	*temp = 217;	// "U" grave
	temp++;
	*temp = 219;	// "U" circumflex
	temp++;
	*temp = 220;	// "U" umlaut
	temp++;
	*temp = 221;	// "Y" acute
	temp++;
	*temp = 225;	// "a" acute
	temp++;
	*temp = 224;	// "a" grave
	temp++;
	*temp = 226;	// "a" circumflex
	temp++;
	*temp = 228;	// "a" umlaut
	temp++;
	*temp = 227;	// "a" tilde
	temp++;
	*temp = 229;	// "a" ring
	temp++;
	*temp = 231;	// "c" cedile
	temp++;
	*temp = 233;	// "e" acute
	temp++;
	*temp = 232;	// "e" grave
	temp++;
	*temp = 234;	// "e" circumflex
	temp++;
	*temp = 235;	// "e" umlaut
	temp++;
	*temp = 237;	// "i" acute
	temp++;
	*temp = 236;	// "i" grave
	temp++;
	*temp = 238;	// "i" circumflex
	temp++;
	*temp = 239;	// "i" umlaut
	temp++;
	*temp = 241;	// "n" tilde
	temp++;
	*temp = 243;	// "o" acute
	temp++;
	*temp = 242;	// "o" grave
	temp++;
	*temp = 244;	// "o" circumflex
	temp++;
	*temp = 246;	// "o" umlaut
	temp++;
	*temp = 245;	// "o" tilde
	temp++;
	*temp = 248;	// "o" strike-through
	temp++;
	*temp = 250;	// "u" acute
	temp++;
	*temp = 249;	// "u" grave
	temp++;
	*temp = 251;	// "u" circumflex
	temp++;
	*temp = 252;	// "u" umlaut
	temp++;
	*temp = 254;	// "y" acute
	temp++;
	*temp = 255;	// "y" umlaut
	temp++;
	*temp = 223;	// beta

// Font glyphs for spell targeting icons
	//ATE: IMPORTANT! INcreate the array above if you add any new items here...
	temp++;
	*temp = FONT_GLYPH_TARGET_POINT;
	temp++;
	*temp = FONT_GLYPH_TARGET_CONE;
	temp++;
	*temp = FONT_GLYPH_TARGET_SINGLE;
	temp++;
	*temp = FONT_GLYPH_TARGET_GROUP;
	temp++;
	*temp = FONT_GLYPH_TARGET_NONE;

// 154
#endif

	   return pTable;
}

//*****************************************************************************
//
// LoadFontFile
//
// Parameter List : filename - File created by the utility tool to open
//
// Return Value  pointer to the base structure
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*FontBase *LoadFontFile(UINT8 *pFilename)
{
  HWFILE           hFileHandle;
  UINT32           uiFileSize;
  UINT32           uiHeightEach;
  UINT32           uiTotalSymbol;
  UINT32           uiNewoffst, uiOldoffst;
  FontBase        *pFontBase;
  SGPPaletteEntry *pNewPalette;
  UINT8           *pPalette;

  if (pFManager == NULL)
  {
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Did not Initialize Font Manager");
    return NULL;
  }

  // Open and read in the file
  if ((hFileHandle = FileOpen(pFilename, FILE_ACCESS_READ, FALSE)) == 0)
  { // damn we failed to open the file
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Cannot open font file");
    return NULL;
  }

  uiFileSize = FileGetSize(hFileHandle);
  if (uiFileSize == 0)
  { // we failed to size up the file
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Font file is empty");
    FileClose(hFileHandle);
    return NULL;
  }

  // Allocate memory for the font header file
  if ((pFontBase = (FontBase *)MemAlloc(sizeof(FontBase))) == NULL)
  {
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not malloc memory");
	  FileClose(hFileHandle);
  }

  // read in these values from the file
  if (FileRead(hFileHandle, &uiHeightEach, sizeof(UINT32), NULL) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not read Height from File");
	  FileClose(hFileHandle);
	  return NULL;
  }

  if (FileRead(hFileHandle, &uiTotalSymbol, sizeof(UINT32), NULL) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not read Total Symbol from File");
	  FileClose(hFileHandle);
	  return NULL;
  }

  // Assign the proper values to the Base structure
  pFontBase->uiHeightEach = uiHeightEach;
  pFontBase->uiTotalElements = uiTotalSymbol;
  pFontBase->pFontObject = (FontObject *)MemAlloc(uiTotalSymbol * sizeof(FontHeader));
  pPalette = (UINT8 *)MemAlloc(PALETTE_SIZE);
  uiOldoffst = (sizeof(FontHeader) + sizeof(FontObject)*pFontBase->uiTotalElements);
  uiNewoffst = uiFileSize - uiOldoffst;
  pFontBase->pPixData8 = (UINT8 *)MemAlloc(uiNewoffst);

  //seek past the FontHeader
  if (FileSeek(hFileHandle, sizeof(FontHeader), FILE_SEEK_FROM_START) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not seek FileHeader");
	  FileClose(hFileHandle);
	  return NULL;
  }

  //read in the FontObject
  if (FileRead(hFileHandle, pFontBase->pFontObject, (uiTotalSymbol)*sizeof(FontHeader), NULL) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not seek Font Objects");
	  FileClose(hFileHandle);
	  return NULL;
  }

  if (FileSeek(hFileHandle, uiOldoffst, FILE_SEEK_FROM_START) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not seek Old offset");
	  FileClose(hFileHandle);
	  return NULL;
  }

  // read in the Pixel data
  if (FileRead(hFileHandle, pFontBase->pPixData8, uiNewoffst, NULL) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not seek Pixel data");
	  FileClose(hFileHandle);
	  return NULL;
  }

  // seek proper position to read in Palette
  if (FileSeek(hFileHandle, sizeof(UINT32)*3, FILE_SEEK_FROM_START) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not seek Palette Start");
	  FileClose(hFileHandle);
	  return NULL;
  }

  // read in Palette
  if (FileRead(hFileHandle, pPalette, PALETTE_SIZE, NULL) == FALSE)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not read Palette");
	  FileClose(hFileHandle);
	  return NULL;
  }

  // set the default pixel depth
  pFontBase->siPixelDepth = pFManager->usDefaultPixelDepth;
  FileClose(hFileHandle);

  // convert from RGB to SGPPaletteEntry
  pNewPalette = ConvertToPaletteEntry(0, 255, pPalette);
  pFontBase->pPalette = pNewPalette;

  // create the 16BPer Pixel palette
  if ((pFontBase->pPalet16 = Create16BPPPalette(pNewPalette)) == NULL)
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Could not create 16 bit palette");
	  return NULL;
  }
  // return the FontBase structure
  return pFontBase;
}	*/



/*void UnloadFont(FontBase *pFontBase)
{
	// free allocated memory in FontBase
	if(pFontBase!=NULL)
	{
		if(pFontBase->pPalette!=NULL)
			MemFree(pFontBase->pPalette);
		if(pFontBase->pPalet16!=NULL)
			MemFree(pFontBase->pPalet16);
		if(pFontBase->pFontObject!=NULL)
			MemFree(pFontBase->pFontObject);
		if(pFontBase->pPixData8!=NULL)
			MemFree(pFontBase->pPixData8);
		if(pFontBase->pPixData16!=NULL)
			MemFree(pFontBase->pPixData16);
		MemFree(pFontBase);
	}
}	*/



//*****************************************************************************
//
// GetMaxFontWidth - Gets the maximum font width
//
// Parameter List : pointer to the base structure
//
// Return Value  Maximum font width
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*UINT16 GetMaxFontWidth(FontBase *pFontBase)
{
	FontObject *pWidth;
	UINT32 siBiggest = 0;
	UINT16 siCount;

  Assert(pFontBase != NULL);
	pWidth = pFontBase->pFontObject;
	// traverse the FontObject structure to find the biggest width
	for(siCount = 0; siCount < pFontBase->uiTotalElements; siCount++)
	{
		if( pWidth->uiFontWidth > siBiggest)
		{
      siBiggest = pWidth->uiFontWidth;
    }
		pWidth++;
	}
	// return the max width
	return (UINT16)siBiggest;
} */

//*****************************************************************************
//
// ConvertToPaletteEntry
//
// Parameter List : Converts from RGB to SGPPaletteEntry
//
// Return Value  pointer to the SGPPaletteEntry
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*
SGPPaletteEntry *ConvertToPaletteEntry(UINT8 sbStart, UINT8 sbEnd, UINT8 *pOldPalette)
{
	UINT16 Index;
  SGPPaletteEntry *pPalEntry;
	SGPPaletteEntry *pInitEntry;

	pPalEntry = (SGPPaletteEntry *)MemAlloc(sizeof(SGPPaletteEntry) * 256);
	pInitEntry = pPalEntry;
  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Converting RGB palette to SGPPaletteEntry");
  for(Index=0; Index <= (sbEnd-sbStart);Index++)
  {
    pPalEntry->peRed = *(pOldPalette + (Index*3));
	  pPalEntry->peGreen = *(pOldPalette + (Index*3) + 1);
 	  pPalEntry->peBlue = *(pOldPalette + (Index*3) + 2);
    pPalEntry->peFlags = 0;
	  pPalEntry++;
  }
  return pInitEntry;
} */

//*****************************************************************************
//
// SetFontPalette - Sets the Palette
//
// Parameter List : pointer to the base structure
//                  new pixel depth
//                  new Palette size
//                  pointer to palette data
//
// Return Value  BOOLEAN
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN SetFontPalette(FontBase *pFontBase, UINT16 siPixelDepth, SGPPaletteEntry *pPalData)
{
	Assert(pFontBase != NULL);
	Assert(pPalData != NULL);
	MemFree(pFontBase->pPalette);

	// assign the new palette to the Base structure
	pFontBase->pPalette = pPalData;
	pFontBase->siPixelDepth = siPixelDepth;
	return TRUE;
}	*/

//*****************************************************************************
//
// SetFont16BitData - Sets the font structure to hold 16 bit data
//
// Parameter List : pointer to the base structure
//                  pointer to new 16 bit data
//
// Return Value  BOOLEAN
//
// Modification History :
// Dec 15th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN SetFont16BitData(FontBase *pFontBase, UINT16 *pData16)
{
	Assert(pFontBase != NULL);
	Assert(pData16 != NULL);
	MemFree(pFontBase->pPixData16);
	pFontBase->pPixData16 = pData16;
	return TRUE;
}	*/



//*****************************************************************************
//
// Blt8Imageto16Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN Blt8Imageto16Dest(UINT32 uiOffStart, UINT32 uiOffEnd, UINT16 siX, UINT16 siY, UINT32 uiWidth, FontBase *pFontBase, UINT8 *pFrameBuffer, UINT16 siDestPitch, UINT16 siHeightEach)
{
	UINT8  *pTrav;
	UINT16 *pFrameTrav;
	UINT16 *p16BPPPalette;
	UINT16  usEffectiveWidth;
	UINT32  uiFrameCount;
	UINT8   amount;
	UINT32  row, count;
    UINT16  modamount, divamount;
	UINT32 trace,modtrace;
	UINT8 sub=0;


	pTrav = pFontBase->pPixData8;
	pFrameTrav = (UINT16 *)pFrameBuffer;
	p16BPPPalette = pFontBase->pPalet16;
    trace = 0;
	modtrace = 0;
	// effective width is pitch/2 as 16 bits per pixel
	usEffectiveWidth = (UINT16)(siDestPitch / 2);
	uiFrameCount = siY*usEffectiveWidth + siX;
	trace += uiFrameCount;
	modtrace = trace % 640;
	pFrameTrav += uiFrameCount;
	pTrav += uiOffStart;

	count = 0;
	row = 0;
	amount = 0;
	while (count < (uiOffEnd-uiOffStart))
	{
	  amount = 0;
    if (*pTrav == ID_BLACK)
	  {
		  pTrav++;
		  count++;
		  amount = *pTrav;
		  modamount = (UINT8)(amount) % (UINT8) uiWidth;
		  divamount = (UINT8)(amount) / (UINT8) uiWidth;
      if ((divamount == 0) && ((row+amount) < (UINT16)uiWidth))
		  {
			  pFrameTrav += amount;
			  trace += amount;
	          modtrace = trace % 640;
			  row += amount;
			  row++;
		  }
		  else
		  {
        if (((row+amount) >= (UINT16)uiWidth) && (divamount ==0))
		    {
                pFrameTrav -= row;
				trace -= row;
	            modtrace = trace % 640;
			    row = amount-((UINT16)uiWidth-row);
			    pFrameTrav += usEffectiveWidth+row;
				trace += usEffectiveWidth+row;
				modtrace = trace % 640;
			    row++;
		    }
		    else
		    {
			    pFrameTrav += (divamount*usEffectiveWidth);
				trace += (divamount*usEffectiveWidth);
				modtrace = trace % 640;
				if(row+modamount > uiWidth)
				{
					sub = (UINT8)((row+modamount) % uiWidth);
					pFrameTrav -= row;
			  	    trace -= row;
				    modtrace = trace % 640;
					pFrameTrav += usEffectiveWidth+sub;
				    trace += usEffectiveWidth + sub;
				    modtrace = trace % 640;
			        row = sub;
			        row++;
				}else
				{
					pFrameTrav += modamount;
				    trace += modamount;
				    modtrace = trace % 640;
			        row = modamount;
			        row++;
				}
		    }
      }
	  } else
	  {
		  if(row >= uiWidth)
		  {
            pFrameTrav += (usEffectiveWidth-uiWidth);
			trace += (usEffectiveWidth-uiWidth);
			modtrace = trace % 640;
            *pFrameTrav = p16BPPPalette[*pTrav];
		    row = 1;
		  }
		  else
		  {
            *pFrameTrav = p16BPPPalette[*pTrav];
		    row++;
		  }
    }

    pFrameTrav++;
	trace++;
	modtrace = trace % 640;
    pTrav++;
    count++;
	}

	return TRUE;
}	*/


//*****************************************************************************
//
// Blt8Imageto8Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN Blt8Imageto8Dest(UINT32 uiOffStart, UINT32 uiOffEnd, UINT16 siX, UINT16 siY, UINT32 uiWidth, FontBase *pFontBase, UINT8 *pFrameBuffer, UINT16 siDestPitch, UINT16 siHeightEach)
{
	UINT8  *pTrav;
	UINT32  uiFrameCount;
	UINT8  *pFrameTrav;
	UINT8   amount;
	UINT32  row,count;
  UINT16  modamount,divamount;

	DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Blitting 8 to 8");
  Assert(pFontBase != NULL);
	Assert(pFrameBuffer != NULL);

	// get the pointers
	pTrav = pFontBase->pPixData8;
	pFrameTrav = (UINT8 *)pFrameBuffer;

	uiFrameCount = siY*siDestPitch + siX;
	pFrameTrav +=uiFrameCount;
	pTrav += uiOffStart;
  // perform blitting

	count=0;
	row = 0;
	amount = 0;
	while (count < (uiOffEnd-uiOffStart))
	{
	  amount = 0;
    if (*pTrav == ID_BLACK)
	  {
		  pTrav++;
		  count++;
		  amount = *pTrav;
		  modamount = amount % (UINT8) uiWidth;
		  divamount = amount / (UINT8) uiWidth;
      if ((divamount == 0) && ((row+amount) < (UINT16)uiWidth))
		  {
			  pFrameTrav += amount;
			  row += amount;
			  row++;
		  }
		  else
		  {
        if (((row+amount) >= (UINT16)uiWidth) && (divamount ==0))
		    {
          pFrameTrav -= row;
			    row = amount-((UINT16)uiWidth-row);
			    pFrameTrav += siDestPitch+row;
			    row++;
		    }
		    else
		    {
			    pFrameTrav += (divamount*siDestPitch)+modamount;
			    row = modamount;
			    row++;
		    }
      }
	  } else
	  {
		  if (row >= uiWidth)
		  {
        pFrameTrav += (siDestPitch-uiWidth);
       *pFrameTrav = *pTrav;
		    row = 1;
		  }
		  else
		  {
       *pFrameTrav = *pTrav;
		    row++;
		  }
    }

    pFrameTrav++;
    pTrav++;
		count++;
	}

	return TRUE;
} */

//*****************************************************************************
//
// Blt16Imageto16Dest
//
// Parameter List : Start offset
//                  End Offset
//                  Dest x, y
//                  Font Width
//                  Pointer to Base structure
//                  Pointer to destination buffer
//                  Destination Pitch
//                  Height of Each element
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
/*BOOLEAN Blt16Imageto16Dest(UINT32 uiOffStart, UINT32 uiOffEnd, UINT16 siX, UINT16 siY, UINT32 uiWidth, FontBase *pFontBase, UINT8 *pFrameBuffer, UINT16 siDestPitch, UINT16 siHeightEach)
{
	UINT16 *pTrav;
	UINT32  uiFrameCount;
	UINT16 *pFrameTrav;
	UINT16  amount;
	UINT32  row,count;
  UINT16  modamount,divamount;
	UINT16  usEffectiveWidth;

	DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Blitting 16 to 16");
  Assert(pFontBase != NULL);
	Assert(pFrameBuffer != NULL);

	//get the pointers
	pTrav = pFontBase->pPixData16;
	pFrameTrav = (UINT16 *)pFrameBuffer;

	// effective width is pitch/2 as 16 bits per pixel
	usEffectiveWidth = (UINT16)(siDestPitch / 2);
	uiFrameCount = siY*usEffectiveWidth + siX;
	pFrameTrav +=uiFrameCount;
	pTrav += uiOffStart;

	count=0;
	row = 0;
	amount = 0;
	while (count < (uiOffEnd-uiOffStart))
	{
    amount = 0;
    if (*pTrav == ID_BLACK)
	  {
		  pTrav++;
		  count++;
		  amount = *pTrav;
		  modamount = amount % (UINT8) uiWidth;
		  divamount = amount / (UINT8) uiWidth;
      if ((divamount == 0) && ((row+amount) < (UINT16)uiWidth))
		  {
			  pFrameTrav += amount;
			  row += amount;
			  row++;
		  }
		  else
		  {
        if (((row+amount) >= (UINT16)uiWidth) && (divamount ==0))
		    {
          pFrameTrav -= row;
			    row = amount-((UINT16)uiWidth-row);
			    pFrameTrav += usEffectiveWidth+row;
			    row++;
		    }
		    else
		    {
			    pFrameTrav += (divamount*usEffectiveWidth)+modamount;
			    row = modamount;
			    row++;
		    }
      }
	  } else
	  {
		  if(row >= uiWidth)
		  {
        pFrameTrav += (usEffectiveWidth-uiWidth);
        *pFrameTrav = *pTrav;
		    row = 1;
		  }
		  else
		  {
        *pFrameTrav = *pTrav;
		    row++;
		  }
    }

    pFrameTrav++;
		pTrav++;
		count++;
	}

	return TRUE;
}	*/

//*****************************************************************************
//
// GetOffset
//
// Parameter List : Given the index, gets the corresponding offset
//
// Return Value  : offset
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*UINT32 GetOffset(FontBase *pFontBase, INT16 ssIndex)
{
  FontObject *pTrav;
  UINT16 siCount=0;

  Assert(pFontBase != NULL);
  // gets the offset based on the index
  if (((UINT32)ssIndex > pFontBase->uiTotalElements) || (ssIndex < 0))
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Incorrect index value passed");
    return 0;
  }
  pTrav = pFontBase->pFontObject;
  while (siCount != ssIndex)
  {
	  siCount++;
	  pTrav++;
  }

  return pTrav->uiFontOffset;
} */


//*****************************************************************************
//
// GetOffLen
//
// Parameter List : Given the index, gets the corresponding offset
// length which is the number of compressed pixels
//
// Return Value  : offset
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
/*UINT32 GetOffLen(FontBase *pFontBase, INT16 ssIndex)
{
  FontObject *pTrav;
  UINT16 siCount=0;

  Assert(pFontBase != NULL);
  // gets the offset based on the index
  if (((UINT32)ssIndex > pFontBase->uiTotalElements) || (ssIndex < 0))
  {
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Incorrect index value passed");
    return 0;
  }
  pTrav = pFontBase->pFontObject;
  while(siCount != ssIndex)
  {
	  siCount++;
	  pTrav++;
  }

  return pTrav->uiOffLen;
} */


//*****************************************************************************
//
// PrintFontString
//
// Parameter List : pointer to \0 (NULL) terminated font string
//                  x,y,TotalWidth, TotalHeight is the bounding rectangle where
//                  the font is to be printed
//                  Multiline if true will print on multiple lines otherwise on 1 line
//                  Pointer to base structure
//
// Return Value  : BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************

/*BOOLEAN PrintFontString(UINT16 *pFontString, UINT8 *pDestBuffer, UINT16 siDestWidth, UINT16 siDestPixelDepth, UINT16 siDestPitch, UINT16 siDestHeight, UINT16 siX, UINT16 siY, UINT16 siTotalWidth, UINT16 siTotalHeight, BOOLEAN fMultiLine, FontBase *pFontBase)
{
  UINT16  siScreenHt;
	UINT16  siScreenWt;
	UINT16  siChar, siHeightEach;
	INT16   ssIndex;
	UINT32  uiWidth, uiOffsetSt, uiOffsetEnd, uiOldoffst;
	UINT16 *pTempFStr;
	UINT16  siNewX, siNewY;
	UINT16  siInitX, siInitY;
	UINT32  uiLen;

	// check for NULL pointers passed in
	Assert(pFontBase != NULL);
	Assert(pFontString != NULL);
	Assert(pDestBuffer != NULL);

	siScreenWt = siDestWidth;
	siScreenHt = siDestHeight;

	// check for invalid coordinates
	if((siX<0) || (siX>siScreenWt) || (siY<0) || (siY>siScreenHt) || (siTotalWidth<0) || (siTotalWidth>siScreenWt) ||	(siTotalHeight<0) || (siTotalHeight>siScreenHt))
	{
	  DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Invalid coordinates passed in");
    return FALSE;
  }
	pTempFStr = pFontString;

	siNewX = siX;
	siNewY = siY;
	siInitX = siX;
  siInitY = siY;

	// Get the height of each font and the offset
	siHeightEach = GetFontHeight(pFontBase);
	uiOldoffst = (sizeof(FontHeader) + sizeof(FontObject)*pFontBase->uiTotalElements);

	// calls the blt routine until the string != to \0
	while(*pTempFStr != STRING_DELIMITER)
	{
	  siChar = *pTempFStr;
	  // get the index value for the font
	  if((ssIndex = GetIndex(siChar)) == -1)
		{
      return FALSE;
    }

	  // get the width of the font
	  uiWidth = GetWidth(pFontBase, ssIndex);

	  // get the font offset
	  uiOffsetSt = GetOffset(pFontBase, ssIndex);

	  uiLen = GetOffLen(pFontBase,ssIndex);

	  // uiOffsetSt -= uiOldoffst;
	  uiOffsetEnd = uiOffsetSt + uiLen;

	  // if Multiline = FALSE and reached the end of line - cannot continue
	  if ((((siNewX+uiWidth) > siScreenWt) || ((siNewX+uiWidth) >= siTotalWidth)) && (fMultiLine == FALSE))
	  {
	    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Cannot continue writing");
      return FALSE;
    }

	  // check if boundary is reached
	  if ((((siNewX+uiWidth) >= siScreenWt) || ((siNewX+uiWidth) >= siTotalWidth)) && (fMultiLine == TRUE))
	  {
		  if (((siInitY+siHeightEach) > siScreenHt) || ((siInitY+siHeightEach) >= siTotalHeight))
		  {
	      DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Cannot continue writing");
        return FALSE;
      }
		  //call the appropriate blit routines
      siNewX = siInitX;
		  siNewY += siHeightEach;
		  siInitY = siNewY;
      if ((siDestPixelDepth == 16) && (pFontBase->siPixelDepth == 16))
	   	{
        Blt16Imageto16Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
      }
		  else
	    {
        if ((siDestPixelDepth == 16) && (pFontBase->siPixelDepth == 8))
        {
		      Blt8Imageto16Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
        }
		    else
	      {
          if ((siDestPixelDepth == 8) && (pFontBase->siPixelDepth == 8))
		      { // if(SetPalette(pFontBase->pPalette) == FALSE)
	          //		    return FALSE;
		        Blt8Imageto8Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
		      }
		      else
		      {
	          DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Invalid pixel depth / destination surface depth");
            return FALSE;
		      }
        }
              siNewX += (UINT16)uiWidth;
      }
	  } else
	  { // if it isnt end of boundary copy at current location
	    if((siDestPixelDepth == 16) && (pFontBase->siPixelDepth == 16))
	    {
        Blt16Imageto16Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
      }
		  else
	    {
        if((siDestPixelDepth == 16) && (pFontBase->siPixelDepth == 8))
		    {
          Blt8Imageto16Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
        }
	      else
		    {
          if ((siDestPixelDepth == 8) && (pFontBase->siPixelDepth == 8))
		      {
            Blt8Imageto8Dest(uiOffsetSt, uiOffsetEnd, siNewX, siNewY, uiWidth, pFontBase, pDestBuffer, siDestPitch, siHeightEach);
          }
          else
		      {
	          DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, "Invalid pixel depth / destination surface depth");
            return FALSE;
		      }
	      }
		  siNewX += (UINT16)uiWidth;
	    }
    }
    // increment string pointer
	  pTempFStr++;
  }
	return TRUE;
}

*/

/*BOOLEAN InitializeFontManager(UINT16 usDefaultPixelDepth, FontTranslationTable *pTransTable)
{
FontTranslationTable *pTransTab;

	// register the appropriate debug topics
	if(pTransTable == NULL)
	{
    return FALSE;
  }
	RegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");

	if ((pFManager = (FontManager *)MemAlloc(sizeof(FontManager)))==NULL)
	{
    return FALSE;
  }

	if((pTransTab = (FontTranslationTable *)MemAlloc(sizeof(FontTranslationTable)))==NULL)
	{
    return FALSE;
  }

	pFManager->pTranslationTable = pTransTab;
	pFManager->usDefaultPixelDepth = usDefaultPixelDepth;
	pTransTab->usNumberOfSymbols = pTransTable->usNumberOfSymbols;
  pTransTab->DynamicArrayOf16BitValues = pTransTable->DynamicArrayOf16BitValues;

	return TRUE;
}	*/

/*void ShutdownFontManager(void)
{
  UnRegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");
  MemFree(pFManager);
}	*/

