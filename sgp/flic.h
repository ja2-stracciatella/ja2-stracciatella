//**************************************************************************
//
// Filename :	flic.h
//
//	Purpose :	to define some flic stuff
//
// Modification history :
//
//**************************************************************************

#ifndef __FLIC_H
#define __FLIC_H

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "sgp.h"
#include <stdio.h>

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define ErrFlicLibAccess  -1
#define ErrFlicAccess -2
#define ErrFlicSeek -3
#define ErrFlicRead -4
#define ErrFlicBad -5
#define ErrFlicBadFrame -6

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

struct _Flic;

typedef signed char Char;		/* Signed 8 bits. */
typedef unsigned char Uchar;	/* Unsigned 8 bits. */
typedef short Short;			/* Signed 16 bits please. */
typedef unsigned short Ushort;	/* Unsigned 16 bits please. */
typedef long Long;				/* Signed 32 bits. */
typedef unsigned long Ulong;	/* Unsigned 32 bits. */

typedef int Boolean;			/* TRUE or FALSE value. */
typedef int ErrCode;			/* ErrXXX or Success. */
typedef int FileHandle;			/* OS file handle. */

typedef Uchar Pixel;			/* Pixel type. */

typedef ErrCode FlicOpenFunc(struct _Flic *flic, const char *filename);
typedef ErrCode FlicCheckFrameFunc(struct _Flic *);
typedef ErrCode FlicSeekFunc(struct _Flic *, long offset);

typedef struct
{
	Uchar r,g,b;
} Colour;					/* One color map entry r,g,b 0-255. */

typedef struct
{
	Pixel pixels[2];
} Pixels2;					/* For word-oriented run length encoding */

	/* Flic Header */
typedef struct
{
	Long		size;			/* Size of flic including this header. */
	Ushort 	type;			/* Either FLI_TYPE or FLC_TYPE below. */
	Ushort	frames;		/* Number of frames in flic. */
	Ushort	width;		/* Flic width in pixels. */
	Ushort	height;		/* Flic height in pixels. */
	Ushort	depth;		/* Bits per pixel.  (Always 8 now.) */
	Ushort	flags;		/* FLI_FINISHED | FLI_LOOPED ideally. */
	Long 		speed;		/* Delay between frames. */
	Short		reserved1;	/* Set to zero. */
	Short		created1;
	Short		created2;
	//Ulong		created;		/* Date of flic creation. (FLC only.) */
	Short		creator1;
	Short		creator2;
	//Ulong		creator;		/* Serial # of flic creator. (FLC only.) */
	Short		updated1;
	Short		updated2;
	//Ulong		updated;		/* Date of flic update. (FLC only.) */
	Short		updater1;
	Short		updater2;
	//Ulong		updater;		/* Serial # of flic updater. (FLC only.) */
	Ushort	aspect_dx;	/* Width of square rectangle. (FLC only.) */
	Ushort	aspect_dy;	/* Height of square rectangle. (FLC only.) */
	Char 		reserved2[38];	/* Set to zero. */
	Long 		oframe1;			/* Offset to frame 1. (FLC only.) */
	Long 		oframe2;			/* Offset to frame 2. (FLC only.) */
	Char 		reserved3[40];	/* Set to zero. */
} FlicHead;

typedef struct
{
	Pixel *pixels;	/* Set to AOOO:0000 for hardware. */
	int width, height;	/* Dimensions of screen. (320x200) */
        char change_palette;  /*True means that the flic changes the palette */
} FlicScreen;					/* Device specific screen type. */

typedef struct
{
  Ulong max_loop_count,
	loop_count;
  Ushort max_frame_index,
         frame_index;
} FlicFrameStatus;

typedef struct
{
//  lmlib_t *names;
  long offset,
       length;
}FlicLib;

typedef struct _Flic
{
  FlicHead head;	/* Flic file header. */
  FILE *file;		/* File handle. */
  const char *name;		/* Name from flic_open.  Helps error reporting. */
  int xoff,yoff;	/* Offset to display flic at. */

  FlicFrameStatus status;
  FlicScreen screen;
  FlicLib lib;

  FlicOpenFunc *open;
  FlicCheckFrameFunc *check_frame;
  FlicSeekFunc       *seek;

} Flic;

	/* Values for FlicHead.type */
#define FLI_TYPE 0xAF11u	/* 320x200 .FLI type ID */
#define FLC_TYPE 0xAF12u	/* Variable rez .FLC type ID */
	/* Values for FlicHead.flags */
#define FLI_FINISHED 0x0001
#define FLI_LOOPED	 0x0002

	/* Optional Prefix Header */
typedef struct
{
	Long size;		/* Size of prefix including header. */
	Ushort type;	/* Always PREFIX_TYPE. */
	Short chunks;	/* Number of subchunks in prefix. */
	Char reserved[8];/* Always 0. */
} PrefixHead;

/* Value for PrefixHead.type */
#define PREFIX_TYPE  0xF100u

/* Frame Header */
typedef struct
{
	Long size;		/* Size of frame including header. */
	Ushort type;	/* Always FRAME_TYPE */
	Short chunks;	/* Number of chunks in frame. */
	Char reserved[8];/* Always 0. */
} FrameHead;

/* Value for FrameHead.type */
#define FRAME_TYPE 0xF1FAu

	/* Chunk Header */
typedef struct
{
	Long size;		/* Size of chunk including header. */
	Ushort type;	/* Value from ChunkTypes below. */
} ChunkHead;

typedef enum
{
	COLOR_256 = 4,	/* 256 level color pallette info. (FLC only.) */
	DELTA_FLC = 7,	/* Word-oriented delta compression. (FLC only.) */
	COLOR_64 = 11,	/* 64 level color pallette info. */
	DELTA_FLI = 12,	/* Byte-oriented delta compression. */
	BLACK_FRAME = 13,		/* whole frame is color 0 */
	BYTE_RUN = 15,	/* Byte run-length compression. */
	LITERAL = 16,	/* Uncompressed pixels. */
	PSTAMP = 18,	/* "Postage stamp" chunk. (FLC only.) */
} ChunkTypes;

//**************************************************************************
//
//				Function prototypes.
//
//**************************************************************************

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void		FlicInit(Flic *flic, unsigned screen_width, unsigned screen_height, char change_palette, char *Buffer);
ErrCode	FlicOpen(Flic *flic, const char *filename);
void		FlicSetOrigin(Flic *flic, unsigned x, unsigned y);
ErrCode	FlicPlay(Flic *flic, Ulong max_loop);
void		FlicClose(Flic *flic);
void		FlicSeekFirst(Flic *flic);
int		  FlicAdvance(Flic *flic, BOOL fDecode);
int		  FlicAdvanceNoDecode(Flic *flic);
int     FlicStart(char *filename, int width, int height, char *buffer, Flic *flic, char usepal);
void		FlicStop(Flic *flic);
ErrCode FlicGetStats(char *filename, int width, int height, Flic *flic, int *piBufferSize, int *piColourPalSize);
ErrCode	FlicGetColourPalette(CHAR *filename, int width, int height, CHAR **ppBuffer, INT *piNumColours);
CHAR		*FlicSeekChunk(Flic *flic, INT iFrame, ChunkTypes eType, INT *piChunkSize);
INT		FlicFindByteRunBeforeFrame(Flic *flic, INT iFrame);
ErrCode	FlicFillBitmapData( Flic *flic, INT iPrevFrame, INT iFrame, HBITMAP hBitmap );
ErrCode	FlicFillFrameData( Flic *flic, INT iPrevFrame, INT iFrame, CHAR *, INT * );
void		FlicClearBitmap( HBITMAP hBitmap, INT iColourIndex );
#ifdef __cplusplus
}
#endif /* __cplusplus */

extern char FlicPal[768];
#endif
