#ifndef __DEBUG_PAGES_H
#define __DEBUG_PAGES_H

#include "Font.h"

#include <string_theory/string>


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

void MPageHeader(const ST::string& header);
void MHeader(INT32 x, INT32 y, const ST::string& header);
void MPrintStat(INT32 x, INT32 y, const ST::string& header, INT32 val);
void MPrintStat(INT32 x, INT32 y, const ST::string& header, const ST::string& val);
void MPrintStat(INT32 x, INT32 y, const ST::string& header, char const* val);
void MPrintStat(INT32 x, INT32 y, const ST::string& header, const void* val);
void MPrintStat(INT32 x, INT32 y, const ST::string& header, INT32 val, INT32 effective_val);

#endif
