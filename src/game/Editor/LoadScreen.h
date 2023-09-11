#ifndef _JA2_LOAD_SCREEN_
#define _JA2_LOAD_SCREEN_

#include "ScreenIDs.h"
#include "Types.h"
#include <string_theory/string>

enum class FileType {
	Parent,
	Directory,
	File,
};

struct FileDialogEntry
{
	FileType type;
	ST::string filename;
};


extern ST::string gFileForIO;
BOOLEAN ExternalLoadMap(const ST::string& szFilename);

void SetErrorCatchString(const ST::string& str);

ScreenID LoadSaveScreenHandle(void);

#endif

