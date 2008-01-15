#ifndef __PHYS_MATH_H
#define __PHYS_MATH_H

#include "Types.h"


typedef float real;

typedef struct
{
	real x, y, z;

} vector_3;


#ifdef __cplusplus
extern "C" {
#endif


vector_3	VAdd( vector_3 *a, vector_3 *b );
vector_3	VMultScalar( vector_3 *a, real b );
real			VDotProduct( vector_3 *a, vector_3 *b );
vector_3	VGetNormal( vector_3 *a );
vector_3	VCrossProduct( vector_3 *a, vector_3 *b );


static inline BOOLEAN VIsNull(const vector_3* const a)
{
	return a->x == 0 && a->y == 0 && a->z == 0;
}

#ifdef __cplusplus
}
#endif

#endif
