#include "SGPFile.h"
#include "RustInterface.h"
#include "Exceptions.h"
#include "Logger.h"

#include <string_theory/string>
#include <string_theory/format>

#define SDL_RWOPS_SGP 222

void DeleteSGPFile(SGPFile *file)
{
    delete file;
}

static int64_t SGPSeekRW(SDL_RWops *context, int64_t offset, int whence)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);
	FileSeekMode mode = FILE_SEEK_FROM_CURRENT;
	switch (whence) {
		case RW_SEEK_SET:
			mode = FILE_SEEK_FROM_START;
			break;
		case RW_SEEK_END:
			mode = FILE_SEEK_FROM_END;
			break;
		default:
			break;
	}

	sgpFile->seek(offset, mode);

	return int64_t(sgpFile->pos());
}

static int64_t SGPSizeRW(SDL_RWops *context)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);

	return sgpFile->size();
}

static size_t SGPReadRW(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);
	return sgpFile->readAtMost(ptr, size * maxnum) / size;
}

static size_t SGPWriteRW(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
    SLOGA("SGPWriteRW not supported");
	return 0;
}

static int SGPCloseRW(SDL_RWops *context)
{
	if(context->type != SDL_RWOPS_SGP)
	{
		return SDL_SetError("Wrong kind of SDL_RWops for SGPCloseRW()");
	}
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);

	delete sgpFile;
	SDL_FreeRW(context);

	return 0;
}

SGPFile::SGPFile(VFile *f)
{
    this->flags = SGPFILE_REAL;
    this->file = f;
}

SGPFile::~SGPFile()
{
    File_close(this->file);
}

void SGPFile::read(void *const pDest, size_t const uiBytesToRead)
{
    bool success = File_readExact(this->file, reinterpret_cast<uint8_t *>(pDest), uiBytesToRead);

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::read failed: {}", err.get()));
    }
}

std::vector<uint8_t> SGPFile::readToEnd()
{
    RustPointer<VecU8> vec;
    vec.reset(File_readToEnd(this->file));

    if (vec.get() == NULL)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::readToEnd failed: {}", err.get()));
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
        throw IoException(ST::format("SGPFile::readAtMost failed: {}", err.get()));
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
        throw IoException(ST::format("SGPFile::write failed: {}", err.get()).c_str());
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
        throw IoException(ST::format("SGPFile::seek failed: {}", err.get()).c_str());
    }
}

INT32 SGPFile::pos() const
{
    uint64_t position = File_seekFromCurrent(this->file, 0);
    bool success = position != UINT64_MAX;

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::pos failed: {}", err.get()).c_str());
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
        throw IoException(ST::format("SGPFile::size failed: {}", err.get()).c_str());
    }
    if (len > UINT32_MAX)
    {
        SLOGW("SGPFile::size truncating from {} to {}", len, UINT32_MAX);
        len = UINT32_MAX;
    }
    return static_cast<UINT32>(len);
}

SDL_RWops* SGPFile::getRwOps()
{
	SDL_RWops* rwOps = SDL_AllocRW();
	if(rwOps == NULL) {
		return NULL;
	}
	rwOps->type = SDL_RWOPS_SGP;
	rwOps->size = SGPSizeRW;
	rwOps->seek = SGPSeekRW;
	rwOps->read = SGPReadRW;
	rwOps->write= SGPWriteRW;
	rwOps->close= SGPCloseRW;
	rwOps->hidden.unknown.data1 = this;

	return rwOps;
}
