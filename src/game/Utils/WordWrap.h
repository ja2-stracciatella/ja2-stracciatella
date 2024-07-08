#ifndef __WORDWRAP_H_
#define __WORDWRAP_H_

#include "Types.h"

#include <string_theory/string>
#include <vector>


//Flags for DrawTextToScreen()


// Defines for coded text For use with IanDisplayWrappedString()
#define TEXT_SPACE		32
#define TEXT_CODE_NEWLINE	177
#define TEXT_CODE_BOLD		178
#define TEXT_CODE_CENTER	179
#define TEXT_CODE_NEWCOLOR	180
#define TEXT_CODE_DEFCOLOR	181

UINT16 IanDisplayWrappedString(UINT16 sx, UINT16 sy, UINT16 max_w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::utf32_buffer& codepoints, UINT8 background, UINT32 flags);
inline UINT16 IanDisplayWrappedString(UINT16 sx, UINT16 sy, UINT16 max_w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::string& str, UINT8 background, UINT32 flags)
{
	return IanDisplayWrappedString(sx, sy, max_w, gap, font, foreground, str.to_utf32(), background, flags);
}


#define LEFT_JUSTIFIED		0x00000001
#define CENTER_JUSTIFIED	0x00000002
#define RIGHT_JUSTIFIED	0x00000004
#define TEXT_SHADOWED		0x00000008

#define INVALIDATE_TEXT	0x00000010
#define DONT_DISPLAY_TEXT	0x00000020 //Wont display the text.  Used if you just want to get how many lines will be displayed

#define MARK_DIRTY		0x00000040


#define IAN_WRAP_NO_SHADOW	32


using WrappedString = std::vector<ST::utf32_buffer>;

WrappedString LineWrap(SGPFont, UINT16 usLineWidthPixels, const ST::utf32_buffer& codepoints);
inline WrappedString LineWrap(SGPFont font, UINT16 usLineWidthPixels, const ST::string& str)
{
	return LineWrap(font, usLineWidthPixels, str.to_utf32());
}
UINT16 DisplayWrappedString(UINT16 x, UINT16 y, UINT16 w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::utf32_buffer& codepoints, UINT8 background, UINT32 flags);
inline UINT16 DisplayWrappedString(UINT16 x, UINT16 y, UINT16 w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::string& str, UINT8 background, UINT32 flags)
{
	return DisplayWrappedString(x, y, w, gap, font, foreground, str.to_utf32(), background, flags);
}
ST::string CleanOutControlCodesFromString(const ST::utf32_buffer& codepoints);
inline ST::string CleanOutControlCodesFromString(const ST::string& str)
{
	return CleanOutControlCodesFromString(str.to_utf32());
}
void DrawTextToScreen(const ST::utf32_buffer& codepoints, UINT16 x, UINT16 y, UINT16 max_w, SGPFont font, UINT8 foreground, UINT8 background, UINT32 flags);
inline void DrawTextToScreen(const ST::string& str, UINT16 x, UINT16 y, UINT16 max_w, SGPFont font, UINT8 foreground, UINT8 background, UINT32 flags)
{
	DrawTextToScreen(str.to_utf32(), x, y, max_w, font, foreground, background, flags);
}
UINT16 IanWrappedStringHeight(UINT16 max_w, UINT8 gap, SGPFont font, const ST::utf32_buffer& codepoints);
inline UINT16 IanWrappedStringHeight(UINT16 max_w, UINT8 gap, SGPFont font, const ST::string& str)
{
	return IanWrappedStringHeight(max_w, gap, font, str.to_utf32());
}

ST::string ReduceStringLength(const ST::utf32_buffer& codepoints, UINT32 widthToFitIn, SGPFont font);
inline ST::string ReduceStringLength(const ST::string& str, UINT32 widthToFitIn, SGPFont font)
{
	return ReduceStringLength(str.to_utf32(), widthToFitIn, font);
}

#endif
