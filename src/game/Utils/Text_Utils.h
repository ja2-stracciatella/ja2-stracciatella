#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include "Text.h"
#include "Types.h"

#include <string_theory/string>


void LoadItemInfo(UINT16 ubIndex, wchar_t Info[]);
inline ST::string LoadItemInfo(UINT16 ubIndex)
{
        wchar_t Info[SIZE_ITEM_INFO];
        LoadItemInfo(ubIndex, Info);
        return Info;
}

#endif
