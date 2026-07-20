#include "SGPFile.h"
#include "Exceptions.h"
#include "Logger.h"

#include <SDL3/SDL_iostream.h>
#include <string_theory/string>
#include <string_theory/format>
#include <string_view>
#include <utility>

void DeleteSGPFile(SGPFile *file)
{
    delete file;
}

class SGPFileException : public IoException
{
public:
	SGPFileException(std::string_view errorMessage,
	                 ST::string const& filename,
	                 RustPointer<char> const& rustError)
		: IoException{ ST::format("SGPFile: '{}' {}: {}",
			filename, errorMessage, rustError.get()) }
	{
	}
};

static Sint64 SDLCALL SGPSeekRW(void *userdata, Sint64 offset, SDL_IOWhence whence)
{
	SGPFile* sgpFile = (SGPFile*)(userdata);
	FileSeekMode mode = FILE_SEEK_FROM_CURRENT;
	switch (whence) {
		case SDL_IO_SEEK_SET:
			mode = FILE_SEEK_FROM_START;
			break;
		case SDL_IO_SEEK_END:
			mode = FILE_SEEK_FROM_END;
			break;
		default:
			break;
	}

	sgpFile->seek(offset, mode);

	return int64_t(sgpFile->pos());
}

static Sint64 SDLCALL SGPSizeRW(void *userdata)
{
	SGPFile* sgpFile = (SGPFile*)(userdata);
	return sgpFile->size();
}

static size_t SDLCALL SGPReadRW(void *userdata, void *ptr, size_t size, SDL_IOStatus *status)
{
	SGPFile* sgpFile = (SGPFile*)(userdata);
	auto bytesRead = sgpFile->readAtMost(ptr, size);
	if (bytesRead < size) {
		*status = SDL_IO_STATUS_EOF;
	}
	return bytesRead;
}

static size_t SDLCALL SGPWriteRW(void *userdata, const void *ptr, size_t size, SDL_IOStatus *status)
{
	*status = SDL_IO_STATUS_READONLY;
	return 0;
}

static bool SDLCALL SGPCloseRW(void *userdata)
{
	SGPFile* sgpFile = (SGPFile*)(userdata);
	delete sgpFile;
	return true;
}

SGPFile::SGPFile(VFile *f, ST::string const& filename) :
	file{ f },
	name{ filename }
{
}

SGPFile::SGPFile(VFile *f, ST::string && filename) :
	file{ f },
	name{ std::move(filename) }
{
}


SGPFile::~SGPFile()
{
    File_close(this->file);
}

SGPFile* SGPFile::openInVfs(Vfs* vfs, const ST::string& filename) {
	RustPointer<VFile> vfile(Vfs_open(vfs, filename.c_str()));
	if (!vfile)
	{
		RustPointer<char> err{getRustError()};
		throw std::runtime_error(ST::format("SGPFile::openInVfs: {}", err.get()).to_std_string());
	}
	return new SGPFile(vfile.release(), filename);
}

void SGPFile::read(void *const pDest, size_t const uiBytesToRead)
{
    bool success = File_readExact(this->file, reinterpret_cast<uint8_t *>(pDest), uiBytesToRead);

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("read failed", name, err);
    }
}

std::vector<uint8_t> SGPFile::readToEnd()
{
    RustPointer<VecU8> vec;
    vec.reset(File_readToEnd(this->file));

    if (vec.get() == NULL)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("readToEnd failed", name, err);
    }
    auto bytes = VecU8_as_ptr(vec.get());
    auto len = VecU8_len(vec.get());
    return std::vector<uint8_t>(bytes, bytes + len);
}

size_t SGPFile::readAtMost(void *const pDest, size_t const uiBytesToRead)
{
    size_t bytesRead = File_read(this->file, reinterpret_cast<uint8_t *>(pDest), uiBytesToRead);

    if (bytesRead == SIZE_MAX)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("readAtMost failed", name, err);
    }

    return bytesRead;
}

ST::string SGPFile::readString(size_t const uiBytesToRead)
{
    ST::char_buffer buf(uiBytesToRead, '\0');
    this->read(buf.data(), uiBytesToRead);
    return ST::string(buf.c_str(), ST_AUTO_SIZE);
}

ST::string SGPFile::readStringToEnd()
{
    uint32_t size = this->size();
    ST::char_buffer buf{size, '\0'};
    this->read(buf.data(), size);
    return ST::string{buf};
}

void SGPFile::write(void const *const pDest, size_t const uiBytesToWrite)
{
    bool success = File_writeAll(this->file, reinterpret_cast<const uint8_t *>(pDest), uiBytesToWrite);

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("write failed", name, err);
    }
}

void SGPFile::seek(INT32 distance, FileSeekMode const how)
{
    bool success;
    switch (how)
    {
    case FILE_SEEK_FROM_START:
        success = distance >= 0 && File_seekFromStart(this->file, static_cast<uint64_t>(distance)) != UINT64_MAX;
        break;
    case FILE_SEEK_FROM_END:
        success = File_seekFromEnd(this->file, distance) != UINT64_MAX;
        break;
    default:
        success = File_seekFromCurrent(this->file, distance) != UINT64_MAX;
        break;
    }

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("seek failed", name, err);
    }
}

INT32 SGPFile::pos() const
{
    uint64_t position = File_seekFromCurrent(this->file, 0);
    bool success = position != UINT64_MAX;

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("pos failed", name, err);
    }
    if (position > INT32_MAX)
    {
        SLOGW("SGPFile::pos truncating from {} to {}", position, INT32_MAX);
        position = INT32_MAX;
    }
    return static_cast<INT32>(position);
}

UINT32 SGPFile::size() const
{

    uint64_t len = File_len(this->file);
    bool success = len != UINT64_MAX;

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw SGPFileException("size failed", name, err);
    }
    if (len > UINT32_MAX)
    {
        SLOGW("SGPFile::size truncating from {} to {}", len, UINT32_MAX);
        len = UINT32_MAX;
    }
    return static_cast<UINT32>(len);
}

SDL_IOStream* SGPFile::getRwOps()
{
	SDL_IOStreamInterface iface;

	SDL_INIT_INTERFACE(&iface);
	iface.size = SGPSizeRW;
	iface.seek = SGPSeekRW;
	iface.read = SGPReadRW;
	iface.write= SGPWriteRW;
	iface.close= SGPCloseRW;

	auto rwops = SDL_OpenIO(&iface, this);
    if (!rwops) {
        iface.close(this);
    }
    return rwops;
}
