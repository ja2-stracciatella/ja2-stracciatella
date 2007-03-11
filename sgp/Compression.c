#include "MemMan.h"
#include "Debug.h"
#include "zlib.h"


// mem allocation functions for ZLIB's purposes

static voidpf ZAlloc(voidpf opaque, uInt items, uInt size)
{
  return( MemAlloc( items * size ) );
}


static void ZFree(voidpf opaque, voidpf address)
{
	MemFree( address );
}

PTR DecompressInit( BYTE * pCompressedData, UINT32 uiDataSize )
{
	z_stream *	pZStream;
	int					iZRetCode;

	// allocate memory for the z_stream struct
	pZStream = MemAlloc( sizeof( z_stream ) );
	if( pZStream == NULL )
	{ // out of memory!
		return( NULL );
	}

	// initial defines
	pZStream->zalloc = ZAlloc;
	pZStream->zfree = ZFree;
	pZStream->opaque = NULL;

	// call the ZLIB init routine
	iZRetCode = inflateInit( pZStream );
	if( iZRetCode != Z_OK )
	{ // ZLIB init error!
		MemFree( pZStream );
		return( NULL );
	}

	// set up our parameters
	pZStream->next_in = pCompressedData;
	pZStream->avail_in = uiDataSize;
	return( (PTR) pZStream );
}

UINT32 Decompress( PTR pDecompPtr, BYTE * pBuffer, UINT32 uiBufferLen )
{
	int					iZRetCode;
	z_stream *	pZStream = (z_stream *) pDecompPtr;

	// these assertions is in here to ensure that we get passed a proper z_stream pointer
	Assert( pZStream != NULL );
	Assert( pZStream->zalloc == ZAlloc );

	if (pZStream->avail_in == 0)
	{ // There is nothing left to decompress!
		return( 0 );
	}

	// set up the z_stream with our parameters
	pZStream->next_out = pBuffer;
	pZStream->avail_out = uiBufferLen;

	// decompress!
	iZRetCode = inflate( pZStream, Z_PARTIAL_FLUSH );
	Assert( iZRetCode == Z_OK || iZRetCode == Z_STREAM_END );

	return( uiBufferLen - pZStream->avail_out );
}

void DecompressFini( PTR pDecompPtr )
{
	z_stream *	pZStream = (z_stream *) pDecompPtr;

	// these assertions is in here to ensure that we get passed a proper z_stream pointer
	Assert( pZStream != NULL );
	Assert( pZStream->zalloc == ZAlloc );

	inflateEnd( pZStream );
	MemFree( pZStream );
}
