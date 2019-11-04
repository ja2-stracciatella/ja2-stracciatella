#ifndef LOADSAVEDATA_H
#define LOADSAVEDATA_H

#include <string_theory/string>

#include "Types.h"
#include "UTF8String.h"
#include "IEncodingCorrector.h"

#define EXTR_STR(S, D, Size)  memcpy((D), (S), Size); (S) += (Size);
#define EXTR_BOOLA(S, D, Size) memcpy((D), (S), Size); (S) += (Size);
#define EXTR_I8A(S, D, Size)  memcpy((D), (S), Size); (S) += (Size);
#define EXTR_U8A(S, D, Size)  memcpy((D), (S), Size); (S) += (Size);
#define EXTR_I16A(S, D, Size) memcpy((D), (S), sizeof(INT16)  * Size); (S) += sizeof(INT16)  * (Size);
#define EXTR_U16A(S, D, Size) memcpy((D), (S), sizeof(UINT16) * Size); (S) += sizeof(UINT16) * (Size);
#define EXTR_I32A(S, D, Size) memcpy((D), (S), sizeof(INT32)  * Size); (S) += sizeof(INT32)  * (Size);
#define EXTR_BOOL(S, D)   (D) = *(const BOOLEAN*)(S); (S) += sizeof(BOOLEAN);
#define EXTR_BYTE(S, D)   (D) = *(const    BYTE*)(S); (S) += sizeof(BYTE);
#define EXTR_I8(S, D)     (D) = *(const    INT8*)(S); (S) += sizeof(INT8);
#define EXTR_U8(S, D)     (D) = *(const   UINT8*)(S); (S) += sizeof(UINT8);
#define EXTR_I16(S, D)    (D) = *(const   INT16*)(S); (S) += sizeof(INT16);
#define EXTR_U16(S, D)    (D) = *(const  UINT16*)(S); (S) += sizeof(UINT16);
#define EXTR_I32(S, D)    (D) = *(const   INT32*)(S); (S) += sizeof(INT32);
#define EXTR_U32(S, D)    (D) = *(const  UINT32*)(S); (S) += sizeof(UINT32);
#define EXTR_FLOAT(S, D)  (D) = *(const   FLOAT*)(S); (S) += sizeof(FLOAT);
#define EXTR_DOUBLE(S, D) (D) = *(const  DOUBLE*)(S); (S) += sizeof(DOUBLE);
#define EXTR_PTR(S, D) (D) = NULL; (S) += 4;
#define EXTR_SKIP(S, Size) (S) += (Size);
#define EXTR_SKIP_I16(S)   (S) += (2);
#define EXTR_SKIP_I32(S)   (S) += (4);
#define EXTR_SKIP_U8(S)    (S) += (1);
#define EXTR_SOLDIER(S, D) (D) = ID2Soldier(*(const SoldierID*)(S)); (S) += sizeof(SoldierID);
#define EXTR_VEC3(S, D) EXTR_FLOAT(S, (D).x); EXTR_FLOAT(S, (D).y); EXTR_FLOAT(S, (D).z);


////////////////////////////////////////////////////////////////////////////
// DataWriter
////////////////////////////////////////////////////////////////////////////


/** Encode wchar_t into UTF-16 and write to the buffer.
 * @param string        String to encode
 * @param outputBuf     Output buffer for the encoded string
 * @param charsToWrite  Number of characters to write (at least one trailing 0x0000 will be written) */
void wchar_to_utf16(const wchar_t *string, void *outputBuf, int charsToWrite);

/** Class for serializing data (writing them into stream of bytes). */
class DataWriter
{
public:
	/** Constructor.
	 * @param buf Pointer to the buffer for writing data. */
	DataWriter(void *buf);

	/** Write wchar string into UTF-16 format.
	 *
	 * If \a numChars is bigger then the number of actual characters in the string,
	 * then zeroes will be written to the buffer.
	 *
	 * @param string      String to write
	 * @param numChars    Number of characters to write. */
	void writeStringAsUTF16(const wchar_t *string, int numChars);

	void writeU8 (uint8_t  value);        /**< Write uint8_t */
	void writeU16(uint16_t value);        /**< Write uint16_t */
	void writeU32(uint32_t value);        /**< Write uint32_t */

	/** Get number of the consumed bytes during writing. */
	size_t getConsumed() const;

protected:
	void *m_buf;
	const void *m_original;

	/** Move pointer to \a numBytes bytes forward. */
	void move(int numBytes);

};


////////////////////////////////////////////////////////////////////////////
// DataReader
////////////////////////////////////////////////////////////////////////////


/** Class for reading serializing data. */
class DataReader
{
public:
	/** Constructor.
	 * @param buf Pointer to the buffer for writing data. */
	DataReader(const void *buf);

	/** Read UTF-16 encoded string.
	 * @param numChars Number of characters to read.
	 * @param fixer Optional encoding corrector.  It is used for fixing incorrectly encoded text. */
	ST::string readUTF16(size_t numChars, const IEncodingCorrector *fixer=NULL);

	/** Read UTF-32 encoded string.
	 * @param numChars Number of characters to read. */
	ST::string readUTF32(size_t numChars);

	/** Read UTF-16 encoded string into wide string buffer.
	 * @param buffer Buffer to read data in.
	 * @param numChars Number of characters to read.
	 * @param fixer Optional encoding corrector.  It is used for fixing incorrectly encoded text. */
	void readUTF16(wchar_t *buffer, size_t numChars, const IEncodingCorrector *fixer=NULL);

	/** Read UTF-32 encoded string into wide string buffer.
	 * @param buffer Buffer to read data in.
	 * @param numChars Number of characters to read. */
	void readUTF32(wchar_t *buffer, size_t numChars);

	uint8_t  readU8();            /**< Read uint8_t */
	uint16_t readU16();           /**< Read uint16_t */
	uint32_t readU32();           /**< Read uint32_t */

	/** Get number of the consumed bytes during reading. */
	size_t getConsumed() const;

protected:
	const void *m_buf;
	const void *m_original;

	/** Move pointer to \a numBytes bytes forward. */
	void move(int numBytes);

};


#define INJ_WSTR(D, S, Size) wchar_to_utf16((S), (D), (Size)); (D) += (Size) * sizeof(UINT16);
#define INJ_STR(D, S, Size)  memcpy((D), (S), (Size)); (D) += (Size);
#define INJ_BOOLA(D, S, Size)  memcpy((D), (S), (Size)); (D) += (Size);
#define INJ_I8A(D, S, Size)  memcpy((D), (S), (Size)); (D) += (Size);
#define INJ_U8A(D, S, Size)  memcpy((D), (S), (Size)); (D) += (Size);
#define INJ_I16A(D, S, Size) memcpy((D), (S), sizeof(INT16)  * (Size)); (D) += sizeof(INT16)  * (Size);
#define INJ_U16A(D, S, Size) memcpy((D), (S), sizeof(UINT16) * (Size)); (D) += sizeof(UINT16) * (Size);
#define INJ_I32A(D, S, Size) memcpy((D), (S), sizeof(INT32)  * (Size)); (D) += sizeof(INT32)  * (Size);
#define INJ_BOOL(D, S)   *(BOOLEAN*)(D) = (S); (D) += sizeof(BOOLEAN);
#define INJ_BYTE(D, S)   *(   BYTE*)(D) = (S); (D) += sizeof(BYTE);
#define INJ_I8(D, S)     *(   INT8*)(D) = (S); (D) += sizeof(INT8);
#define INJ_U8(D, S)     *(  UINT8*)(D) = (S); (D) += sizeof(UINT8);
#define INJ_I16(D, S)    *(  INT16*)(D) = (S); (D) += sizeof(INT16);
#define INJ_U16(D, S)    *( UINT16*)(D) = (S); (D) += sizeof(UINT16);
#define INJ_I32(D, S)    *(  INT32*)(D) = (S); (D) += sizeof(INT32);
#define INJ_U32(D, S)    *( UINT32*)(D) = (S); (D) += sizeof(UINT32);
#define INJ_FLOAT(D, S)  *(  FLOAT*)(D) = (S); (D) += sizeof(FLOAT);
#define INJ_DOUBLE(D, S) *( DOUBLE*)(D) = (S); (D) += sizeof(DOUBLE);
#define INJ_PTR(D, S) INJ_SKIP(D, 4)
#define INJ_SKIP(D, Size) memset(D, 0, Size); (D) += Size;
#define INJ_SKIP_I16(D)   memset(D, 0, 2);    (D) += 2;
#define INJ_SKIP_I32(D)   memset(D, 0, 4);    (D) += 4;
#define INJ_SKIP_U8(D)    memset(D, 0, 1);    (D) += 1;
#define INJ_SOLDIER(D, S) *(SoldierID*)(D) = Soldier2ID((S)); (D) += sizeof(SoldierID);
#define INJ_VEC3(D, S) INJ_FLOAT(D, (S).x); INJ_FLOAT(D, (S).y); INJ_FLOAT(D, (S).z);

#endif
