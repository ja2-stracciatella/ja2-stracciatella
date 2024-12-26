#include "TileDef.h"
#include "WorldDef.h"	//LEVELNODE def
#include "WorldMan.h"	//ReplaceStructIndex
#include "SmartMethod.h"
#include "Smoothing_Utils.h"
#include "Editor_Undo.h"


UINT8 gubDoorUIValue = 0;
UINT8 gubWindowUIValue = 0;
UINT8 gubWallUIValue = FIRSTWALL;
UINT8 gubBrokenWallUIValue = 0;

void CalcSmartWallDefault( UINT16 *pusObjIndex, UINT16 *pusUseIndex )
{
	*pusUseIndex = 0;
	*pusObjIndex = gubWallUIValue;
}

void CalcSmartDoorDefault( UINT16 *pusObjIndex, UINT16 *pusUseIndex )
{
	*pusUseIndex = 4 * ( gubDoorUIValue % 2 ); //open or closed -- odd or even
	*pusObjIndex = FIRSTDOOR + gubDoorUIValue / 2;
}

void CalcSmartWindowDefault( UINT16 *pusObjIndex, UINT16 *pusUseIndex )
{
	*pusUseIndex = 44 + gubWindowUIValue; //first exterior top right oriented window
	*pusObjIndex = FIRSTWALL;
}

void CalcSmartBrokenWallDefault( UINT16 *pusObjIndex, UINT16 *pusUseIndex )
{
	switch( gubBrokenWallUIValue )
	{
		case 0:
		case 1:
			*pusUseIndex = 49 + gubBrokenWallUIValue;
			break;
		case 3:
			*pusUseIndex = 62;
			break;
		case 4:
			*pusUseIndex = 64;
			break;
	}
	*pusObjIndex = FIRSTWALL;
}


static UINT16 CalcSmartWindowIndex(UINT16 usWallOrientation)
{
	return (UINT16)(33 + usWallOrientation*3 + gubWindowUIValue);
}


static UINT16 CalcSmartDoorIndex(UINT16 usWallOrientation)
{
	//convert the orientation values as the graphics are in reverse order
	//orientation values:   INSIDE_TOP_LEFT=1,  INSIDE_TOP_RIGHT=2,  OUTSIDE_TOP_LEFT=3, OUTSIDE_TOP_RIGHT=4
	//door graphics order:	INSIDE_TOP_LEFT=15, INSIDE_TOP_RIGHT=10, OUTSIDE_TOP_LEFT=5, OUTSIDE_TOP_RIGHT=0
	usWallOrientation = (4 - usWallOrientation) * 5;
	//4 * (gubDoorUIValue%2) evaluates to +4 if the door is open, 0 if closed
	return (UINT16)( 1 + usWallOrientation + 4 * (gubDoorUIValue % 2) );
}


static UINT16 CalcSmartDoorType(void)
{
	return (UINT16)( FIRSTDOOR + gubDoorUIValue / 2 );
}


static UINT16 CalcSmartBrokenWallIndex(UINT16 usWallOrientation)
{
	if( gubBrokenWallUIValue == 2 ) //the hole in the wall
		return 0xffff;
	if( gubBrokenWallUIValue < 2 ) //broken walls
	{
		//convert the orientation value as the graphics are in a different order.
		//orientation values:   INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2, OUTSIDE_TOP_LEFT=3, OUTSIDE_TOP_RIGHT=4
		//			4			6		8			10
		//door graphics order:  INSIDE_TOP_LEFT=4, INSIDE_TOP_RIGHT=6, OUTSIDE_TOP_LEFT=0, OUTSIDE_TOP_RIGHT=2
		usWallOrientation = usWallOrientation * 2 + 2;
		usWallOrientation -= usWallOrientation > 6 ? 8 : 0;
		return (UINT16)( usWallOrientation + 48 + gubBrokenWallUIValue );
	}

	//cracked and smudged walls

	//convert the orientation value as the graphics are in a different order.
	//orientation values:   INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2, OUTSIDE_TOP_LEFT=3, OUTSIDE_TOP_RIGHT=4
	//door graphics order:  INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2, OUTSIDE_TOP_LEFT=5, OUTSIDE_TOP_RIGHT=6
	usWallOrientation += usWallOrientation > 1 ? 2 : 0;
	usWallOrientation += gubBrokenWallUIValue == 4 ? 2 : 0; //smudged type which is 2 index values higher.
	return (UINT16)( usWallOrientation + 57 );
}

void IncSmartWallUIValue()
{
	gubWallUIValue += gubWallUIValue < LASTWALL ? 1 : -3;
}

void DecSmartWallUIValue()
{
	gubWallUIValue -= gubWallUIValue > FIRSTWALL ? 1 : -3;
}

void IncSmartDoorUIValue()
{
	gubDoorUIValue += gubDoorUIValue < 7 ? 1 : -7;
}

void DecSmartDoorUIValue()
{
	gubDoorUIValue -= gubDoorUIValue > 0 ? 1 : -7;
}

void IncSmartWindowUIValue()
{
	gubWindowUIValue += gubWindowUIValue < 2 ? 1 : -2;
}

void DecSmartWindowUIValue()
{
	gubWindowUIValue -= gubWindowUIValue > 0 ? 1 : -2;
}

void IncSmartBrokenWallUIValue()
{
	gubBrokenWallUIValue += gubBrokenWallUIValue < 4 ? 1 : -4;
}

void DecSmartBrokenWallUIValue()
{
	gubBrokenWallUIValue -= gubBrokenWallUIValue > 0 ? 1 : -4;
}

BOOLEAN CalcWallInfoUsingSmartMethod( UINT32 iMapIndex, UINT16 *pusWallType, UINT16 *pusIndex )
{
	return false;
}

BOOLEAN CalcDoorInfoUsingSmartMethod( UINT32 iMapIndex, UINT16 *pusDoorType, UINT16 *pusIndex )
{
	LEVELNODE *pWall = NULL;
	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartDoorIndex( usWallOrientation ) - 1;
		*pusDoorType = CalcSmartDoorType();
		return true;
	}
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartDoorIndex( usWallOrientation ) - 1;
		*pusDoorType = CalcSmartDoorType();
		return true;
	}
	return false;
}

BOOLEAN CalcWindowInfoUsingSmartMethod( UINT32 iMapIndex, UINT16 *pusWallType, UINT16 *pusIndex )
{
	LEVELNODE *pWall = NULL;

	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		*pusWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartWindowIndex( usWallOrientation ) - 1;
		return true;
	}
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		*pusWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartWindowIndex( usWallOrientation ) - 1;
		return true;
	}
	return false;
}

BOOLEAN CalcBrokenWallInfoUsingSmartMethod( UINT32 iMapIndex, UINT16 *pusWallType, UINT16 *pusIndex )
{
	LEVELNODE *pWall = NULL;

	if( gubBrokenWallUIValue == 2 ) //the hole in the wall
	{
		*pusWallType = 0xffff;
		*pusIndex = 0xffff;	//but it won't draw it.
		return true;
	}

	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		*pusWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartBrokenWallIndex( usWallOrientation ) - 1;
		return true;
	}
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		*pusWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		*pusIndex = CalcSmartBrokenWallIndex( usWallOrientation ) - 1;
		return true;
	}
	return false;
}


//This is a very difficult function to document properly.  The reason being is that it is sooo
//subliminal by nature.  I have thought up of priorities and choose the best piece to draw based
//on the surrounding conditions.  Here are the priorities which are referenced below via comments:
//A)  If there is currently a bottom piece and a right piece, immediately exit.
//B)  We are currently over a bottom piece.  Now, we don't automatically want to draw a right piece here
//		for multiple reasons.  First, the UI will be too quick and place bottom and right pieces for every
//		place the user clicks, which isn't what we want.  Therefore, we look to see if there is a right
//    piece in the y-1 gridno.  It would then make sense to place a right piece down here.  Regardless,
//		if we encounter a bottom piece here, we will exit.
//C)  This is the counterpart to B, but we are looking at a current right piece, and are determining if
//		we should place a bottom piece based on another bottom piece existing in the x-1 gridno.
//D)  Now, we analyse the neighboring tiles and determine the orientations that would add weight to the
//    current tile either towards drawing a horizontal piece or a vertical piece.
//E)  Now that we have the information, we give the highest priority to any weights that match the current
//		wall piece type selected by the user.  Based on that, we will only consider the best match of the
//		type and use it.  If there are no matches on type, we continue.
//F)  We failed to find weights matching the current wall type, but before we give up using the user's wall
//		type, there are two more cases.  When there is a bottom wall in the y+1 position or a right wall in
//		the x+1 position.  If there are matching walls, there, then we draw two pieces to connect the current
//		gridno with the respective position.
void PasteSmartWall( UINT32 iMapIndex )
{
	UINT16 usWallType;

	//These are the counters for the walls of each type
	UINT16 usNumV[4]={0,0,0,0}; //vertical wall weights
	UINT16 usNumH[4]={0,0,0,0}; //horizontal wall weights

	//*A* See above documentation
	if( GetVerticalWall( iMapIndex ) && GetHorizontalWall( iMapIndex ) )
		return;
	//*B* See above documentation
	usWallType = GetHorizontalWallType( iMapIndex );
	if( usWallType )
	{
		if( usWallType == gubWallUIValue )
		{
			usWallType = GetVerticalWallType( iMapIndex - WORLD_COLS );
			if( usWallType == gubWallUIValue )
			{
				if( FloorAtGridNo( iMapIndex + 1 ) )
					BuildWallPiece( iMapIndex, EXTERIOR_RIGHT, gubWallUIValue );
				else
					BuildWallPiece( iMapIndex, INTERIOR_RIGHT, gubWallUIValue );
				return;
			}
			usWallType = GetHorizontalWallType( iMapIndex - WORLD_COLS );
			if( usWallType == gubWallUIValue )
			{
				if( FloorAtGridNo( iMapIndex + 1 ) )
				{
					BuildWallPiece( iMapIndex, EXTERIOR_RIGHT, gubWallUIValue );
					if( !GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) )
						ChangeVerticalWall( iMapIndex, INTERIOR_EXTENDED );
				}
				else
				{
					BuildWallPiece( iMapIndex, INTERIOR_RIGHT, gubWallUIValue );
					if( !GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) )
						ChangeVerticalWall( iMapIndex, EXTERIOR_EXTENDED );
				}
			}
		}
		return;
	}
	//*C* See above documentation
	usWallType = GetVerticalWallType( iMapIndex );
	if( usWallType )
	{
		if( usWallType == gubWallUIValue )
		{
			usWallType = GetHorizontalWallType( iMapIndex - 1 );
			if( usWallType == gubWallUIValue )
			{
				if( FloorAtGridNo( iMapIndex + WORLD_COLS ) )
					BuildWallPiece( iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue );
				else
					BuildWallPiece( iMapIndex, INTERIOR_BOTTOM, gubWallUIValue );
			}
		}
		return;
	}
	//*D* See above documentation
	//Evaluate left adjacent tile
	if( (usWallType = GetVerticalWallType( iMapIndex - 1 )) )
		usNumH[ usWallType - FIRSTWALL ]++;
	if( (usWallType = GetHorizontalWallType( iMapIndex - 1 )) )
		usNumH[ usWallType - FIRSTWALL ]++;
	//Evaluate right adjacent tile
	if( (usWallType = GetHorizontalWallType( iMapIndex + 1 )) )
		usNumH[ usWallType - FIRSTWALL ]++;
	//Evaluate upper adjacent tile
	if( (usWallType = GetVerticalWallType( iMapIndex - WORLD_COLS )) )
		usNumV[ usWallType - FIRSTWALL ]++;
	if( (usWallType = GetHorizontalWallType( iMapIndex - WORLD_COLS )) )
		usNumV[ usWallType - FIRSTWALL ]++;
	//Evaluate lower adjacent tile
	if( (usWallType = GetVerticalWallType( iMapIndex + WORLD_COLS )) )
		usNumV[ usWallType - FIRSTWALL ]++;
	//*E* See above documentation
	if( usNumV[gubWallUIValue - FIRSTWALL] | usNumH[gubWallUIValue - FIRSTWALL] )
	{
		if( usNumV[gubWallUIValue - FIRSTWALL] >= usNumH[gubWallUIValue - FIRSTWALL] )
		{
			if( FloorAtGridNo( iMapIndex + 1 ) )
			{ //inside
				BuildWallPiece( iMapIndex, EXTERIOR_RIGHT, gubWallUIValue );
				//Change to extended piece if it is a new top right corner to cover the end part.
				if( GetHorizontalWall( iMapIndex - WORLD_COLS ) && !GetHorizontalWall( iMapIndex - WORLD_COLS + 1 )
						&& !GetVerticalWall( iMapIndex - WORLD_COLS ) )
					ChangeVerticalWall( iMapIndex, INTERIOR_EXTENDED );
				else if( GetHorizontalWall( iMapIndex - WORLD_COLS ) && !GetHorizontalWall( iMapIndex - WORLD_COLS - 1 )
					&& !GetVerticalWall( iMapIndex - WORLD_COLS - 1 ) )
				{
					ChangeVerticalWall( iMapIndex, INTERIOR_EXTENDED );
					EraseHorizontalWall( iMapIndex - WORLD_COLS );
				}
			}
			else
			{	//outside
				BuildWallPiece( iMapIndex, INTERIOR_RIGHT, gubWallUIValue );
				if( GetHorizontalWall( iMapIndex - WORLD_COLS ) && !GetHorizontalWall( iMapIndex - WORLD_COLS + 1 )
						&& !GetVerticalWall( iMapIndex - WORLD_COLS ) )
					ChangeVerticalWall( iMapIndex, EXTERIOR_EXTENDED );
				else if( GetHorizontalWall( iMapIndex - WORLD_COLS ) && !GetHorizontalWall( iMapIndex - WORLD_COLS - 1 )
					&& !GetVerticalWall( iMapIndex - WORLD_COLS - 1 ) )
				{
					ChangeVerticalWall( iMapIndex, EXTERIOR_EXTENDED );
					EraseHorizontalWall( iMapIndex - WORLD_COLS );
				}
			}
		}
		else
		{
			if( GetVerticalWall( iMapIndex - 1 ) && !GetVerticalWall( iMapIndex - WORLD_COLS - 1 )
				&& !GetHorizontalWall( iMapIndex - WORLD_COLS - 1 ) )
			{
				EraseVerticalWall( iMapIndex - 1);
			}
			if( FloorAtGridNo( iMapIndex + WORLD_COLS ) )
			{ //inside
				BuildWallPiece( iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue );
				if( GetVerticalWall( iMapIndex + WORLD_COLS ) )
					ChangeVerticalWall( iMapIndex + WORLD_COLS, INTERIOR_EXTENDED );
				if( GetVerticalWall( iMapIndex + WORLD_COLS - 1 ) && !GetVerticalWall( iMapIndex - 1 ) )
					ChangeVerticalWall( iMapIndex + WORLD_COLS - 1,  INTERIOR_EXTENDED );
				else if( GetVerticalWall( iMapIndex - 1 ) && !GetVerticalWall( iMapIndex + WORLD_COLS - 1 )
					&& FloorAtGridNo( iMapIndex ) )
				{
					ChangeVerticalWall( iMapIndex - 1, INTERIOR_BOTTOMEND );
				}
			}
			else
			{ //outside
				BuildWallPiece( iMapIndex, INTERIOR_BOTTOM, gubWallUIValue );
				if( GetVerticalWall( iMapIndex + WORLD_COLS ) )
					ChangeVerticalWall( iMapIndex + WORLD_COLS, EXTERIOR_EXTENDED );
				if( GetVerticalWall( iMapIndex + WORLD_COLS - 1 ) && !GetVerticalWall( iMapIndex - 1 ) )
					ChangeVerticalWall( iMapIndex + WORLD_COLS - 1,  EXTERIOR_EXTENDED );
				else if( GetVerticalWall( iMapIndex - 1 ) && !GetVerticalWall( iMapIndex + WORLD_COLS - 1 )
					&& FloorAtGridNo( iMapIndex ) )
				{
					ChangeVerticalWall( iMapIndex - 1, EXTERIOR_BOTTOMEND );
				}
			}
		}
		return;
	}
	//*F* See above documentation
	usWallType = GetHorizontalWallType( iMapIndex + WORLD_COLS );
	if( usWallType == gubWallUIValue )
	{
		if( !GetHorizontalWall( iMapIndex + WORLD_COLS - 1 ) )
			EraseHorizontalWall( iMapIndex + WORLD_COLS );
		if( FloorAtGridNo( iMapIndex + 1 ) )
		{ //inside
			BuildWallPiece( iMapIndex + WORLD_COLS, EXTERIOR_RIGHT, gubWallUIValue );
			BuildWallPiece( iMapIndex, EXTERIOR_RIGHT, gubWallUIValue );
			if( !GetVerticalWall( iMapIndex + WORLD_COLS * 2 ) && FloorAtGridNo( iMapIndex + WORLD_COLS * 2 + 1 ) )
				ChangeVerticalWall( iMapIndex + WORLD_COLS, INTERIOR_BOTTOMEND );
			else //override the damn other smoothing.
				ChangeVerticalWall( iMapIndex + WORLD_COLS, INTERIOR_R );
		}
		else
		{ //outside
			BuildWallPiece( iMapIndex + WORLD_COLS, INTERIOR_RIGHT, gubWallUIValue );
			BuildWallPiece( iMapIndex, INTERIOR_RIGHT, gubWallUIValue );
			if( !GetVerticalWall( iMapIndex + WORLD_COLS * 2 ) && !FloorAtGridNo( iMapIndex + WORLD_COLS * 2 + 1 ) )
				ChangeVerticalWall( iMapIndex + WORLD_COLS, EXTERIOR_BOTTOMEND );
			else //override the damn other smoothing.
				ChangeVerticalWall( iMapIndex + WORLD_COLS, EXTERIOR_R );
		}
		return;
	}
	usWallType = GetVerticalWallType( iMapIndex + 1 );
	if( usWallType == gubWallUIValue )
	{
		if(FloorAtGridNo( iMapIndex + WORLD_COLS ) )
		{ //inside
			BuildWallPiece( iMapIndex + 1, EXTERIOR_BOTTOM, gubWallUIValue );
			BuildWallPiece( iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue );
			if( !GetVerticalWall( iMapIndex - WORLD_COLS + 1 ) )
			{
				EraseVerticalWall( iMapIndex + 1 );
				ChangeVerticalWall( iMapIndex + WORLD_COLS + 1, INTERIOR_EXTENDED );
			}
			if( !GetVerticalWall( iMapIndex + WORLD_COLS + 1) )
			{
				if( !GetHorizontalWall( iMapIndex - WORLD_COLS + 1) && !GetVerticalWall( iMapIndex - WORLD_COLS + 1 )
						&& GetHorizontalWall( iMapIndex - WORLD_COLS + 2 ) )
					ChangeVerticalWall( iMapIndex + 1, INTERIOR_EXTENDED );
				else
					ChangeVerticalWall( iMapIndex + 1, INTERIOR_BOTTOMEND );
			}
		}
		else
		{ //outside
			BuildWallPiece( iMapIndex + 1, INTERIOR_BOTTOM, gubWallUIValue );
			BuildWallPiece( iMapIndex, INTERIOR_BOTTOM, gubWallUIValue );
			if( !GetVerticalWall( iMapIndex - WORLD_COLS + 1 ) )
			{
				EraseVerticalWall( iMapIndex + 1 );
				ChangeVerticalWall( iMapIndex + WORLD_COLS + 1, EXTERIOR_EXTENDED );
			}
			if( !GetVerticalWall( iMapIndex + WORLD_COLS + 1) )
			{
				if( !GetHorizontalWall( iMapIndex - WORLD_COLS + 1) && !GetVerticalWall( iMapIndex - WORLD_COLS + 1 )
						&& GetHorizontalWall( iMapIndex - WORLD_COLS + 2 ) )
					ChangeVerticalWall( iMapIndex + 1, EXTERIOR_EXTENDED );
				else
					ChangeVerticalWall( iMapIndex + 1, EXTERIOR_BOTTOMEND );
			}
		}
		return;
	}
	//Check for the highest weight value.
}

void PasteSmartDoor( UINT32 iMapIndex )
{
	LEVELNODE *pWall = NULL;
	UINT16 usDoorType;
	UINT16 usIndex;

	if( (pWall = GetVerticalWall( iMapIndex )) )
	{
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartDoorIndex( usWallOrientation );
		usDoorType = CalcSmartDoorType();
		AddToUndoList( iMapIndex );
		UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usDoorType, usIndex);
		ReplaceStructIndex( iMapIndex, pWall->usIndex, usTileIndex );
	}
	if( (pWall = GetHorizontalWall( iMapIndex )) )
	{
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartDoorIndex( usWallOrientation );
		usDoorType = CalcSmartDoorType();
		AddToUndoList( iMapIndex );
		UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usDoorType, usIndex);
		ReplaceStructIndex( iMapIndex, pWall->usIndex, usTileIndex );
	}
}

void PasteSmartWindow( UINT32 iMapIndex )
{
	LEVELNODE *pWall = NULL;
	UINT16 usIndex;

	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		UINT16 const usWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartWindowIndex( usWallOrientation );
		//Calculate the new graphic for the window type selected.

		AddToUndoList( iMapIndex );
		UINT16 usNewWallIndex = GetTileIndexFromTypeSubIndex(usWallType, usIndex);
		ReplaceStructIndex( iMapIndex, pWall->usIndex, usNewWallIndex );
	}
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		UINT16 const usWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartWindowIndex( usWallOrientation );
		//Calculate the new graphic for the window type selected.
		AddToUndoList( iMapIndex );
		UINT16 usNewWallIndex = GetTileIndexFromTypeSubIndex(usWallType, usIndex);
		ReplaceStructIndex( iMapIndex, pWall->usIndex, usNewWallIndex );
	}
}

void PasteSmartBrokenWall( UINT32 iMapIndex )
{
	LEVELNODE *pWall;
	UINT16 usIndex;

	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		UINT16 const usWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartBrokenWallIndex( usWallOrientation );
		if( usIndex == 0xffff )
		{
			AddToUndoList( iMapIndex );
			RemoveStruct( iMapIndex, pWall->usIndex );
		}
		else
		{
			AddToUndoList( iMapIndex );
			UINT16 usNewWallIndex = GetTileIndexFromTypeSubIndex(usWallType, usIndex);
			ReplaceStructIndex( iMapIndex, pWall->usIndex, usNewWallIndex );
		}
	}
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		UINT16 const usWallType = GetWallType(pWall, iMapIndex);
		UINT16 usWallOrientation = GetWallOrientation(pWall->usIndex);
		usIndex = CalcSmartBrokenWallIndex( usWallOrientation );
		if( usIndex == 0xffff )
		{
			AddToUndoList( iMapIndex );
			RemoveStruct( iMapIndex, pWall->usIndex );
		}
		else
		{
			AddToUndoList( iMapIndex );
			UINT16 usNewWallIndex = GetTileIndexFromTypeSubIndex(usWallType, usIndex);
			ReplaceStructIndex( iMapIndex, pWall->usIndex, usNewWallIndex );
		}
		//Calculate the new graphic for the window type selected.
	}
}
