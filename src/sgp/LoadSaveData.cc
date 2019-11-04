#include "LoadSaveData.h"
#include "UTF8String.h"


/** Encode wchar_t into UTF-16 and write to the buffer.
 * @param string        String to encode
 * @param outputBuf     Output buffer for the encoded string
 * @param charsToWrite  Number of characters to write (at least one trailing 0x0000 will be written) */
void wchar_to_utf16(const wchar_t *string, void *outputBuf, int charsToWrite)
{
	if(charsToWrite > 0)
	{
#ifdef _WIN32
		// Windows.
		// wchar_t is already encoded in utf-16
		const uint16_t *utf16_string = (const uint16_t*)string;
		size_t stringLen = wcslen(string);
#else
		// Linux or Mac OS.
		// Let's convert to utf-8 and then to utf-16
		UTF8String str((const uint32_t*)string);
		std::vector<uint16_t> utf16_data = str.getUTF16();
		const uint16_t *utf16_string = utf16_data.data();
		size_t stringLen = utf16_data.size();
#endif

		// calculating number of characters to copy
		int charsToCopy = MIN(charsToWrite, stringLen);
		memcpy(outputBuf, utf16_string, charsToCopy * 2);
		if(charsToCopy < charsToWrite)
		{
			memset(((char*)outputBuf) + charsToCopy * 2, 0, (charsToWrite-charsToCopy)*2);
		}
	}
}

/** Get number of the consumed bytes during reading. */
size_t DataReader::getConsumed() const
{
	return ((const char*)m_buf) - ((const char*)m_original);
}

////////////////////////////////////////////////////////////////////////////
// DataWriter
////////////////////////////////////////////////////////////////////////////

/** Constructor.
 * @param buf Pointer to the buffer for writing data. */
DataWriter::DataWriter(void *buf)
	:m_buf(buf), m_original(buf)
{
}

/** Write wchar string into UTF-16 format.
 *
 * If \a numChars is bigger then the number of actual characters in the string,
 * then zeroes will be written to the buffer.
 *
 * @param string      String to write
 * @param numChars    Number of characters to write. */
void DataWriter::writeStringAsUTF16(const wchar_t *string, int numChars)
{
	wchar_to_utf16(string, m_buf, numChars);
	move(2*numChars);
}

void DataWriter::writeU8 (uint8_t  value)
{
	*((uint8_t*)m_buf) = value;
	move(1);
}

void DataWriter::writeU16(uint16_t value)
{
	*((uint16_t*)m_buf) = value;
	move(2);
}

void DataWriter::writeU32(uint32_t value)
{
	*((uint32_t*)m_buf) = value;
	move(4);
}

/** Move pointer to \a numBytes bytes forward. */
void DataWriter::move(int numBytes)
{
	m_buf = ((char*)m_buf) + numBytes;
}

/** Get number of the consumed bytes during writing. */
size_t DataWriter::getConsumed() const
{
	return ((const char*)m_buf) - ((const char*)m_original);
}

////////////////////////////////////////////////////////////////////////////
// DataReader
////////////////////////////////////////////////////////////////////////////


/** Constructor.
 * @param buf Pointer to the buffer for writing data. */
DataReader::DataReader(const void *buf)
	:m_buf(buf), m_original(buf)
{
}

/** Read UTF-16 encoded string.
 * @param numChars Number of characters to read. */
UTF8String DataReader::readUTF16(int numChars, const IEncodingCorrector *fixer)
{
	std::vector<uint16_t> data;
	while(numChars-- > 0)
	{
		uint16_t codePoint = readU16();
		if(fixer)
		{
			codePoint = fixer->fix(codePoint);
		}
		data.push_back(codePoint);
	}
	data.push_back(0);
	return UTF8String(data.data());
}

/** Read UTF-32 encoded string.
 * @param numChars Number of characters to read. */
ST::string DataReader::readUTF32(size_t numChars)
{
	if (numChars == 0) return ST::null;
	ST::utf32_buffer data;
	data.allocate(numChars);
	for (size_t i = 0; i < numChars; i++)
	{
		data[i] = readU32();
	}
	return ST::string::from_utf32(data.c_str()); // can throw ST::unicode_error
}

/** Read UTF-16 encoded string into wide string buffer.
 * @param buffer Buffer to read data in.
 * @param numChars Number of characters to read.
 * @param fixer Optional encoding corrector.  It is used for fixing incorrectly encoded text. */
void DataReader::readUTF16(wchar_t *buffer, int numChars, const IEncodingCorrector *fixer)
{
	UTF8String str = readUTF16(numChars, fixer);
	std::vector<wchar_t> wstr = str.getWCHAR();
	int charsToCopy = MIN(wstr.size(), numChars);
	memcpy(buffer, wstr.data(), charsToCopy * sizeof(wchar_t));
}

/** Read UTF-32 encoded string into wide string buffer.
 * @param buffer Buffer to read data in.
 * @param numChars Number of characters to read. */
void DataReader::readUTF32(wchar_t *buffer, size_t numChars)
{
	if (numChars == 0) return;
	ST::wchar_buffer wstr = readUTF32(numChars).to_wchar();
	size_t const charsToCopy = std::min<size_t>(wstr.size() + 1, numChars);
	memcpy(buffer, wstr.c_str(), charsToCopy * sizeof(wchar_t)); // might not terminate with '\0'
}

uint8_t DataReader::readU8()
{
	uint8_t value = *((uint8_t*)m_buf);
	move(1);
	return value;
}

uint16_t DataReader::readU16()
{
	uint16_t value = *((uint16_t*)m_buf);
	move(2);
	return value;
}

/** Move pointer to \a numBytes bytes forward. */
uint32_t DataReader::readU32()
{
	uint32_t value = *((uint32_t*)m_buf);
	move(4);
	return value;
}

/** Move pointer to \a numBytes bytes forward. */
void DataReader::move(int numBytes)
{
	m_buf = ((char*)m_buf) + numBytes;
}

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
