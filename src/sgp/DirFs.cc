#include "DirFs.h"
#include "Exceptions.h"
#include "FileMan.h"
#include "Logger.h"

#include <utility>
#include <vector>

namespace
{
	/** Chunk size used when a file has to be copied instead of moved. */
	constexpr size_t COPY_BUFFER_SIZE = 64 * 1024;

	/** Collects the strings of a rust vector, optionally reducing them to file names. */
	std::vector<ST::string> collectPaths(RustPointer<VecCString> vec, bool returnOnlyNames)
	{
		std::vector<ST::string> paths;
		if (!vec)
		{
			RustPointer<char> err{getRustError()};
			SLOGW("DirFs: listing failed: {}", err.get());
			return paths;
		}
		size_t len = VecCString_len(vec.get());
		for (size_t i = 0; i < len; i++)
		{
			RustPointer<char> path{VecCString_get(vec.get(), i)};
			if (returnOnlyNames)
			{
				RustPointer<char> filename{Path_filename(path.get())};
				paths.emplace_back(filename.get());
			}
			else
			{
				paths.emplace_back(path.get());
			}
		}
		return paths;
	}
}

std::unique_ptr<DirFs> DirFs::createTemp()
{
	RustPointer<VfsLayer> layer{VfsLayer_fromTempDir()};
	if (!layer)
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("Failed to create temporary directory: {}", err.get()));
	}
	RustPointer<char> path{VfsLayer_dirPath(layer.get())};
	return std::unique_ptr<DirFs>(new DirFs(std::move(layer), path.get()));
}

VfsLayer *DirFs::layer()
{
	if (!m_layer)
	{
		m_layer.reset(VfsLayer_fromDir(m_location.c_str(), true));
		if (!m_layer)
		{
			RustPointer<char> err{getRustError()};
			throw IoException(ST::format("DirFs('{}') could not be opened: {}", m_location, err.get()));
		}
	}
	return m_layer.get();
}

VfsLayer *DirFs::layerOrNull() noexcept
{
	try
	{
		return layer();
	}
	catch (const IoException &)
	{
		return nullptr;
	}
}

ST::string DirFs::basePath()
{
	return m_location;
}

SGPFile *DirFs::open(const ST::string &path, uint8_t options, const char *operation)
{
	RustPointer<VFile> file{VfsLayer_open(layer(), path.c_str(), options)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::{}('{}') failed: {}", operation, path, err.get()));
	}
	return new SGPFile(file.release(), path);
}

SGPFile *DirFs::openForWriting(const ST::string &path, bool truncate)
{
	uint8_t options = FILE_OPEN_WRITE | FILE_OPEN_CREATE;
	if (truncate)
	{
		options |= FILE_OPEN_TRUNCATE;
	}
	return open(path, options, "openForWriting");
}

SGPFile *DirFs::openForAppend(const ST::string &path)
{
	return open(path, FILE_OPEN_APPEND | FILE_OPEN_CREATE, "openForAppend");
}

SGPFile *DirFs::openForReadWrite(const ST::string &path)
{
	return open(path, FILE_OPEN_READ | FILE_OPEN_WRITE | FILE_OPEN_CREATE, "openForReadWrite");
}

SGPFile *DirFs::openForReading(const ST::string &path)
{
	return open(path, FILE_OPEN_READ, "openForReading");
}

void DirFs::deleteFile(const ST::string &path)
{
	if (exists(path) && !VfsLayer_removeFile(layer(), path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::deleteFile('{}') failed: {}", path, err.get()));
	}
}

void DirFs::createDir(const ST::string &path)
{
	if (!isDir(path) && !VfsLayer_createDir(layer(), path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::createDir('{}') failed: {}", path, err.get()));
	}
}

void DirFs::eraseDir(const ST::string &path)
{
	if (!VfsLayer_eraseDir(layer(), path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::eraseDir('{}') failed: {}", path, err.get()));
	}
}

std::vector<ST::string>
DirFs::findFilesInDir(const ST::string &path, const ST::string &ext, bool, bool returnOnlyNames, bool, bool recursive)
{
	RustPointer<VecCString> vec{VfsLayer_findAllFilesInDir(layer(), path.c_str(), ext.c_str(), recursive)};
	return collectPaths(std::move(vec), returnOnlyNames);
}

std::vector<ST::string>
DirFs::findAllFilesInDir(const ST::string &path, bool, bool recursive, bool returnOnlyNames)
{
	RustPointer<VecCString> vec{VfsLayer_findAllFilesInDir(layer(), path.c_str(), nullptr, recursive)};
	return collectPaths(std::move(vec), returnOnlyNames);
}

std::vector<ST::string>
DirFs::findAllDirsInDir(const ST::string &path, bool, bool recursive, bool returnOnlyNames)
{
	RustPointer<VecCString> vec{VfsLayer_findAllDirsInDir(layer(), path.c_str(), recursive)};
	return collectPaths(std::move(vec), returnOnlyNames);
}

ST::string DirFs::resolveExistingComponents(const ST::string &path)
{
	RustPointer<char> resolved{VfsLayer_resolveExistingComponents(layer(), path.c_str())};
	return resolved.get();
}

bool DirFs::isFile(const ST::string &path)
{
	VfsLayer *layer = layerOrNull();
	return layer && VfsLayer_isFile(layer, path.c_str());
}

bool DirFs::isDir(const ST::string &path)
{
	VfsLayer *layer = layerOrNull();
	return layer && VfsLayer_isDir(layer, path.c_str());
}

bool DirFs::isReadOnly(const ST::string &path)
{
	bool readonly = false;
	if (!VfsLayer_getReadOnly(layer(), path.c_str(), &readonly))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::isReadOnly('{}') failed: {}", path, err.get()));
	}
	return readonly;
}

bool DirFs::exists(const ST::string &path)
{
	VfsLayer *layer = layerOrNull();
	return layer && VfsLayer_exists(layer, path.c_str());
}

void DirFs::moveFile(const ST::string &from, const ST::string &to)
{
	if (!VfsLayer_rename(layer(), from.c_str(), to.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::moveFile('{}', '{}') failed: {}", from, to, err.get()));
	}
}

void DirFs::moveFileBetween(DirFs &fromFs, const ST::string &from, DirFs &toFs, const ST::string &to)
{
	RustPointer<char> fromDir{VfsLayer_dirPath(fromFs.layer())};
	RustPointer<char> toDir{VfsLayer_dirPath(toFs.layer())};
	if (fromDir && toDir)
	{
		// Both are plain directories, so let the filesystem move the file. It falls back to a copy
		// by itself when the two are on different volumes.
		FileMan::moveFile(
			FileMan::joinPaths(fromDir.get(), fromFs.resolveExistingComponents(from)),
			FileMan::joinPaths(toDir.get(), toFs.resolveExistingComponents(to)));
		return;
	}

	// At least one side has no path the filesystem could move to, so copy the contents over.
	{
		AutoSGPFile source{fromFs.openForReading(from)};
		AutoSGPFile target{toFs.openForWriting(to, true)};
		std::vector<uint8_t> buffer(COPY_BUFFER_SIZE);
		for (;;)
		{
			size_t read = source->readAtMost(buffer.data(), buffer.size());
			if (read == 0) break;
			target->write(buffer.data(), read);
		}
	}
	fromFs.deleteFile(from);
}

double DirFs::getLastModifiedTime(const ST::string &path)
{
	double lastModified = 0;
	if (!VfsLayer_modifiedSecs(layer(), path.c_str(), &lastModified))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::getLastModifiedTime('{}') failed: {}", path, err.get()));
	}
	return lastModified;
}

uint64_t DirFs::getFreeSpace(const ST::string&)
{
	uint64_t bytes = 0;
	if (!VfsLayer_freeSpace(layer(), &bytes))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("DirFs::getFreeSpace() failed: {}", err.get()));
	}
	return bytes;
}
