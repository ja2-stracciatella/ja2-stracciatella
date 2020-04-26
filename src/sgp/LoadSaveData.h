#ifndef LOADSAVEDATA_H
#define LOADSAVEDATA_H

#include "Types.h"
#include "IEncodingCorrector.h"

#include <string_theory/string>

#include <algorithm>
#include <type_traits>


////////////////////////////////////////////////////////////////////////////
// DataWriter
////////////////////////////////////////////////////////////////////////////


/** Class for serializing data.
 * Assumes the endianess is correct. */
class DataWriter
{
private:
	DataWriter(const DataWriter&) = delete; // no copy
	DataWriter& operator=(const DataWriter&) = delete; // no assign
public:
	/** Constructor.
	 * @param buf Pointer to the buffer for writing data. */
	DataWriter(void* buf);

	/** Write UTF-8 encoded string.
	 * @param str      String to write
	 * @param numChars Number of `char` characters to write. */
	void writeUTF8(const ST::string& str, size_t numChars);

	/** Write UTF-16 encoded string.
	 * @param str      String to write
	 * @param numChars Number of `char16_t` characters to write. */
	void writeUTF16(const ST::string& str, size_t numChars);

	/** Write UTF-32 encoded string.
	 * @param str      String to write
	 * @param numChars Number of `char32_t` characters to write. */
	void writeUTF32(const ST::string& str, size_t numChars);

	void writeU8 (uint8_t  value);        /**< Write uint8_t */
	void writeU16(uint16_t value);        /**< Write uint16_t */
	void writeU32(uint32_t value);        /**< Write uint32_t */

	/* Write a value. */
	template<typename T>
	void write(const T& value)
	{
		static_assert(std::is_trivially_copyable<T>::value, "memcpy requires a trivially copyable type");
		size_t numBytes = sizeof(T);
		memcpy(m_buf, &value, numBytes);
		move(numBytes);
	}

	/* Write an array of values. */
	template<typename T>
	void writeArray(const T* arr, size_t len)
	{
		static_assert(std::is_trivially_copyable<T>::value, "memcpy requires a trivially copyable type");
		size_t numBytes = len * sizeof(T);
		memcpy(m_buf, arr, numBytes);
		move(numBytes);
	}

	/* Write zeroed bytes. */
	void skip(size_t numBytes);

	/** Get number of the consumed bytes during writing. */
	size_t getConsumed() const;

protected:
	void* m_buf;
	void* m_original;

	/** Move pointer to \a numBytes bytes forward. */
	void move(size_t numBytes);
};


////////////////////////////////////////////////////////////////////////////
// DataReader
////////////////////////////////////////////////////////////////////////////


/** Class for reading serializing data.
 * Assumes the endianess is correct. */
class DataReader
{
private:
	DataReader(const DataReader&) = delete; // no copy
	DataReader& operator=(const DataReader&) = delete; // no assign
public:
	/** Constructor.
	 * @param buf Pointer to the buffer for writing data. */
	DataReader(const void* buf);

	/** Read UTF-8 encoded string.
	 * @param numChars Number of `char` characters to read.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF8(size_t numChars, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	/** Read UTF-16 encoded string.
	 * @param numChars Number of `char16_t` characters to read.
	 * @param fixer Optional encoding corrector.  It is used for fixing incorrectly encoded text.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF16(size_t numChars, const IEncodingCorrector* fixer = nullptr, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	/** Read UTF-32 encoded string.
	 * @param numChars Number of `char32_t` characters to read.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF32(size_t numChars, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	uint8_t  readU8();            /**< Read uint8_t */
	uint16_t readU16();           /**< Read uint16_t */
	uint32_t readU32();           /**< Read uint32_t */

	/* Read raw bytes. */
	void readBytes(uint8_t* bytes, size_t numBytes);

	/* Read a value. */
	template<typename T>
	T read()
	{
		static_assert(std::is_trivially_copyable<T>::value, "memcpy requires a trivially copyable type");
		size_t numBytes = sizeof(T);
		T value;
		memcpy(&value, m_buf, numBytes);
		move(numBytes);
		return value;
	}

	/* Read an array of values. */
	template<typename T>
	void readArray(T* arr, size_t len)
	{
		static_assert(std::is_trivially_copyable<T>::value, "memcpy requires a trivially copyable type");
		size_t numBytes = len * sizeof(T);
		memcpy(arr, m_buf, numBytes);
		move(numBytes);
	}

	/* Read and discard bytes. */
	void skip(size_t numBytes);

	/** Get number of the consumed bytes during reading. */
	size_t getConsumed() const;

protected:
	const void* m_buf;
	const void* m_original;

	/** Move pointer to \a numBytes bytes forward. */
	void move(size_t numBytes);
};


#define INJ_STR(D, S, Size)  (D).writeArray<char>((S), (Size));
#define INJ_BOOLA(D, S, Size)  (D).writeArray<BOOLEAN>((S), (Size));
#define INJ_I8A(D, S, Size)  (D).writeArray<INT8>((S), (Size));
#define INJ_U8A(D, S, Size)  (D).writeArray<UINT8>((S), (Size));
#define INJ_I16A(D, S, Size) (D).writeArray<INT16>((S), (Size));
#define INJ_U16A(D, S, Size) (D).writeArray<UINT16>((S), (Size));
#define INJ_I32A(D, S, Size) (D).writeArray<INT32>((S), (Size));
#define INJ_BOOL(D, S)   (D).write<BOOLEAN>((S));
#define INJ_BYTE(D, S)   (D).write<BYTE>((S));
#define INJ_I8(D, S)     (D).write<INT8>((S));
#define INJ_U8(D, S)     (D).write<UINT8>((S));
#define INJ_I16(D, S)    (D).write<INT16>((S));
#define INJ_U16(D, S)    (D).write<UINT16>((S));
#define INJ_I32(D, S)    (D).write<INT32>((S));
#define INJ_U32(D, S)    (D).write<UINT32>((S));
#define INJ_FLOAT(D, S)  (D).write<FLOAT>((S));
#define INJ_DOUBLE(D, S) (D).write<DOUBLE>((S));
#define INJ_PTR(D, S) INJ_SKIP(D, 4)
#define INJ_SKIP(D, Size) (D).skip((Size));
#define INJ_SKIP_I16(D)   (D).skip(2);
#define INJ_SKIP_I32(D)   (D).skip(4);
#define INJ_SKIP_U8(D)    (D).skip(1);
#define INJ_SOLDIER(D, S) (D).write<SoldierID>(Soldier2ID((S)));
#define INJ_VEC3(D, S) INJ_FLOAT(D, (S).x); INJ_FLOAT(D, (S).y); INJ_FLOAT(D, (S).z);

#define EXTR_STR(S, D, Size)  (S).readArray<char>((D), (Size));
#define EXTR_BOOLA(S, D, Size) (S).readArray<BOOLEAN>((D), (Size));
#define EXTR_I8A(S, D, Size)  (S).readArray<INT8>((D), (Size));
#define EXTR_U8A(S, D, Size)  (S).readArray<UINT8>((D), (Size));
#define EXTR_I16A(S, D, Size) (S).readArray<INT16>((D), (Size));
#define EXTR_U16A(S, D, Size) (S).readArray<UINT16>((D), (Size));
#define EXTR_I32A(S, D, Size) (S).readArray<INT32>((D), (Size));
#define EXTR_BOOL(S, D)   (D) = (S).read<BOOLEAN>();
#define EXTR_BYTE(S, D)   (D) = (S).read<BYTE>();
#define EXTR_I8(S, D)     (D) = (S).read<INT8>();
#define EXTR_U8(S, D)     (D) = (S).read<UINT8>();
#define EXTR_I16(S, D)    (D) = (S).read<INT16>();
#define EXTR_U16(S, D)    (D) = (S).read<UINT16>();
#define EXTR_I32(S, D)    (D) = (S).read<INT32>();
#define EXTR_U32(S, D)    (D) = (S).read<UINT32>();
#define EXTR_FLOAT(S, D)  (D) = (S).read<FLOAT>();
#define EXTR_DOUBLE(S, D) (D) = (S).read<DOUBLE>();
#define EXTR_PTR(S, D) (D) = NULL; (S).skip(4);
#define EXTR_SKIP(S, Size) (S).skip((Size));
#define EXTR_SKIP_I16(S)   (S).skip(2);
#define EXTR_SKIP_I32(S)   (S).skip(4);
#define EXTR_SKIP_U8(S)    (S).skip(1);
#define EXTR_SOLDIER(S, D) (D) = ID2Soldier((S).read<SoldierID>());
#define EXTR_VEC3(S, D) EXTR_FLOAT(S, (D).x); EXTR_FLOAT(S, (D).y); EXTR_FLOAT(S, (D).z);

#endif
