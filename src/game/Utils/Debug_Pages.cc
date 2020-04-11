#include "Debug_Pages.h"
#include "Font.h"
#include "Font_Control.h"

#include <string_theory/format>


void MHeader(INT32 const x, INT32 const y, wchar_t const* const str)
{
	SetFontColors(DEBUG_PAGE_HEADER_COLOR);
	MPrint(x, y, str);
	SetFontColors(DEBUG_PAGE_TEXT_COLOR);
}


void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, INT32 const val)
{
	MHeader(x, y, header);
	MPrint(x+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{}", val));
}

void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, char const* const val)
{
	MHeader(x, y, header);
	MPrint(x+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{}", val));
}

void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, wchar_t const* const val)
{
	MHeader(x, y, header);
	MPrint(x+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{}", val));
}


void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, void const* const val)
{
	MHeader(x, y, header);
	MPrint(x+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{#x}", reinterpret_cast<uintptr_t>(val)));
}

void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, INT32 const val, INT32 const effective_val)
{
	MHeader(x, y, header);
	MPrint(x+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} ( {} )", val, effective_val));
}
