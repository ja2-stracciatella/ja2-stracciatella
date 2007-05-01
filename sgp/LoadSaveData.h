#ifndef LOADSAVEDATA_H
#define LOADSAVEDATA_H

#include "Types.h"

static inline void ExtractWideString(wchar_t* Dst, const BYTE* Src, size_t Size)
{
	const UINT16* S = (const UINT16*)Src;
	for (size_t i = 0; i < Size; i++) Dst[i] = S[i];
}


#define EXTR_WSTR(S, D, Size) ExtractWideString(D, S, Size); (S) += (Size) * sizeof(UINT16);
#define EXTR_STR(S, D, Size)  memcpy(D, S, Size); (S) += (Size);
#define EXTR_I8A(S, D, Size)  memcpy(D, S, Size); (S) += (Size);
#define EXTR_U8A(S, D, Size)  memcpy(D, S, Size); (S) += (Size);
#define EXTR_I16A(S, D, Size) memcpy(D, S, sizeof(INT16) * Size); (S) += sizeof(INT16) * (Size);
#define EXTR_U16A(S, D, Size) memcpy(D, S, sizeof(UINT16) * Size); (S) += sizeof(UINT16) * (Size);
#define EXTR_BOOL(S, D) (D) = *(const BOOLEAN*)S; (S) += sizeof(BOOLEAN);
#define EXTR_I8(S, D)   (D) = *(const   INT8*)S;  (S) += sizeof(INT8);
#define EXTR_U8(S, D)   (D) = *(const  UINT8*)S;  (S) += sizeof(UINT8);
#define EXTR_I16(S, D)  (D) = *(const   INT16*)S; (S) += sizeof(INT16);
#define EXTR_U16(S, D)  (D) = *(const  UINT16*)S; (S) += sizeof(UINT16);
#define EXTR_I32(S, D)  (D) = *(const   INT32*)S; (S) += sizeof(INT32);
#define EXTR_U32(S, D)  (D) = *(const  UINT32*)S; (S) += sizeof(UINT32);
#define EXTR_PTR(S, D) (D) = NULL; (S) += (4);
#define EXTR_SKIP(S, Size) (S) += (Size);

#endif
