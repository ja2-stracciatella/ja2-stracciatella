#ifdef PRECOMPILEDHEADERS
	#include "TileEngine All.h"
#else
	#include "types.h"
	#include "Fog Of War.h"
	#include "Isometric Utils.h"
	#include "worldman.h"
	#include "Simple Render Utils.h"
	#include "Renderworld.h"
	#include "lighting.h"
#endif

//When line of sight reaches a gridno, and there is a light there, it turns it on.
//This is only done in the cave levels.
void RemoveFogFromGridNo( UINT32 uiGridNo )
{
	INT32 i;
	INT32 x, y;
	UINT32 uiAdjacentGridNo = 0;
	x = uiGridNo % WORLD_COLS;
	y = uiGridNo / WORLD_COLS;
	for( i = 0; i < MAX_LIGHT_SPRITES; i++ )
	{
		if( LightSprites[ i ].iX == x && LightSprites[ i ].iY == y )
		{
			if( !(LightSprites[ i ].uiFlags & LIGHT_SPR_ON) )
			{
				LightSpritePower( i, TRUE );
				LightDraw(	LightSprites[i].uiLightType, LightSprites[i].iTemplate, LightSprites[i].iX, LightSprites[i].iY, i );
				MarkWorldDirty();
				return;
			}
		}
	}
}
