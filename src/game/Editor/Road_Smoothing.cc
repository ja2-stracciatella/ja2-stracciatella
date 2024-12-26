#include "TileDef.h"
#include "Types.h"
#include "Road_Smoothing.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Editor_Undo.h"
#include <array>


struct MACROSTRUCT
{
	INT16 sMacroID;
	INT16 sOffset;
};
//road macros

//These define the macros for the 32 road pieces.  The column contains the macro ID and
//the second contains the gridno offset from the anchor position (where the user clicks in the world to
//place the road).  The actual index of the array refers to the offset from ROADPIECE001.
static constexpr MACROSTRUCT gRoadMacros[]
{
	//left 1
	{L1, -2		},
	{L1, -162	},
	{L1, -322	},
	{L1, -1		},
	{L1, -161	},
	{L1, -321	},
	{L1, 0		},
	{L1, -160	},
	{L1, -320	},
	//right 1
	{R1, -2		},
	{R1, -162	},
	{R1, -322	},
	{R1, -1		},
	{R1, -161	},
	{R1, -321	},
	{R1, 0		},
	{R1, -160	},
	{R1, -320	},
	//bottom 1
	{B1, -2		},
	{B1, -162	},
	{B1, -322	},
	{B1, -1		},
	{B1, -161	},
	{B1, -321	},
	{B1, 0		},
	{B1, -160	},
	{B1, -320	},
	//top 1
	{T1, -2		},
	{T1, -162	},
	{T1, -322	},
	{T1, -1		},
	{T1, -161	},
	{T1, -321	},
	{T1, 0		},
	{T1, -160	},
	{T1, -320	},
	//top half of top-right corner
	{TTR, -4		},
	{TTR, -164	},
	{TTR, -324	},
	{TTR, -3		},
	{TTR, -163	},
	{TTR, -323	},
	{TTR, -2		},
	{TTR, -162	},
	{TTR, -1		},
	{TTR, -161	},
	{TTR, 0		},
	//bottom half of top-right corner
	{BTR, -5		},
	{BTR, -165	},
	{BTR, -325	},
	{BTR, -4		}, //50
	{BTR, -164	},
	{BTR, -324	},
	{BTR, -3		},
	{BTR, -163	},
	{BTR, -323	},
	{BTR, -2		},
	{BTR, -162	},
	{BTR, -322	},
	{BTR, -1		},
	{BTR, -161	},
	{BTR, -321	},
	{BTR, 0			},
	{BTR, -160	},
	//left half of bottom-left corner
	{LBL, -322	},
	{LBL, -482	},
	{LBL, -642	},
	{LBL, -161	},
	{LBL, -321	},
	{LBL, -481	},
	{LBL, -641	},
	{LBL, 0			},
	{LBL, -160	},
	{LBL, -320	},
	{LBL, -480	},
	{LBL, -640	},
	//right half of bottom-left corner
	{RBL, -162	},
	{RBL, -322	},
	{RBL, -482	},
	{RBL, -642	},
	{RBL, -802	},
	{RBL, -1		},
	{RBL, -161	},
	{RBL, -321	},
	{RBL, -481	},
	{RBL, -641	},
	{RBL, -801	},
	{RBL, 0			},
	{RBL, -160	},
	{RBL, -320	},
	{RBL, -480	},
	{RBL, -640	},
	{RBL, -800	},
	//left half of the top-left corner
	{LTL, -2		},
	{LTL, -162	},
	{LTL, -322	},
	{LTL, -1		},
	{LTL, -161	},
	{LTL, -321	},
	{LTL, -481	},
	{LTL, 0			},	//100
	{LTL, -160	},
	{LTL, -320	},
	{LTL, -480	},
	{LTL, -640	},
	//right half of top-left corner
	{RTL, -2		},
	{RTL, -162	},
	{RTL, -322	},
	{RTL, -482	},
	{RTL, -642	},
	{RTL, -802  },
	{RTL, -1		},
	{RTL, -161	},
	{RTL, -321	},
	{RTL, -481	},
	{RTL, -641	},
	{RTL, -801  },
	{RTL, 0 		},
	{RTL, -160	},
	{RTL, -320	},
	{RTL, -480	},
	{RTL, -640	},
	{RTL, -800  },
	//right half of top-left corner
	{RBR, 159		},
	{RBR, -1		},
	{RBR, -161	},
	{RBR, -321	},
	{RBR, -481	},
	{RBR, -641	},
	{RBR, 0 		},
	{RBR, -160	},
	{RBR, -320	},
	{RBR, -480	},
	{RBR, -640	},
	{RBR, -159	},
	{RBR, -319	},
	{RBR, -479	},
	{RBR, -639	},
	//right half of top-left corner
	{LBR, 158		},
	{LBR, -2		},
	{LBR, -162	},
	{LBR, -322	},
	{LBR, -482	},
	{LBR, -642	},
	{LBR, 159		},
	{LBR, -1		},
	{LBR, -161	},
	{LBR, -321	},
	{LBR, -481	},
	{LBR, -641	},
	{LBR, 160		}, //150
	{LBR, 0 		},
	{LBR, -160	},
	{LBR, -320	},
	{LBR, -480	},
	{LBR, -640	},
	//left 2
	{L2, -2		},
	{L2, -162	},
	{L2, -322	},
	{L2, -1		},
	{L2, -161	},
	{L2, -321	},
	{L2, 0		},
	{L2, -160	},
	{L2, -320	},
	//right 2
	{R2, -2		},
	{R2, -162	},
	{R2, -322	},
	{R2, -1		},
	{R2, -161	},
	{R2, -321	},
	{R2, 0		},
	{R2, -160	},
	{R2, -320	},
	//left 3
	{L3, -2		},
	{L3, -162	},
	{L3, -322	},
	{L3, -1		},
	{L3, -161	},
	{L3, -321	},
	{L3, 0		},
	{L3, -160	},
	{L3, -320	},
	//right 3
	{R3, -2		},
	{R3, -162	},
	{R3, -322	},
	{R3, -1		},
	{R3, -161	},
	{R3, -321	},
	{R3, 0		},
	{R3, -160	},
	{R3, -320	},
	//bottom 2
	{B2, -2		},
	{B2, -162	},
	{B2, -322	},
	{B2, -1		},
	{B2, -161	},
	{B2, -321	},
	{B2, 0		},
	{B2, -160	},
	{B2, -320	}, //200
	//top 2
	{T2, -2		},
	{T2, -162	},
	{T2, -322	},
	{T2, -1		},
	{T2, -161	},
	{T2, -321	},
	{T2, 0		},
	{T2, -160	},
	{T2, -320	},
	//bottom 3
	{B3, -2		},
	{B3, -162	},
	{B3, -322	},
	{B3, -1		},
	{B3, -161	},
	{B3, -321	},
	{B3, 0		},
	{B3, -160	},
	{B3, -320	},
	//top 3
	{T3, -2		},
	{T3, -162	},
	{T3, -322	},
	{T3, -1		},
	{T3, -161	},
	{T3, -321	},
	{T3, 0		},
	{T3, -160	},
	{T3, -320	},
	//bottom interior
	{BI, -2		},
	{BI, -162	},
	{BI, -322	},
	{BI, -1		},
	{BI, -161	},
	{BI, -321	},
	{BI, 0		},
	{BI, -160	},
	{BI, -320	},
	//left interior
	{LI, -2		},
	{LI, -162	},
	{LI, -322	},
	{LI, -1		},
	{LI, -161	},
	{LI, -321	},
	{LI, 0		},
	{LI, -160	},
	{LI, -320	},
	//top interior
	{TI, -2		},
	{TI, -162	},
	{TI, -322	},
	{TI, -1		},
	{TI, -161	}, //250
	{TI, -321	},
	{TI, 0		},
	{TI, -160	},
	{TI, -320	},
	//right interior
	{RI, -2		}, //0
	{RI, -162	},
	{RI, -322	},
	{RI, -1		},
	{RI, -161	},
	{RI, -321	},
	{RI, 0		},
	{RI, -160	},
	{RI, -320	},
	//left 4
	{L4, -2		},
	{L4, -162	},
	{L4, -322	},
	{L4, -1		},
	{L4, -161	},
	{L4, -321	},
	{L4, 0		},
	{L4, -160	},
	{L4, -320	},
	//right 4
	{R4, -2		},
	{R4, -162	},
	{R4, -322	},
	{R4, -1		},
	{R4, -161	},
	{R4, -321	},
	{R4, 0		},
	{R4, -160	},
	{R4, -320	},
	//bottom 4
	{B4, -2		},
	{B4, -162	},
	{B4, -322	},
	{B4, -1		},
	{B4, -161	},
	{B4, -321	},
	{B4, 0		},
	{B4, -160	},
	{B4, -320	},
	//top 4
	{T4, -2		},
	{T4, -162	},
	{T4, -322	},
	{T4, -1		},
	{T4, -161	},
	{T4, -321	},
	{T4, 0		},
	{T4, -160	},
	{T4, -320	},
	//right edge (end of road)
	{RE, 0		},
	{RE, -160		},
	{RE, -320		},
	//left edge
	{LE, 0		},
	{LE, -160		},
	{LE, -320		},
	//bottom edge
	{BE, -2		},
	{BE, -1	},
	{BE, 0	},
	//top edge
	{TE, -2		},
	{TE, -1	},
	{TE, 0	}
};

static constexpr std::array<INT16, NUM_ROAD_MACROS> gsRoadMacroStartIndex{ []
{
	//A simple optimization function that calculates the first index in the
	//large database for the particular macro ID.
	std::array<INT16, NUM_ROAD_MACROS> startIndices{};

	INT16 sMacro = 0;
	constexpr INT16 end = std::size(gRoadMacros);
	for (INT16 i = 0; i < end; i++)
	{
		if (gRoadMacros[i].sMacroID == sMacro)
		{
			startIndices[sMacro] = i;
			sMacro++;
		}
	}

	return startIndices;
}() };


//Road macros vary in size from 3 gridnos to 18 gridnos.  Using the anchor gridno based off of the original
//road system, this function will place the new macro (consisting of multiple road pieces in multiple
//gridnos).
void PlaceRoadMacroAtGridNo( INT32 iMapIndex, INT32 iMacroID )
{
	INT32 i;
	i = gsRoadMacroStartIndex[ iMacroID ];
	while( gRoadMacros[ i ].sMacroID == iMacroID )
	{
		AddToUndoList( iMapIndex + gRoadMacros[ i ].sOffset );
		RemoveAllObjectsOfTypeRange( i, ROADPIECES, ROADPIECES );
		UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ROADPIECES, i + 1);
		AddObjectToHead( iMapIndex + gRoadMacros[ i ].sOffset, usTileIndex );
		i++;
	}
}

//The old road system used multi-tiled roads as a single image.  The new road system has taken these large
//pieces and chopped them up into single tiled images (to mitigate lighting problems).  Some of the larger
//road pieces turned into 18 smaller pieces.  So this function will go analyse the world, and replaces any
//locations containing the original road tile information, delete it, and replace it by inserting it's
//equivalent macro.
void ReplaceObsoleteRoads()
{
	INT32 i;
	INT32 iMacro;
	LEVELNODE *pObject;
	BOOLEAN fRoadExistsAtGridNo;
	for( i = 0; i < WORLD_MAX; i++ )
	{
		pObject = gpWorldLevelData[ i ].pObjectHead;
		fRoadExistsAtGridNo = false;
		while( pObject )
		{
			if( pObject->usIndex >= FIRSTROAD1 && pObject->usIndex <= FIRSTROAD32 )
			{
				fRoadExistsAtGridNo = true;
				iMacro = pObject->usIndex - FIRSTROAD1;
				PlaceRoadMacroAtGridNo( i, iMacro );
			}
			pObject = pObject->pNext;
		}
		if( fRoadExistsAtGridNo )
		{
			RemoveAllObjectsOfTypeRange( i, FIRSTROAD, FIRSTROAD );
		}
	}
}
