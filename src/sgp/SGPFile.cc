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

SGPFile::SGPFile(File *f)
{
    this->flags = SGPFILE_REAL;
    this->u.file = f;
}

SGPFile::SGPFile(VfsFile *f)
{
    this->flags = SGPFILE_NONE;
    this->u.vfile = f;
}

SGPFile::~SGPFile()
{
    if (this->flags & SGPFILE_REAL)
    {
        File_close(this->u.file);
    }
    else
    {
        VfsFile_close(this->u.vfile);
    }
}

void SGPFile::read(void *const pDest, size_t const uiBytesToRead)
{
    bool success;
    if (this->flags & SGPFILE_REAL)
    {
        success = File_readExact(this->u.file, reinterpret_cast<uint8_t *>(pDest), uiBytesToRead);
    }
    else
    {
        success = VfsFile_readExact(this->u.vfile, static_cast<uint8_t *>(pDest), uiBytesToRead);
    }

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::read failed: {}", err.get()));
    }
}

std::vector<uint8_t> SGPFile::readToEnd()
{
    RustPointer<VecU8> vec;
    if (this->flags & SGPFILE_REAL)
    {
        vec.reset(File_readToEnd(this->u.file));
    }
    else
    {
        vec.reset(VfsFile_readToEnd(this->u.vfile));
    }
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
    size_t bytesRead = 0;
    if (this->flags & SGPFILE_REAL)
    {
        bytesRead = File_read(this->u.file, reinterpret_cast<uint8_t *>(pDest), uiBytesToRead);
    }
    else
    {
        bytesRead = VfsFile_read(this->u.vfile, static_cast<uint8_t *>(pDest), uiBytesToRead);
    }

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
    bool success;
    if (this->flags & SGPFILE_REAL)
    {
        success = File_writeAll(this->u.file, reinterpret_cast<const uint8_t *>(pDest), uiBytesToWrite);
    }
    else
    {
        success = VfsFile_writeAll(this->u.vfile, reinterpret_cast<const uint8_t *>(pDest), uiBytesToWrite);
    }

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::write failed: {}", err.get()).c_str());
    }
}

void SGPFile::seek(INT32 distance, FileSeekMode const how)
{
    bool success;
    if (this->flags & SGPFILE_REAL)
    {
        switch (how)
        {
        case FILE_SEEK_FROM_START:
            success = distance >= 0 && File_seekFromStart(this->u.file, static_cast<uint64_t>(distance)) != UINT64_MAX;
            break;
        case FILE_SEEK_FROM_END:
            success = File_seekFromEnd(this->u.file, distance) != UINT64_MAX;
            break;
        default:
            success = File_seekFromCurrent(this->u.file, distance) != UINT64_MAX;
            break;
        }
    }
    else
    {
        switch (how)
        {
        case FILE_SEEK_FROM_START:
            success = distance >= 0 && VfsFile_seekFromStart(this->u.vfile, static_cast<uint64_t>(distance), nullptr);
            break;
        case FILE_SEEK_FROM_END:
            success = VfsFile_seekFromEnd(this->u.vfile, distance, nullptr);
            break;
        default:
            success = VfsFile_seekFromCurrent(this->u.vfile, distance, nullptr);
            break;
        }
    }
    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::seek failed: {}", err.get()).c_str());
    }
}

INT32 SGPFile::pos() const
{
    bool success;
    uint64_t position = 0;
    if (this->flags & SGPFILE_REAL)
    {
        position = File_seekFromCurrent(this->u.file, 0);
        success = position != UINT64_MAX;
    }
    else
    {
        success = VfsFile_seekFromCurrent(this->u.vfile, 0, &position);
    }

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::pos failed: {}", err.get()).c_str());
    }
    if (position > INT32_MAX)
    {
        STLOGW("SGPFile::pos truncating from {} to {}", position, INT32_MAX);
        position = INT32_MAX;
    }
    return static_cast<INT32>(position);
}

UINT32 SGPFile::size() const
{
    bool success;
    uint64_t len = 0;
    if (this->flags & SGPFILE_REAL)
    {
        len = File_len(this->u.file);
        success = len != UINT64_MAX;
    }
    else
    {
        success = VfsFile_len(this->u.vfile, &len);
    }

    if (!success)
    {
        RustPointer<char> err{getRustError()};
        throw IoException(ST::format("SGPFile::size failed: {}", err.get()).c_str());
    }
    if (len > UINT32_MAX)
    {
        STLOGW("SGPFile::size truncating from {} to {}", len, UINT32_MAX);
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
