#include "BuildDefines.h"

#ifdef JA2EDITOR

#ifndef _JA2_LOAD_SCREEN_
#define _JA2_LOAD_SCREEN_

#include "FileMan.h"

typedef struct FDLG_LIST FDLG_LIST;
struct FDLG_LIST
{
	char       filename[260];
	FDLG_LIST* pNext;
	FDLG_LIST* pPrev;
};

FDLG_LIST* AddToFDlgList(FDLG_LIST* pList, const char* filename);


BOOLEAN ExternalLoadMap(const wchar_t* szFilename);
BOOLEAN ExternalSaveMap(const wchar_t* szFilename);

extern BOOLEAN gfErrorCatch;
extern wchar_t gzErrorCatchString[256];

#endif
#endif
