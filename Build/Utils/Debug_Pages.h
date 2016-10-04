#include "Font.h"

#ifndef __DEBUG_PAGES_H
#define __DEBUG_PAGES_H

#define DEBUG_PAGE_HEADER_COLOR FONT_MCOLOR_BLACK<<8 | FONT_MCOLOR_LTGREEN
#define DEBUG_PAGE_TEXT_COLOR FONT_MCOLOR_BLACK<<8 | FONT_MCOLOR_LTGRAY2

#define DEBUG_PAGE_FONT FONT14ARIAL
#define DEBUG_PAGE_SCREEN_OFFSET_Y 20
#define DEBUG_PAGE_SCREEN_OFFSET_X 10
#define DEBUG_PAGE_LINE_HEIGHT 15
#define DEBUG_PAGE_START_Y DEBUG_PAGE_SCREEN_OFFSET_Y + DEBUG_PAGE_LINE_HEIGHT
#define DEBUG_PAGE_FIRST_COLUMN DEBUG_PAGE_SCREEN_OFFSET_X
#define DEBUG_PAGE_SECOND_COLUMN 300
#define DEBUG_PAGE_LABEL_WIDTH 150

#define MPageHeader(...) SetFont(DEBUG_PAGE_FONT); SetFontColors(DEBUG_PAGE_HEADER_COLOR); mprintf(DEBUG_PAGE_FIRST_COLUMN, DEBUG_PAGE_SCREEN_OFFSET_Y, __VA_ARGS__); SetFontColors(DEBUG_PAGE_TEXT_COLOR);

void MHeader(INT32 const x, INT32 const y, wchar_t const* const str);
void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, INT32 const val);
void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, char const* const val);
void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, wchar_t const* const val);
void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, void const* const val);
void MPrintStat(INT32 const x, INT32 const y, wchar_t const* const header, INT32 const val, INT32 const effective_val);

#endif
