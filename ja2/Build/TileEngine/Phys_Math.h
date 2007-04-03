#ifndef __PHYS_MATH_H
#define __PHYS_MATH_H


// explicit dependencies
#include <math.h>		// for sqrt
#include <float.h>

typedef float real;

#define		 REAL_MAX				FLT_MAX
#define		 REAL_MIN				FLT_MIN

#define			PI2						3.14159265358979323846
#define			Epsilon2 			0.00001


typedef struct
{
	real x, y, z;

} vector_3;

typedef struct
{
	real aElements[3][3];

} matrix_3x3;


#define RADIANS_FROM( d )						( ( d * PI2 ) / 180 )
#define DEGREES_FROM( r )						( ( r * 180 ) / PI2 )

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
