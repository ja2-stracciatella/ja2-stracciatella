#include "Campaign_Types.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"


#define A  SAND
#define AR SAND_ROAD
#define C  COASTAL
#define CR COASTAL_ROAD
#define D  DENSE
#define DR DENSE_ROAD
#define E  EDGEOFWORLD
#define F  FARMLAND
#define FR FARMLAND_ROAD
#define G  GROUNDBARRIER
#define H  HILLS
#define HR HILLS_ROAD
#define N  NS_RIVER
#define P  PLAINS
#define PR PLAINS_ROAD
#define R  ROAD
#define S  SPARSE
#define SR SPARSE_ROAD
#define T  TROPICS
#define TR TROPICS_ROAD
#define W  SWAMP
#define WR SWAMP_ROAD
#define WT WATER
#define X  TOWN

static const UINT8 g_traverse_we[][17] =
{
	/*          1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16  */
	/* A */ { E, P, P, E, E, E, P, H, P, X, P, D, D, P, P, E, E },
	/* B */ { E, P, P, S, P, P, P, P, P, R, R, R, R, P, P, P, E },
	/* C */ { E, P, R, P, P, X, R, R, R, P, D, D, P, P, G, P, E },
	/* D */ { E, G, P, P, X, P, R, P, S, P, P, D, P, P, D, P, E },
	/* E */ { E, G, P, P, H, P, P, S, P, W, P, P, R, P, P, E, E },
	/* F */ { E, G, N, P, H, H, P, P, X, P, P, P, P, P, P, E, E },
	/* G */ { E, X, P, R, R, R, R, R, X, R, R, R, P, P, P, P, E },
	/* H */ { E, X, X, P, P, P, H, P, P, A, A, P, R, X, P, P, E },
	/* I */ { E, G, P, P, D, S, P, P, A, A, P, P, S, X, P, P, E },
	/* J */ { E, G, R, W, W, D, D, A, A, A, P, P, S, S, P, E, E },
	/* K */ { E, G, W, W, W, D, R, R, R, R, R, R, R, R, P, E, E },
	/* L */ { E, P, W, W, P, P, P, A, A, P, P, X, P, P, P, E, E },
	/* M */ { E, G, R, P, P, R, P, P, P, P, G, G, G, P, E, E, E },
	/* N */ { E, G, G, X, X, R, R, R, R, R, G, G, E, E, E, E, E },
	/* O */ { E, G, G, X, G, G, G, G, P, G, G, G, G, E, E, E, E },
	/* P */ { E, G, G, G, G, G, G, G, G, G, G, G, G, E, E, E, E }
	/*        ^------------------ always E -------------------^ */
};

static const UINT8 g_traverse_ns[][16] =
{
	/*         1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 */
	/* A  */ { E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E }, // always E
	/* AB */ { P, X, P, E, E, P, P, P, R, P, P, D, P, P, P, E },
	/* CB */ { P, R, S, S, P, P, P, P, R, P, D, D, X, P, P, P },
	/* CD */ { P, P, R, P, X, R, S, P, R, P, D, P, X, P, P, P },
	/* ED */ { G, P, R, H, P, D, R, S, R, P, P, P, R, P, P, E },
	/* EF */ { G, G, R, H, H, P, R, P, R, P, P, R, P, P, P, E },
	/* GF */ { G, P, R, P, P, G, R, X, X, P, P, R, P, P, P, E },
	/* GH */ { X, X, R, P, G, R, P, X, P, A, P, R, P, P, P, P },
	/* IH */ { G, G, R, D, P, R, P, P, A, A, P, S, X, X, P, P },
	/* IJ */ { G, G, R, W, D, R, P, A, A, A, P, P, S, R, P, E },
	/* KJ */ { G, R, P, W, W, R, A, A, R, P, P, P, P, R, P, E },
	/* KL */ { G, R, W, W, D, R, A, A, A, R, R, R, P, P, P, E },
	/* ML */ { G, R, W, P, P, R, P, A, P, R, G, G, P, P, E, E },
	/* MN */ { G, G, R, P, R, P, P, P, P, R, G, G, E, E, E, E },
	/* ON */ { G, G, X, X, G, G, G, P, P, G, G, G, E, E, E, E },
	/* OP */ { G, G, X, G, G, G, G, G, G, G, G, G, G, E, E, E },
	/*  P */ { E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E }  // always E
};

static const UINT8 g_traverse_through[][16] =
{
	/*         1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */
	/* A */ { T , X , P , E , E , H , H , H , X , X , F , D , S , P , D , E  },
	/* B */ { T , X , S , S , F , F , P , F , PR, SR, SR, FR, X , WT, W , D  },
	/* C */ { T , TR, PR, S , X , X , PR, SR, PR, S , D , S , X , WT, W , W  },
	/* D */ { G , X , PR, H , X , F , DR, D , PR, P , S , W , X , WT, X , D  },
	/* E */ { G , T , PR, H , H , P , FR, F , FR, W , S , S , WT, W , DR, E  },
	/* F */ { G , C , PR, H , H , H , HR, X , X , F , P , SR, S , PR, DR, E  },
	/* G */ { X , X , CR, HR, HR, HR, FR, X , X , AR, AR, PR, P , PR, D , W  },
	/* H */ { X , X , X , D , H , HR, P , X , P , A , A , PR, X , X , W , S  },
	/* I */ { G , G , CR, D , S , X , S , X , A , A , P , P , X , X , S , S  },
	/* J */ { G , CR, CR, W , W , SR, S , A , X , A , A , S , S , FR, S , E  },
	/* K */ { G , CR, W , X , W , PR, AR, AR, AR, PR, PR, PR, SR, SR, D , E  },
	/* L */ { C , CR, W , W , D , PR, P , A , P , SR, X , X , C , W , D , E  },
	/* M */ { G , CR, WR, F , DR, DR, P , P , P , TR, G , G , S , W , E , E  },
	/* N */ { G , G , X , X , X , TR, CR, CR, TR, TR, G , G , E , E , E , E  },
	/* O */ { G , G , X , X , G , G , G , T , T , G , G , G , G , E , E , E  },
	/* P */ { G , G , X , G , G , G , G , G , G , G , G , G , G , E , E , E  }
};

static const UINT8 g_travel_rating[][16] =
{
	/*         1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */
	/* A */ { 10, 60,  9,  0,  0,  5,  4, 14, 70, 70, 18,  4, 14, 10,  8,  0 },
	/* B */ { 10, 60,  6,  4, 15, 15, 10, 20, 70, 50, 50, 50, 85, 15, 10,  0 },
	/* C */ {  8, 40, 40, 20, 80, 75, 45, 48, 80, 12,  3,  8, 60, 15,  7,  5 },
	/* D */ {  7,  7, 40, 12, 49, 50, 45, 16, 50, 11,  5, 11, 60, 12,  8,  5 },
	/* E */ {  0, 10, 45, 11,  9, 11, 50, 15, 56, 11,  9, 35, 45,  8,  8,  0 },
	/* F */ {  0,  4, 40,  9,  6,  9, 50, 60, 65, 15,  9, 40,  8, 12,  3,  0 },
	/* G */ { 60,  7, 55, 65, 65, 55, 55, 65, 65, 50, 25, 55, 65, 60, 16,  4 },
	/* H */ { 67, 55, 65,  8, 10, 60,  8, 15, 15,  3,  7, 12, 65, 65, 12,  3 },
	/* I */ {  0,  3, 45,  7,  5, 55, 10,  5,  5,  5, 10, 10, 10, 55, 10,  2 },
	/* J */ {  0, 50, 50,  4,  3, 50,  6, 10, 80, 10, 15, 10, 12, 50, 10,  0 },
	/* K */ {  0, 55,  4, 45, 15, 60, 60, 55, 55, 55, 65, 70, 65, 50,  7,  0 },
	/* L */ {  4, 55,  5, 15, 10, 65, 10,  7,  8,  9, 17, 55, 18,  7,  3,  0 },
	/* M */ {  0, 65, 70, 38, 70, 65, 12,  8,  8,  7,  5, 12,  5,  2,  0,  0 },
	/* N */ {  0,  0, 80, 80, 80, 40, 20, 10,  5,  5,  0,  0,  0,  0,  0,  0 },
	/* O */ {  0,  0, 90, 90,  0,  0,  0,  5,  5,  0,  0,  0,  0,  0,  0,  0 },
	/* P */ {  0,  0,100,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
};

#undef A
#undef AR
#undef C
#undef CR
#undef D
#undef DR
#undef E
#undef F
#undef FR
#undef G
#undef H
#undef HR
#undef N
#undef P
#undef PR
#undef R
#undef S
#undef SR
#undef T
#undef TR
#undef W
#undef WT
#undef WR
#undef X


void InitStrategicMovementCosts()
{
	for (INT32 y = 0; y < 16; ++y)
	{
		for (INT32 x = 0; x < 16; ++x)
		{
			SECTORINFO& s = SectorInfo[SECTOR(x + 1, y + 1)];
			s.ubTravelRating                           = g_travel_rating[   y][x];
			s.ubTraversability[WEST_STRATEGIC_MOVE]    = g_traverse_we[     y][x];
			s.ubTraversability[EAST_STRATEGIC_MOVE]    = g_traverse_we[     y][x + 1];
			s.ubTraversability[NORTH_STRATEGIC_MOVE]   = g_traverse_ns[     y][x];
			s.ubTraversability[SOUTH_STRATEGIC_MOVE]   = g_traverse_ns[ 1 + y][x];
			s.ubTraversability[THROUGH_STRATEGIC_MOVE] = g_traverse_through[y][x];
		}
	}
}


UINT8 GetTraversability( INT16 sStartSector, INT16 sEndSector )
{
	UINT8 ubDirection = 0;
	INT16 sDifference = 0;

	// given start and end sectors
	sDifference = sEndSector - sStartSector;


	if( sDifference == -1 )
	{
		ubDirection = WEST_STRATEGIC_MOVE;
	}
	else if( sDifference == 1 )
	{
		ubDirection = EAST_STRATEGIC_MOVE;
	}
	else if( sDifference == 16 )
	{
		ubDirection = SOUTH_STRATEGIC_MOVE;
	}
	else
	{
		ubDirection = NORTH_STRATEGIC_MOVE;
	}

	return( SectorInfo[ sStartSector ].ubTraversability[ ubDirection ] );
}


bool SectorIsPassable(INT16 const sSector)
{
	// returns true if the sector is impassable in all directions
	const UINT8 t = SectorInfo[sSector].ubTraversability[THROUGH_STRATEGIC_MOVE];
	return t != GROUNDBARRIER && t != EDGEOFWORLD;
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(StrategicMovementCostsTest, asserts)
{
	EXPECT_EQ(lengthof(g_traverse_we), 16);
	EXPECT_EQ(lengthof(g_traverse_ns), 17);
	EXPECT_EQ(lengthof(g_traverse_through), 16);
	EXPECT_EQ(lengthof(g_travel_rating), 16);
}

#endif
