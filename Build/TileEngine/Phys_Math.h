#ifndef __PHYS_MATH_H
#define __PHYS_MATH_H

#include "Types.h"


typedef float real;

struct vector_3
{
	real x, y, z;
};


vector_3	VAdd( vector_3 *a, vector_3 *b );
vector_3	VMultScalar( vector_3 *a, real b );
real			VDotProduct( vector_3 *a, vector_3 *b );
vector_3	VGetNormal( vector_3 *a );

#endif
