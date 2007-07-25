#if !defined( COMPRESSION_H )
#define COMPRESSION_H

#if defined WITH_ZLIB

#include "Types.h"

// Notes on how to use these functions without getting your hands dirty:

// To decompress:
//
// 1) call DecompressInit() with a pointer to your compressed data, and the
// size of that compressed data.  DecompressInit() returns a "decompression
// pointer" that you should pass to Decompress() and DecompressFini()
//
// 2) call Decompress() with the decompression pointer, a pointer to a
// buffer for decompressed data, and the length of that buffer.  If the
// buffer is not large enough to hold all of the decompressed data,
// Decompress() will fill it completely, and you can call Decompress() again
// to continue your decompression.  You are responsible for knowing the
// size your data will be after decompression.  (The STI/STCI file format
// records your original data size for you...)  Decompress() returns the
// number of bytes of output.
//
// 3) call DecompressFini() with the decompression pointer when you're done

PTR DecompressInit( BYTE * pCompressedData, UINT32 uiDataSize );
UINT32 Decompress( PTR pDecompPtr, BYTE * pBuffer, UINT32 uiBufferLen );
void DecompressFini( PTR pDecompPtr );

#endif

#endif
