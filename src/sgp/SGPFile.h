#pragma once

#include <stdint.h>

#include "sgp/AutoObj.h"

struct SGP_FILETIME
{
	uint32_t Lo;
	uint32_t Hi;
};

enum SGPFileFlags
{
	SGPFILE_NONE = 0U,
	SGPFILE_REAL = 1U << 0
};

struct LibraryFile;

struct SGPFile
{
	SGPFileFlags flags;
	union
	{
		FILE*       file;
		LibraryFile* lib;
	} u;
};

enum FileSeekMode
{
	FILE_SEEK_FROM_START,
	FILE_SEEK_FROM_END,
	FILE_SEEK_FROM_CURRENT
};

extern void FileClose(SGPFile*);

typedef SGP::AutoObj<SGPFile, FileClose> AutoSGPFile;
