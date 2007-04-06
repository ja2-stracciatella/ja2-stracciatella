#ifndef __TYPES_
#define __TYPES_

#include <stdlib.h>
#include "SGPStrings.h"


#define UNIMPLEMENTED() \
	fprintf(stderr, "===> %s:%d: %s() is not implemented\n", __FILE__, __LINE__, __func__); \
	abort();
#define FIXME \
	fprintf(stderr, "===> %s:%d: %s() FIXME\n", __FILE__, __LINE__, __func__);


#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1];


#define lengthof(a) (sizeof(a) / sizeof(a[0]))


#define __max(a, b) ((a) > (b) ? (a) : (b))
#define __min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) __max(a, b)
#define min(a, b) __min(a, b)


// build defines header....
#include "BuildDefines.h"


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
typedef char            CHAR;
typedef char			      CHAR8;
typedef wchar_t					CHAR16;
typedef char *			    STR;
typedef char *					STR8;
typedef wchar_t *				STR16;
// other
typedef unsigned char		BOOLEAN;
typedef void *					PTR;
typedef UINT8						BYTE;
typedef CHAR8						STRING512[512];
typedef UINT32					HWFILE;

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

typedef struct
{
  INT32 iLeft;
  INT32 iTop;
  INT32 iRight;
  INT32 iBottom;

} SGPRect;

typedef struct
{
	INT32 	iX;
	INT32   iY;

} SGPPoint;

#endif
