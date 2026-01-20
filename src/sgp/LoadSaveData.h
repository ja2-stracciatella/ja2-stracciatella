#ifndef LOADSAVEDATA_H
#define LOADSAVEDATA_H

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string_theory/string>

template<typename T>
concept memcpyable = std::is_trivially_copyable_v<T>;

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
	explicit DataWriter(uint8_t* buf);

	/** Write UTF-8 encoded string.
	 * @param str      String to write
	 * @param numChars Number of `char` characters to write. */
	void writeUTF8(const ST::string& str, size_t numChars);

	/** Write UTF-16 encoded string.
	 * @param str      String to write
	 * @param numChars Number of `char16_t` characters to write. */
	void writeUTF16(const ST::string& str, size_t numChars);

	void writeU8 (uint8_t  value);        /**< Write uint8_t */
	void writeU16(uint16_t value);        /**< Write uint16_t */
	void writeU32(uint32_t value);        /**< Write uint32_t */

	/* Write a value. */
	void write(memcpyable auto const& value)
	{
		constexpr size_t numBytes = sizeof(value);
		std::memcpy(m_buf, &value, numBytes);
		move(numBytes);
	}

	/* Write an array of values. */
	template<memcpyable T>
	void writeArray(const T* arr, size_t len)
	{
		size_t numBytes = len * sizeof(T);
		std::memcpy(m_buf, arr, numBytes);
		move(numBytes);
	}

	/* Write zeroed bytes. */
	void skip(size_t numBytes);

	/** Get number of the consumed bytes during writing. */
	constexpr size_t getConsumed() const noexcept
	{
		return m_buf - m_original;
	}


protected:
	uint8_t* m_buf;
	uint8_t* m_original;

	/** Move pointer to \a numBytes bytes forward. */
	constexpr void move(size_t const numBytes) noexcept
	{
		m_buf += numBytes;
	}
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
	explicit DataReader(const uint8_t* buf);

	/** Read UTF-8 encoded string.
	 * @param numChars Number of `char` characters to read.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF8(size_t numChars, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	/** Read UTF-16 encoded string.
	 * @param numChars Number of `char16_t` characters to read.
	 * @param isCorrectlyEncoded Used for fixing incorrectly encoded text.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF16(size_t numChars, bool const isCorrectlyEncoded = true, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	/** Read UTF-32 encoded string.
	 * @param numChars Number of `char32_t` characters to read.
	 * @param validation What happens with invalid character sequences. */
	ST::string readUTF32(size_t numChars, ST::utf_validation_t validation = ST_DEFAULT_VALIDATION);

	/** Read a string that is either UTF-32 or UTF-16 encoded.
	 * @param numChars Number of `char32_t` characters to read.
	 * @param stracLinuxFormat true if the data was written using the
	   short-lived file format where string were UTF-32 encoded on Unix
	   platforms or the current, Windows-like file format with UTF-16
	   encoded strings.	*/
	auto readString(size_t const numChars, bool const stracLinuxFormat)
	{
		if (stracLinuxFormat) {
			return readUTF32(numChars, ST_DEFAULT_VALIDATION);
		}
		return readUTF16(numChars, true, ST_DEFAULT_VALIDATION);
	}

	uint8_t  readU8();            /**< Read uint8_t */
	uint16_t readU16();           /**< Read uint16_t */
	uint32_t readU32();           /**< Read uint32_t */

	/* Read raw bytes. */
	void readBytes(uint8_t* bytes, size_t numBytes);

	/* Read a value. */
	template<memcpyable T>
	T read()
	{
		constexpr size_t numBytes = sizeof(T);
		T value;
		std::memcpy(&value, m_buf, numBytes);
		move(numBytes);
		return value;
	}

	/* Read an array of values. */
	template<memcpyable T>
	void readArray(T* arr, size_t len)
	{
		size_t numBytes = len * sizeof(T);
		std::memcpy(arr, m_buf, numBytes);
		move(numBytes);
	}

	/* Read and discard bytes. */
	constexpr void skip(size_t const numBytes) noexcept
	{
		move(numBytes);
	}

	/** Get number of the consumed bytes during reading. */
	constexpr size_t getConsumed() const noexcept
	{
		return m_buf - m_original;
	}

protected:
	const uint8_t* m_buf;
	const uint8_t* m_original;

	/** Move pointer to \a numBytes bytes forward. */
	constexpr void move(size_t numBytes) noexcept
	{
		m_buf += numBytes;
	}
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
#define INJ_U8(D, S)     (D).writeU8((S));
#define INJ_I16(D, S)    (D).write<INT16>((S));
#define INJ_U16(D, S)    (D).writeU16((S));
#define INJ_I32(D, S)    (D).write<INT32>((S));
#define INJ_U32(D, S)    (D).writeU32((S));
#define INJ_FLOAT(D, S)  (D).write<FLOAT>((S));
#define INJ_DOUBLE(D, S) (D).write<DOUBLE>((S));
#define INJ_PTR(D, S) INJ_SKIP(D, 4)
#define INJ_SKIP(D, Size) (D).skip((Size));
#define INJ_SKIP_I16(D)   (D).skip(2);
#define INJ_SKIP_I32(D)   (D).skip(4);
#define INJ_SKIP_U8(D)    (D).skip(1);
#define INJ_SOLDIER(D, S) (D).write<SoldierID>(Soldier2ID((S)));
#define INJ_VEC3(D, S) INJ_FLOAT(D, (S).x); INJ_FLOAT(D, (S).y); INJ_FLOAT(D, (S).z);
static inline void INJ_AUTO(DataWriter & D, auto const& S) { D.write(S); }

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
#define EXTR_U8(S, D)     (D) = (S).readU8();
#define EXTR_I16(S, D)    (D) = (S).read<INT16>();
#define EXTR_U16(S, D)    (D) = (S).readU16();
#define EXTR_I32(S, D)    (D) = (S).read<INT32>();
#define EXTR_U32(S, D)    (D) = (S).readU32();
#define EXTR_FLOAT(S, D)  (D) = (S).read<FLOAT>();
#define EXTR_DOUBLE(S, D) (D) = (S).read<DOUBLE>();
#define EXTR_PTR(S, D) (D) = NULL; (S).skip(4);
#define EXTR_ENUM_U8(S, D, T)   (D) = static_cast<T>((S).readU8());
#define EXTR_SKIP(S, Size) (S).skip((Size));
#define EXTR_SKIP_I16(S)   (S).skip(2);
#define EXTR_SKIP_I32(S)   (S).skip(4);
#define EXTR_SKIP_U8(S)    (S).skip(1);
#define EXTR_SOLDIER(S, D) (D) = ID2Soldier((S).read<SoldierID>());
#define EXTR_VEC3(S, D) EXTR_FLOAT(S, (D).x); EXTR_FLOAT(S, (D).y); EXTR_FLOAT(S, (D).z);
static inline void EXTR_AUTO(DataReader & S, auto & D) { D = S.read<std::remove_reference_t<decltype(D)>>(); }

#endif
