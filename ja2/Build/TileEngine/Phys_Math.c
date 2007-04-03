#include "Phys_Math.h"
#include <memory.h>


vector_3 VSubtract( vector_3 *a, vector_3 *b )
{
	vector_3 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;

	return( c );
}

vector_3 VAdd( vector_3 *a, vector_3 *b )
{
	vector_3 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;

	return( c );
}

vector_3 VMultScalar( vector_3 *a, real b )
{
	vector_3 c;

	c.x = a->x * b;
	c.y = a->y * b;
	c.z = a->z * b;

	return( c );
}

vector_3 VDivScalar( vector_3 *a, real b )
{
	vector_3 c;

	c.x = a->x / b;
	c.y = a->y / b;
	c.z = a->z / b;

	return( c );
}

real VDotProduct( vector_3 *a, vector_3 *b )
{
	return ( ( a->x * b->x ) + ( a->y * b->y ) + ( a->z * b->z ) );
}


static real VPerpDotProduct(vector_3* a, vector_3* b)
{
	return ( ( a->x * b->x ) - ( a->y * b->y ) - ( a->z * b->z ) );
}


vector_3 VCrossProduct( vector_3 *a, vector_3 *b )
{
	vector_3 c;

	c.x = ( a->y * b->z ) - ( a->z *  b->y );
	c.y = ( a->x * b->z ) - ( a->z *  b->x );
	c.z = ( a->x * b->y ) - ( a->y *  b->x );

	return( c );
}


static vector_3 VGetPerpendicular(vector_3* a)
{
	vector_3 c;

	c.x = -a->y;
	c.y =  a->x;
	c.z =  a->z;

	return( c );
}

real VGetLength( vector_3 *a )
{
	return( (real) sqrt( ( a->x * a->x ) + ( a->y * a->y ) + ( a->z * a->z ) ) );
}


vector_3 VGetNormal( vector_3 *a )
{
	vector_3	c;
	real OneOverLength, Length;

	Length = VGetLength( a );

	if ( Length == 0 )
	{
		c.x = 0;
		c.y = 0;
		c.z = 0;
	}
	else
	{
		OneOverLength = 1/Length;

		c.x = OneOverLength * a->x;
		c.y = OneOverLength * a->y;
		c.z = OneOverLength * a->z;
	}
	return ( c );
}
