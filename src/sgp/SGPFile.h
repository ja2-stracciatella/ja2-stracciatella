#pragma once

#include <stdint.h>

#include "AutoObj.h"
#include "Types.h"
#include <string_theory/string>
#include <SDL_rwops.h>

enum FileSeekMode
{
	FILE_SEEK_FROM_START,
	FILE_SEEK_FROM_END,
	FILE_SEEK_FROM_CURRENT
};

struct VfsFile;

class SGPFile
{
private:
	VFile *file;
	ST::string name;

public:
	/** Create a SGP file from a file on disk. */
	/* The filename is only needed for diagnostic purposes. */
	SGPFile(VFile *file, ST::string const& filename);
	SGPFile(VFile *file, ST::string && filename);

	/** Closes file. */
	~SGPFile();

	/** Opens file in VFS */
	static SGPFile* openInVfs(Vfs* vfs, const ST::string& filename);

	/** Read exactly the number of bytes specified from the file into pDest. */
	void read(void *const pDest, size_t const bytesToRead);
	/** Read at most the number of bytes specified from the file into pDest. The actual number of bytes read is returned. */
	size_t readAtMost(void *const pDest, size_t const bytesToRead);
	/** Read the rest of the file from the current position into a vector. */
	std::vector<uint8_t> readToEnd();
	/** Read the next bytesToRead bytes to a string. */
	ST::string readString(size_t const bytesToRead);
	/** Read the rest of the file from the current position into a string. */
	ST::string readStringToEnd();

	/** Write bytesToWrite bytes from pSrc to the file. */
	void write(void const *const pSrc, size_t const bytesToWrite);

	/** Write size elements from data to the file. */
	template <typename T, typename U>
	void writeArray(T const &size, U const *const data)
	{
		this->write(&size, sizeof(size));
		if (size != 0)
			this->write(data, sizeof(*data) * size);
	}

	/** Seek a distance within the file. */
	void seek(INT32 distance, FileSeekMode const how);
	/** Get current position within the file. */
	INT32 pos() const;
	/** Get the size of the file. */
	UINT32 size() const;

	/** Get an SDL_RWops from the file. */
	SDL_RWops* getRwOps();

	/** Get the name of the file. */
	auto const& getName() const noexcept { return name; }
};

void DeleteSGPFile(SGPFile *file);

typedef SGP::AutoObj<SGPFile, DeleteSGPFile> AutoSGPFile;
