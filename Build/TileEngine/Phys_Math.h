#ifndef __PHYS_MATH_H
#define __PHYS_MATH_H

typedef float real;

typedef struct
{
	real x, y, z;

} vector_3;


#ifdef __cplusplus
extern "C" {
#endif


vector_3	VSubtract( vector_3 *a, vector_3 *b );
vector_3	VAdd( vector_3 *a, vector_3 *b );
vector_3	VMultScalar( vector_3 *a, real b );
vector_3	VDivScalar( vector_3 *a, real b );
real			VDotProduct( vector_3 *a, vector_3 *b );
real			VGetLength( vector_3 *a );
vector_3	VGetNormal( vector_3 *a );
vector_3	VCrossProduct( vector_3 *a, vector_3 *b );

#ifdef __cplusplus
}
#endif

#endif
