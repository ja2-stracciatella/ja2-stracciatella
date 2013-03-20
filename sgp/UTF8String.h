#pragma once

#include <stdint.h>
#include <string>
#include <vector>


class InvalidEncodingException;


/**
 * Exception which will be thrown in case
 * of invalid encoding. */
class InvalidEncodingException : public std::exception
{
public:
  InvalidEncodingException(const char *what)
    :m_what(what)
  {
  }

  virtual ~InvalidEncodingException() throw ()
  {
  }

  const char* what() const throw ()
  {
    return m_what.c_str();
  }

protected:
  std::string m_what;
};


/**
 * @brief UTF-8 encoded string.
 *
 * This is immutable object for storing UTF-8 encoded string.
 *
 * It is possible to create the object from UTF-8, UTF-16, UTF-32 encoded
 * strings and covert object into UTF-8, UTF-16 and UTF-32 encoded strings.
 */
class UTF8String
{
public:
  /** Create object from existing UTF-8 encoded string. */
  UTF8String(const char *utf8Encoded) throw (InvalidEncodingException);

  /** Create object from UTF-16 encoded string. */
  UTF8String(const uint16_t *utf16Encoded) throw (InvalidEncodingException);

  /** Create object from UTF-32 encoded string. */
  UTF8String(const uint32_t *utf32Encoded) throw (InvalidEncodingException);

  /** Get UTF-8 encoded string. */
  const char* getUTF8() const;

  /** Get UTF-16 encoded string. */
  std::vector<uint16_t> getUTF16() const;

  /** Get UTF-32 encoded string. */
  std::vector<uint32_t> getUTF32() const;

  /** Get number of characters in the string. */
  size_t getNumCharacters() const;

  /** Get number of bytes occupied by utf-8 string.
   * Trailing zero is not included. */
  size_t getNumBytes() const;

protected:
  /** UTF-8 encoded string. */
  std::string m_encoded;
};

