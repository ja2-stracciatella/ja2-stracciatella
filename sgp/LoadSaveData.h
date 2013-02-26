#ifndef LOADSAVEDATA_H
#define LOADSAVEDATA_H

#include "Types.h"


static inline void ExtractWideString16(wchar_t* Dst, const BYTE* Src, size_t Size)
{
	const UINT16* S = (const UINT16*)Src;
	for (size_t i = 0; i < Size; i++)
	{
		wchar_t c = S[i];
#if defined RUSSIAN || defined RUSSIAN_GOLD
		/* The Russian data files are incorrectly encoded. The original texts seem to
		 * be encoded in CP1251, but then they were converted from CP1252 (!) to
		 * UTF-16 to store them in the data files. Undo this damage here. */
		if (0xC0 <= c && c <= 0xFF) c += 0x350;
#endif
		Dst[i] = c;
	}
}


static inline void ExtractWideString32(wchar_t* Dst, const BYTE* Src, size_t Size)
{
	const UINT32* S = (const UINT32*)Src;
	for (size_t i = 0; i < Size; i++) Dst[i] = S[i];
}


#define EXTR_WSTR16(S, D, Size) ExtractWideString16((D), (S), Size); (S) += (Size) * sizeof(UINT16);
#define EXTR_WSTR32(S, D, Size) ExtractWideString32((D), (S), Size); (S) += (Size) * sizeof(UINT32);
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
#define EXTR_SOLDIER(S, D) (D) = ID2Soldier(*(const SoldierID*)(S)); (S) += sizeof(SoldierID);
#define EXTR_VEC3(S, D) EXTR_FLOAT(S, (D).x); EXTR_FLOAT(S, (D).y); EXTR_FLOAT(S, (D).z);


static inline void InjectWideString16(const wchar_t* Src, BYTE* Dst, size_t Size)
{
	UINT16* D = (UINT16*)Dst;
	for (size_t i = 0; i < Size; i++) D[i] = Src[i];
}


static inline void InjectWideString32(const wchar_t* Src, BYTE* Dst, size_t Size)
{
	UINT32* D = (UINT32*)Dst;
	for (size_t i = 0; i < Size; i++) D[i] = Src[i];
}


#define INJ_WSTR16(D, S, Size) InjectWideString16((S), (D), (Size)); (D) += (Size) * sizeof(UINT16);
#define INJ_WSTR32(D, S, Size) InjectWideString32((S), (D), (Size)); (D) += (Size) * sizeof(UINT32);
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
#define INJ_SOLDIER(D, S) *(SoldierID*)(D) = Soldier2ID((S)); (D) += sizeof(SoldierID);
#define INJ_VEC3(D, S) INJ_FLOAT(D, (S).x); INJ_FLOAT(D, (S).y); INJ_FLOAT(D, (S).z);

#endif
