#include "UTF8String.h"
#include "utf8.h"

// #include <stdio.h>

/** Create object from existing UTF-8 encoded string. */
UTF8String::UTF8String(const char *utf8Encoded) throw (InvalidEncodingException)
  :m_encoded("")
{
  std::string value(utf8Encoded);
  if(utf8::is_valid(value.begin(), value.end()))
  {
    m_encoded = value;
    rebuildBufferWCHAR();
  }
  else
  {
    throw InvalidEncodingException("Invalid UTF-8 encoding");
  }
}

/** Create object from existing UTF-8 encoded string. */
UTF8String::UTF8String(const uint8_t *utf8Encoded) throw (InvalidEncodingException)
{
  std::string value((const char*)utf8Encoded);
  if(utf8::is_valid(value.begin(), value.end()))
  {
    m_encoded = value;
    rebuildBufferWCHAR();
  }
  else
  {
    throw InvalidEncodingException("Invalid UTF-8 encoding");
  }
}

/** Create object from UTF-16 encoded string. */
UTF8String::UTF8String(const uint16_t *utf16Encoded) throw (InvalidEncodingException)
{
  m_encoded = "";
  append(utf16Encoded);
}

/** Create object from UTF-32 encoded string. */
UTF8String::UTF8String(const uint32_t *utf32Encoded) throw (InvalidEncodingException)
{
  m_encoded = "";
  append(utf32Encoded);
}


#ifdef WCHAR_SUPPORT
/** Create string from wchar. */
UTF8String::UTF8String(const wchar_t *string)
{
  m_encoded = "";
#if _WIN32
  append((const uint16_t *)string);
#else
  append((const uint32_t *)string);
#endif
}
#endif


/** Get UTF-8 encoded string. */
const char* UTF8String::getUTF8() const
{
  return m_encoded.c_str();
}


/** Get UTF-16 encoded string.
 * @return Array of code points including trailing 0x0000. */
std::vector<uint16_t> UTF8String::getUTF16() const
{
  std::vector<uint16_t> code_points;
  utf8::utf8to16(m_encoded.begin(), m_encoded.end(), back_inserter(code_points));
  code_points.push_back(0);
  return code_points;
}

/** Get UTF-32 encoded string.
 * @return Array of code points including trailing 0x00000000. */
std::vector<uint32_t> UTF8String::getUTF32() const
{
  std::vector<uint32_t> code_points;
  utf8::utf8to32(m_encoded.begin(), m_encoded.end(), back_inserter(code_points));
  code_points.push_back(0);
  return code_points;
}

#ifdef WCHAR_SUPPORT

void UTF8String::rebuildBufferWCHAR()
{
  m_wcharBuffer.clear();
#if _WIN32
  utf8::utf8to16(m_encoded.begin(), m_encoded.end(), back_inserter(m_wcharBuffer));
#else
  utf8::utf8to32(m_encoded.begin(), m_encoded.end(), back_inserter(m_wcharBuffer));
#endif
  m_wcharBuffer.push_back(0);
}

/** Get wchar_t string.
 * @return Reference to the internal array of wchar_t characters.  The array includes trailing zero. */
const std::vector<wchar_t>& UTF8String::getWCHAR() const
{
  return m_wcharBuffer;
}

#endif


/** Get number of characters in the string. */
size_t UTF8String::getNumCharacters() const
{
  return utf8::distance(m_encoded.begin(), m_encoded.end());
}


/** Get number of bytes occupied by utf-8 string.
 * Trailing zero is not included. */
size_t UTF8String::getNumBytes() const
{
  return m_encoded.size();
}


/** Append UTF16 encoded data to the string. */
void UTF8String::append(const uint16_t *utf16Encoded) throw (InvalidEncodingException)
{
  const uint16_t *zeroSearch = utf16Encoded;
  while(*zeroSearch) zeroSearch++;
  try
  {
    utf8::utf16to8(utf16Encoded, zeroSearch, back_inserter(m_encoded));
    rebuildBufferWCHAR();
  }
  catch(utf8::invalid_utf16 &x)
  {
    throw InvalidEncodingException(x.what());
  }
}

/** Append UTF32 encoded data to the string. */
void UTF8String::append(const uint32_t *utf32Encoded) throw (InvalidEncodingException)
{
  const uint32_t *zeroSearch = utf32Encoded;
  while(*zeroSearch) zeroSearch++;
  try
  {
    utf8::utf32to8(utf32Encoded, zeroSearch, back_inserter(m_encoded));
    rebuildBufferWCHAR();
  }
  catch(utf8::invalid_code_point &x)
  {
    throw InvalidEncodingException(x.what());
  }
}
