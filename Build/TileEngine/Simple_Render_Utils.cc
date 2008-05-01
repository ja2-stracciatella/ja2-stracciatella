#include "Simple_Render_Utils.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"


void MarkMapIndexDirty( INT32 iMapIndex )
{
	gpWorldLevelData[ iMapIndex ].uiFlags |= MAPELEMENT_REDRAW;
	SetRenderFlags( RENDER_FLAG_MARKED );
}


void CenterScreenAtMapIndex(const INT32 iMapIndex)
{
	//Set the render values, so that the screen will render here next frame.
	ConvertGridNoToCellXY(iMapIndex, &gsRenderCenterX, &gsRenderCenterY);
	SetRenderFlags(RENDER_FLAG_FULL);
}


void MarkWorldDirty(void)
{
	SetRenderFlags( RENDER_FLAG_FULL );
}
