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
  }
  else
  {
    throw InvalidEncodingException("Invalid UTF-8 encoding");
  }
}

/** Create object from UTF-16 encoded string. */
UTF8String::UTF8String(const uint16_t *utf16Encoded) throw (InvalidEncodingException)
{
  const uint16_t *zeroSearch = utf16Encoded;
  while(*zeroSearch) zeroSearch++;
  m_encoded = "";
  try
  {
    utf8::utf16to8(utf16Encoded, zeroSearch, back_inserter(m_encoded));
  }
  catch(utf8::invalid_utf16 &x)
  {
    throw InvalidEncodingException(x.what());
  }
}

/** Create object from UTF-32 encoded string. */
UTF8String::UTF8String(const uint32_t *utf32Encoded) throw (InvalidEncodingException)
{
  const uint32_t *zeroSearch = utf32Encoded;
  while(*zeroSearch) zeroSearch++;
  m_encoded = "";
  try
  {
    utf8::utf32to8(utf32Encoded, zeroSearch, back_inserter(m_encoded));
  }
  catch(utf8::invalid_code_point &x)
  {
    throw InvalidEncodingException(x.what());
  }
}


/** Get UTF-8 encoded string. */
const char* UTF8String::getUTF8() const
{
  return m_encoded.c_str();
}


/** Get UTF-16 encoded string. */
std::vector<uint16_t> UTF8String::getUTF16() const
{
  std::vector<uint16_t> words;
  utf8::utf8to16(m_encoded.begin(), m_encoded.end(), back_inserter(words));
  return words;
}

/** Get UTF-32 encoded string. */
std::vector<uint32_t> UTF8String::getUTF32() const
{
  std::vector<uint32_t> words;
  utf8::utf8to32(m_encoded.begin(), m_encoded.end(), back_inserter(words));
  return words;
}

#ifdef WCHAR_SUPPORT

/** Get wchar_t string.
 * @return Pointer to the internal buffer. */
const wchar_t * UTF8String::getWCHAR()
{
  m_wcharBuffer.clear();
#if _WIN32
  utf8::utf8to16(m_encoded.begin(), m_encoded.end(), back_inserter(m_wcharBuffer));
#else
  utf8::utf8to32(m_encoded.begin(), m_encoded.end(), back_inserter(m_wcharBuffer));
#endif
  m_wcharBuffer.push_back(0);
  return m_wcharBuffer.data();
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


// protected:
//   /** UTF-8 encoded string. */
//   std::string m_encoded;
// };
