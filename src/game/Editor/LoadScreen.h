#ifndef _JA2_LOAD_SCREEN_
#define _JA2_LOAD_SCREEN_

#include "ScreenIDs.h"

#include <string_theory/string>


struct FDLG_LIST
{
	char       filename[260];
	FDLG_LIST* pNext;
	FDLG_LIST* pPrev;
};

FDLG_LIST* AddToFDlgList(FDLG_LIST*, char const* filename);


BOOLEAN ExternalLoadMap(const ST::string& szFilename);
BOOLEAN ExternalSaveMap(const ST::string& szFilename);

void SetErrorCatchString(const ST::string& str);

ScreenID LoadSaveScreenHandle(void);

#endif

