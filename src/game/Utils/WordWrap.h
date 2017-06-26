#ifndef __WORDWRAP_H_
#define __WORDWRAP_H_

#include "Types.h"

//Flags for DrawTextToScreen()


// Defines for coded text For use with IanDisplayWrappedString()
#define TEXT_SPACE		32
#define TEXT_CODE_NEWLINE	177
#define TEXT_CODE_BOLD		178
#define TEXT_CODE_CENTER	179
#define TEXT_CODE_NEWCOLOR	180
#define TEXT_CODE_DEFCOLOR	181

UINT16 IanDisplayWrappedString(UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT8 ubGap, SGPFont, UINT8 ubColor, const wchar_t* pString, UINT8 ubBackGroundColor, UINT32 uiFlags);


#define LEFT_JUSTIFIED		0x00000001
#define CENTER_JUSTIFIED	0x00000002
#define RIGHT_JUSTIFIED	0x00000004
#define TEXT_SHADOWED		0x00000008

#define INVALIDATE_TEXT	0x00000010
#define DONT_DISPLAY_TEXT	0x00000020 //Wont display the text.  Used if you just want to get how many lines will be displayed

#define MARK_DIRTY		0x00000040


#define IAN_WRAP_NO_SHADOW	32


struct WRAPPED_STRING
{
	WRAPPED_STRING* pNextWrappedString;
	wchar_t sString[];
};


WRAPPED_STRING* LineWrap(SGPFont, UINT16 usLineWidthPixels, wchar_t const* pString);
UINT16 DisplayWrappedString(UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT8 ubGap, SGPFont, UINT8 ubColor, const wchar_t* pString, UINT8 ubBackGroundColor, UINT32 ulFlags);
void CleanOutControlCodesFromString(const wchar_t* pSourceString, wchar_t* pDestString);
void DrawTextToScreen(const wchar_t* pStr, UINT16 LocX, UINT16 LocY, UINT16 usWidth, SGPFont, UINT8 ubColor, UINT8 ubBackGroundColor, UINT32 ulFlags);
UINT16 IanWrappedStringHeight(UINT16 usWidth, UINT8 ubGap, SGPFont, const wchar_t* pString);

void ReduceStringLength(wchar_t* pString, size_t Length, UINT32 uiWidth, SGPFont);

#endif
