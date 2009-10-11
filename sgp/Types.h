#ifndef __TYPES_
#define __TYPES_

#include <SDL_video.h>

#include "SGPStrings.h"


#if defined DUTCH + defined ENGLISH + defined FRENCH + defined GERMAN + defined ITALIAN + defined POLISH + defined RUSSIAN + defined RUSSIAN_GOLD != 1
#	error Exactly one of DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN or RUSSIAN_GOLD must be defined.
#endif


#include <stdlib.h> // for abort()
#define UNIMPLEMENTED \
	fprintf(stderr, "===> %s:%d: %s() is not implemented\n", __FILE__, __LINE__, __func__); \
	abort();

#ifdef WITH_FIXMES
#	define FIXME \
	fprintf(stderr, "===> %s:%d: %s() FIXME\n", __FILE__, __LINE__, __func__);
#else
#	define FIXME (void)0;
#endif


#if defined(_WIN32) && !defined(_WIN64) // XXX HACK000A
#	define CASSERT(x)
#else
#	define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1];
#endif


#define lengthof(a) (sizeof(a) / sizeof(a[0]))
#define endof(a) ((a) + lengthof(a))


#define __max(a, b) ((a) > (b) ? (a) : (b))
#define __min(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) __max(a, b)
#define MIN(a, b) __min(a, b)

#define FOR_EACHX(type, iter, array, x) for (type* iter = (array); iter != endof((array)); (x), ++iter)
#define FOR_EACH(type, iter, array)     FOR_EACHX(type, iter, (array), (void)0)

template<typename T> static inline void Swap(T& a, T& b)
{
	T t(a);
	a = b;
	b = t;
}


typedef          int  INT;
typedef unsigned int UINT;
typedef unsigned int UINT32;
typedef   signed int  INT32;

// integers
typedef unsigned char   UINT8;
typedef signed char     INT8;
typedef unsigned short  UINT16;
typedef signed short    INT16;
// floats
typedef float           FLOAT;
typedef double          DOUBLE;
// strings
typedef char			      CHAR8;
typedef wchar_t					CHAR16;
// other
typedef unsigned char		BOOLEAN;
typedef void *					PTR;
typedef UINT8						BYTE;
typedef CHAR8						STRING512[512];

#define SGPFILENAME_LEN 100
typedef CHAR8 SGPFILENAME[SGPFILENAME_LEN];


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define BAD_INDEX -1

#define PI 3.1415926

#ifndef NULL
#define NULL 0
#endif

struct SGPBox
{
	INT16 x;
	INT16 y;
	INT16 w;
	INT16 h;
};

struct SGPRect
{
  INT32 iLeft;
  INT32 iTop;
  INT32 iRight;
  INT32 iBottom;
};

struct SGPPoint
{
	INT32 	iX;
	INT32   iY;
};


struct SDL_Color;
typedef SDL_Color SGPPaletteEntry;
CASSERT(sizeof(SGPPaletteEntry) == 4);


typedef UINT32 COLORVAL;

struct AuxObjectData;
struct ETRLEObject;
struct RelTileLoc;
struct SGPImage;

class SGPVObject;
typedef SGPVObject* HVOBJECT;
typedef SGPVObject* Font;

class SGPVSurface;

struct BUTTON_PICS;

struct SGPFile;
typedef SGPFile* HWFILE;


#define TRANSPARENT ((UINT16)0)


#ifdef __cplusplus
#	define ENUM_BITSET(type)                                                                   \
		static inline type operator ~  (type  a)         { return     (type)~(int)a;           } \
		static inline type operator &  (type  a, type b) { return     (type)((int)a & (int)b); } \
		static inline type operator &= (type& a, type b) { return a = (type)((int)a & (int)b); } \
		static inline type operator |  (type  a, type b) { return     (type)((int)a | (int)b); } \
		static inline type operator |= (type& a, type b) { return a = (type)((int)a | (int)b); }
#else
#	define ENUM_BITSET(type)
#endif

#endif
