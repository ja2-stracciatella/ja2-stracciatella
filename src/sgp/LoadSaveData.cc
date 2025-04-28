#include "LoadSaveData.h"
#include "Logger.h"


////////////////////////////////////////////////////////////////////////////
// DataWriter
////////////////////////////////////////////////////////////////////////////

/** Constructor.
 * @param buf Pointer to the buffer for writing data. */
DataWriter::DataWriter(uint8_t* buf)
	:m_buf(buf), m_original(buf)
{
}

void DataWriter::writeUTF8(const ST::string& str, size_t numChars)
{
	size_t n = std::min<size_t>(str.size() + 1, numChars);
	if (str.size() > n)
	{
		SLOGW("DataWriter::writeUTF8: truncating '{}' {}->{}", str, str.size(), n);
	}
	writeArray(str.c_str(), n);
	skip(sizeof(char) * (numChars - n));
}

void DataWriter::writeUTF16(const ST::string& str, size_t numChars)
{
	ST::utf16_buffer buf = str.to_utf16();
	size_t n = std::min<size_t>(buf.size() + 1, numChars);
	if (buf.size() > n)
	{
		SLOGW("DataWriter::writeUTF16: truncating '{}' {}->{}", str, buf.size(), n);
	}
	writeArray(buf.c_str(), n);
	skip(sizeof(char16_t) * (numChars - n));
}

void DataWriter::writeU8 (uint8_t  value)
{
	write(value);
}

void DataWriter::writeU16(uint16_t value)
{
	write(value);
}

void DataWriter::writeU32(uint32_t value)
{
	write(value);
}

void DataWriter::skip(size_t numBytes)
{
	std::fill_n(m_buf, numBytes, 0);
	move(numBytes);
}

////////////////////////////////////////////////////////////////////////////
// DataReader
////////////////////////////////////////////////////////////////////////////


DataReader::DataReader(const uint8_t* buf)
	: m_buf(buf), m_original(buf)
{
}

ST::string DataReader::readUTF8(size_t numChars, ST::utf_validation_t validation)
{
	ST::char_buffer buf{numChars, '\0'};
	readArray(buf.data(), numChars);
	return ST::string(buf.c_str(), ST_AUTO_SIZE, validation);
}

ST::string DataReader::readUTF16(size_t numChars, bool const isCorrectlyEncoded, ST::utf_validation_t validation)
{
	ST::utf16_buffer buf{numChars, u'\0'};
	readArray(buf.data(), numChars);

	/* The Russian data files are incorrectly encoded. The original texts seem to
	 * be encoded in CP1251, but then they were converted from CP1252 (!) to
	 * UTF-16 to store them in the data files. Undo this damage here. */
	if (!isCorrectlyEncoded) {
		for (char16_t& c : buf) {
			if (0xC0 <= c && c <= 0xFF) c += 0x350;
			if (c == u'\0') break;
		}
	}

	return ST::string(buf.c_str(), ST_AUTO_SIZE, validation);
}

ST::string DataReader::readUTF32(size_t numChars, ST::utf_validation_t validation)
{
	ST::utf32_buffer buf{numChars, U'\0'};
	readArray(buf.data(), numChars);
	return ST::string(buf.c_str(), ST_AUTO_SIZE, validation);
}

uint8_t DataReader::readU8()
{
	return read<uint8_t>();
}

uint16_t DataReader::readU16()
{
	return read<uint16_t>();
}

uint32_t DataReader::readU32()
{
	return read<uint32_t>();
}

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
