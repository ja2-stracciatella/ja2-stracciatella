#ifdef PRECOMPILEDHEADERS
	#include "TileEngine All.h"
#else
	#include "types.h"
	#include "Isometric Utils.h"
	#include "renderworld.h"
#endif

void MarkMapIndexDirty( INT32 iMapIndex )
{
	gpWorldLevelData[ iMapIndex ].uiFlags |= MAPELEMENT_REDRAW;
	SetRenderFlags( RENDER_FLAG_MARKED );
}

void CenterScreenAtMapIndex( INT32 iMapIndex )
{
	INT16 sWorldX, sWorldY;
	INT16 sCellX, sCellY;

	//Get X, Y world GRID Coordinates
	sWorldY = ( iMapIndex / WORLD_COLS );
	sWorldX = iMapIndex - ( sWorldY * WORLD_COLS );

	//Convert into cell coords
	sCellY = sWorldY * CELL_Y_SIZE;
	sCellX = sWorldX * CELL_X_SIZE;

	//Set the render values, so that the screen will render here next frame.
	gsRenderCenterX = sCellX;
	gsRenderCenterY = sCellY;

	SetRenderFlags( RENDER_FLAG_FULL );
}

void MarkWorldDirty()
{
	SetRenderFlags( RENDER_FLAG_FULL );
}
