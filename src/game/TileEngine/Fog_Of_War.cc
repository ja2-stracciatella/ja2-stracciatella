#include "Types.h"
#include "Fog_Of_War.h"
#include "Simple_Render_Utils.h"
#include "Lighting.h"
#include "WorldDef.h"


//When line of sight reaches a gridno, and there is a light there, it turns it on.
//This is only done in the cave levels.
void RemoveFogFromGridNo( UINT32 uiGridNo )
{
	INT32 x, y;
	x = uiGridNo % WORLD_COLS;
	y = uiGridNo / WORLD_COLS;
	FOR_EACH_LIGHT_SPRITE(l)
	{
		if (l->iX == x && l->iY == y)
		{
			if (!(l->uiFlags & LIGHT_SPR_ON))
			{
				LightSpritePower(l, true);
				LightDraw(l);
				MarkWorldDirty();
				return;
			}
		}
	}
}
