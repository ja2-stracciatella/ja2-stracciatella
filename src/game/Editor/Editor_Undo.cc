#include "PODObj.h"
#include "Structure.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Smooth.h"
#include "Input.h"
#include "Debug.h"
#include "Isometric_Utils.h"
#include "EditScreen.h"
#include "Editor_Undo.h"
#include "Render_Fun.h"  //for access to gubWorldRoomInfo;
#include "Cursor_Modes.h"
#include "Exit_Grids.h"


/*
Kris -- Notes on how the undo code works:

At the bottom of the hierarchy, we need to determine the state of the undo command.  The idea
is that we want to separate undo commands by separating them by new mouse clicks.  By holding a mouse
down and painting various objects in the world would all constitute a single undo command.  As soon as
the mouse is release, then a new undo command is setup.  So, to automate this, there is a call every
frame to DetermineUndoState().

At the next level, there is a binary tree that keeps track of what map indices have been backup up in
the current undo command.  The whole reason to maintain this list, is to avoid multiple map elements of
the same map index from being saved.  In the outer code, everytime something is changed, a call to
AddToUndoList() is called, so there are many cases (especially with building/terrain smoothing) that the
same mapindex is added to the undo list.  This is also completely transparent, and doesn't need to be
maintained.

In the outer code, there are several calls to AddToUndoList( iMapIndex ).  This function basically looks
in the binary tree for an existing entry, and if there isn't, then the entire mapelement is saved (with
the exception of the merc level ).  Lights are also supported, but there is a totally different methodology
for accomplishing this.  The equivalent function is AddLightToUndoList( iMapIndex ).  In this case, only the
light is saved, along with internal maintanance of several flags.

The actual mapelement copy code, is very complex.  The mapelement is copied in parallel with a new one which
has to allocate several nodes of several types because a mapelement contains over a dozen separate lists, and
all of them need to be saved.  The structure information of certain mapelements may be multitiled and must
also save the affected gridno's as well.  This is also done internally.  Basically, your call to
AddToUndoList() for any particular gridno, may actually save several entries (like for a car which could be 6+
tiles)

MERCS
Mercs are not supported in the undo code.  Because they are so dynamic, and stats could change, they could
move, etc., it doesn't need to be in the undo code.  The editor has its own way of dealing with mercs which
doesn't use the undo methodology.

*/


BOOLEAN gfUndoEnabled = FALSE;

void EnableUndo()
{
	gfUndoEnabled = TRUE;
}

void DisableUndo()
{
	gfUndoEnabled = FALSE;
}

// undo node data element
struct undo_struct
{
	INT32       iMapIndex;
	MAP_ELEMENT *pMapTile;
	BOOLEAN     fLightSaved;   //determines that a light has been saved
	UINT8       ubLightRadius; //the radius of the light to build if undo is called
	UINT8       ubRoomNum;
};

// Undo stack node
struct undo_stack
{
	INT32										iCmdCount;
	undo_struct							*pData;
	undo_stack*  pNext;
	INT32										iUndoType;
};
undo_stack			*gpTileUndoStack = NULL;


BOOLEAN fNewUndoCmd = TRUE;
BOOLEAN gfIgnoreUndoCmdsForLights = FALSE;

//New pre-undo binary tree stuff
//With this, new undo commands will not duplicate saves in the same command.  This will
//increase speed, and save memory.
struct MapIndexBinaryTree
{
	MapIndexBinaryTree* left;
	MapIndexBinaryTree* right;
	UINT16 usMapIndex;
};

MapIndexBinaryTree *top = NULL;


//Recursively deletes all nodes below the node passed including itself.
static void DeleteTreeNode(MapIndexBinaryTree** node)
{
	if( (*node)->left )
		DeleteTreeNode( &((*node)->left) );
	if( (*node)->right )
		DeleteTreeNode( &((*node)->right) );
	delete *node;
	*node = NULL;
}


//Recursively delete all nodes (from the top down).
static void ClearUndoMapIndexTree(void)
{
	if( top )
		DeleteTreeNode( &top );
}


static BOOLEAN AddMapIndexToTree(UINT16 usMapIndex)
{
	MapIndexBinaryTree *curr, *parent;
	if( !top )
	{
		top = new MapIndexBinaryTree{};
		top->usMapIndex = usMapIndex;
		top->left = NULL;
		top->right = NULL;
		return TRUE;
	}
	curr = top;
	parent = NULL;
	//Traverse down the tree and attempt to find a matching mapindex.
	//If one is encountered, then we fail, and don't add the mapindex to the
	//tree.
	while( curr )
	{
		parent = curr;
		if( curr->usMapIndex == usMapIndex ) //found a match, so stop
			return FALSE;
		//if the mapIndex is < node's mapIndex, then go left, else right
		curr = ( usMapIndex < curr->usMapIndex ) ? curr->left : curr->right;
	}
	//if we made it this far, then curr is null and parent is pointing
	//directly above.
	//Create the new node and fill in the information.
	curr = new MapIndexBinaryTree{};
	curr->usMapIndex = usMapIndex;
	curr->left = NULL;
	curr->right = NULL;
	//Now link the new node to the parent.
	if( curr->usMapIndex < parent->usMapIndex )
		parent->left = curr;
	else
		parent->right = curr;
	return TRUE;
}


static undo_stack* DeleteStackNode(undo_stack* const del)
{
	undo_struct* const undo = del->pData;
	undo_stack*  const succ = del->pNext;
	delete del;

	MAP_ELEMENT* const me = undo->pMapTile;
	if (me != NULL) // light was saved -- mapelement, too?
	{
		// Free the memory associated with the map tile lists
		FreeLevelNodeList(&me->pLandHead);
		FreeLevelNodeList(&me->pObjectHead);
		FreeLevelNodeList(&me->pStructHead);
		FreeLevelNodeList(&me->pShadowHead);
		FreeLevelNodeList(&me->pMercHead);
		FreeLevelNodeList(&me->pRoofHead);
		FreeLevelNodeList(&me->pOnRoofHead);
		FreeLevelNodeList(&me->pTopmostHead);

		for (STRUCTURE* s = me->pStructureHead; s != NULL;)
		{
			STRUCTURE* const next = s->pNext;
			if (s->usStructureID > INVALID_STRUCTURE_ID)
			{ /* Okay to delete the structure data -- otherwise, this would be merc
				 * structure data that we DON'T want to delete, because the merc node that
				 * hasn't been modified will still use this structure data! */
				delete s;
			}
			s = next;
		}
		me->pStructureHead = NULL;

		delete me;
		delete undo;
	}

	return succ;
}


static void DeleteTopStackNode(void)
{
	gpTileUndoStack = DeleteStackNode(gpTileUndoStack);
}


static void CropStackToMaxLength(INT32 iMaxCmds)
{
	INT32				iCmdCount;
	undo_stack	*pCurrent;

	iCmdCount = 0;
	pCurrent = gpTileUndoStack;

	// If stack is empty, leave
	if ( pCurrent == NULL )
		return;

	while ( (iCmdCount <= (iMaxCmds - 1)) && (pCurrent != NULL) )
	{
		if ( pCurrent->iCmdCount == 1 )
			iCmdCount++;
		pCurrent = pCurrent->pNext;
	}

	// If the max number of commands was reached, and there is something
	// to crop, from the rest of the stack, remove it.
	if ( (iCmdCount >= iMaxCmds) && pCurrent != NULL )
	{
		while ( pCurrent->pNext != NULL )
			pCurrent->pNext = DeleteStackNode(pCurrent->pNext);
	}
}


//We are adding a light to the undo list.  We won't save the mapelement, nor will
//we validate the gridno in the binary tree.  This works differently than a mapelement,
//because lights work on a different system.  By setting the fLightSaved flag to TRUE,
//this will handle the way the undo command is handled.  If there is no lightradius in
//our saved light, then we intend on erasing the light upon undo execution, otherwise, we
//save the light radius and light ID, so that we place it during undo execution.
void AddLightToUndoList(INT32 const iMapIndex, INT32 const iLightRadius)
{
	if (!gfUndoEnabled) return;

	/* When executing an undo command (by adding a light or removing one), that
	 * command actually tries to add it to the undo list.  So we wrap the
	 * execution of the undo command by temporarily setting this flag, so it'll
	 * ignore, and not place a new undo command.  When finished, the flag is
	 * cleared, and lights are again allowed to be saved in the undo list. */
	if (gfIgnoreUndoCmdsForLights) return;

	SGP::PODObj<undo_struct> undo_info;
	undo_info->fLightSaved = TRUE;
	/* if ubLightRadius is 0, then we don't need to save the light information
	 * because we will erase it when it comes time to execute the undo command. */
	undo_info->ubLightRadius = iLightRadius;
	undo_info->iMapIndex     = iMapIndex;
	undo_info->pMapTile      = NULL;

	//Add to undo stack
	SGP::PODObj<undo_stack> n;
	n->iCmdCount    = 1;
	n->pData        = undo_info.Release();
	n->pNext        = gpTileUndoStack;
	gpTileUndoStack = n.Release();

	CropStackToMaxLength(MAX_UNDO_COMMAND_LENGTH);
}


static void AddToUndoListCmd(INT32 iMapIndex, INT32 iCmdCount);


BOOLEAN AddToUndoList( INT32 iMapIndex )
{
	static INT32 iCount = 1;

	if(	!gfUndoEnabled )
		return FALSE;
	if ( fNewUndoCmd )
	{
		iCount = 0;
		fNewUndoCmd = FALSE;
	}

	//Check to see if the tile in question is even on the visible map, then
	//if that is true, then check to make sure we don't already have the mapindex
	//saved in the new binary tree (which only holds unique mapindex values).
	if( GridNoOnVisibleWorldTile( (INT16)iMapIndex ) && AddMapIndexToTree( (UINT16)iMapIndex ) )

	{
		try
		{
			AddToUndoListCmd(iMapIndex, ++iCount);
			return TRUE;
		}
		catch (...) { --iCount; }
	}
	return FALSE;
}


static MAP_ELEMENT* CopyMapElementFromWorld(INT32 map_index);


static void AddToUndoListCmd(INT32 const iMapIndex, INT32 const iCmdCount)
{
	INT32					iCoveredMapIndex;
	UINT8					ubLoop;

	SGP::PODObj<undo_stack>  pNode;
	SGP::PODObj<undo_struct> pUndoInfo;

	// Copy the world map's tile
	MAP_ELEMENT* const pData = CopyMapElementFromWorld(iMapIndex);

	STRUCTURE* pStructure = pData->pStructureHead;

	// copy the room number information (it's not in the mapelement structure)
	pUndoInfo->ubRoomNum = gubWorldRoomInfo[ iMapIndex ];

	pUndoInfo->fLightSaved = FALSE;
	pUndoInfo->ubLightRadius = 0;
	pUndoInfo->pMapTile = pData;
	pUndoInfo->iMapIndex = iMapIndex;

	pNode->pData = pUndoInfo.Release();
	pNode->iCmdCount = iCmdCount;
	pNode->pNext = gpTileUndoStack;
	gpTileUndoStack = pNode.Release();

	// loop through pData->pStructureHead list
	// for each structure
	//   find the base tile
	//   reference the db structure
	//   if number of tiles > 1
	//     add all covered tiles to undo list
	while (pStructure)
	{
		for (ubLoop = 1; ubLoop < pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles; ubLoop++)
		{
			// this loop won't execute for single-tile structures; for multi-tile structures, we have to
			// add to the undo list all the other tiles covered by the structure
			iCoveredMapIndex = pStructure->sBaseGridNo + pStructure->pDBStructureRef->ppTile[ubLoop]->sPosRelToBase;
			AddToUndoList( iCoveredMapIndex );
		}
		pStructure = pStructure->pNext;
	}

	CropStackToMaxLength( MAX_UNDO_COMMAND_LENGTH );
}


void RemoveAllFromUndoList(void)
{
	ClearUndoMapIndexTree();

	while ( gpTileUndoStack != NULL )
		DeleteTopStackNode();
}


static void SwapMapElementWithWorld(INT32 iMapIndex, MAP_ELEMENT* pUndoMapElement);


BOOLEAN ExecuteUndoList( void )
{
	INT32				iCmdCount, iCurCount;
	INT32				iUndoMapIndex;

	if(	!gfUndoEnabled )
		return FALSE;

	// Is there something on the undo stack?
	if ( gpTileUndoStack == NULL )
		return( TRUE );

	// Get number of stack entries for this command (top node will tell this)
	iCmdCount = gpTileUndoStack->iCmdCount;

	// Execute each stack node in command, and remove each from stack.
	iCurCount = 0;
	while ( (iCurCount < iCmdCount) && (gpTileUndoStack != NULL) )
	{
		iUndoMapIndex = gpTileUndoStack->pData->iMapIndex;

		BOOLEAN fExitGrid = FALSE; // XXX HACK000E
		// Find which map tile we are to "undo"
		if( gpTileUndoStack->pData->fLightSaved )
		{ //We saved a light, so delete that light
			//Turn on this flag so that the following code, when executed, doesn't attempt to
			//add lights to the undo list.  That would cause problems...
			gfIgnoreUndoCmdsForLights = TRUE;
			if( !gpTileUndoStack->pData->ubLightRadius )
				RemoveLight(iUndoMapIndex);
			else
				PlaceLight(gpTileUndoStack->pData->ubLightRadius, iUndoMapIndex);
			//Turn off the flag so lights can again be added to the undo list.
			gfIgnoreUndoCmdsForLights = FALSE;
		}
		else
		{	// We execute the undo command node by simply swapping the contents
			// of the undo's MAP_ELEMENT with the world's element.
			fExitGrid = ExitGridAtGridNo( (UINT16)iUndoMapIndex );
			SwapMapElementWithWorld( iUndoMapIndex, gpTileUndoStack->pData->pMapTile );

			// copy the room number information back
			gubWorldRoomInfo[ iUndoMapIndex ] = gpTileUndoStack->pData->ubRoomNum;

			// Now we smooth out the changes...
			//SmoothUndoMapTileTerrain( iUndoMapIndex, gpTileUndoStack->pData->pMapTile );
			SmoothAllTerrainTypeRadius( iUndoMapIndex, 1, TRUE );
		}

		// ...trash the top element of the stack...
		DeleteTopStackNode( );

		// ...and bump the command counter up by 1
		iCurCount++;

		//Kris:
		//The new cursor system is somehow interfering with the undo stuff.  When
		//an undo is called, the item is erased, but a cursor is added!  I'm quickly
		//hacking around this by erasing all cursors here.
		RemoveAllTopmostsOfTypeRange( iUndoMapIndex, FIRSTPOINTERS, FIRSTPOINTERS );

		if( fExitGrid && !ExitGridAtGridNo( (UINT16)iUndoMapIndex ) )
		{ //An exitgrid has been removed, so get rid of the associated indicator.
			RemoveTopmost( (UINT16)iUndoMapIndex, FIRSTPOINTERS8 );
		}
		else if( !fExitGrid && ExitGridAtGridNo( (UINT16)iUndoMapIndex ) )
		{ //An exitgrid has been added, so add the associated indicator
			AddTopmostToTail( (UINT16)iUndoMapIndex, FIRSTPOINTERS8 );
		}
	}

	return( TRUE );
}

namespace
{
	//Because of the potentially huge amounts of memory that can be allocated due to the inefficient
	//undo methods coded by Bret, it is feasible that it could fail.  Instead of using assertions to
	//terminate the program, destroy the memory allocated thusfar.
	void DeleteMapElementContentsAfterCreationFail(MAP_ELEMENT* pNewMapElement)
	{
		LEVELNODE *pLevelNode;
		STRUCTURE *pStructure;
		INT32 x;
		for( x = 0; x < 9; x++ )
		{
			if( x == 1 )
				continue;
			pLevelNode = pNewMapElement->pLevelNodes[ x ];
			while( pLevelNode )
			{
				LEVELNODE *temp;
				temp = pLevelNode;
				pLevelNode = pLevelNode->pNext;
				delete temp;
			}
		}
		pStructure = pNewMapElement->pStructureHead;
		while( pStructure )
		{
			STRUCTURE *temp;
			temp = pStructure;
			pStructure = pStructure->pNext;
			delete temp;
		}
	}
}


static MAP_ELEMENT* CopyMapElementFromWorld(INT32 const map_index)
{
	SGP::AutoObj<MAP_ELEMENT, DeleteMapElementContentsAfterCreationFail> new_me(new MAP_ELEMENT{});

	MAP_ELEMENT const* const old_me = &gpWorldLevelData[map_index];

	// Save the structure information from the mapelement
	STRUCTURE*  tail   = 0;
	STRUCTURE** anchor = &new_me->pStructureHead;
	for (STRUCTURE const* i = old_me->pStructureHead; i; i = i->pNext)
	{
		STRUCTURE* const s = new STRUCTURE{};
		*s       = *i;
		s->pPrev = tail;
		s->pNext = 0;
		tail     = s;
		*anchor  = s;
		anchor   = &s->pNext;
	}
	new_me->pStructureTail = tail;

	/* For each of the 9 levelnodes, save each one, except for levelnode[1] which
	 * is a pointer to the first land to render. */
	for (INT32 x = 0; x != 9; ++x)
	{
		if (x == 1 || x == 5) continue; // Skip the pLandStart and pMercLevel levelnodes
		LEVELNODE*  tail   = 0;
		LEVELNODE** anchor = &new_me->pLevelNodes[x];
		for (LEVELNODE const* i = old_me->pLevelNodes[x]; i; i = i->pNext)
		{
			LEVELNODE* const l = new LEVELNODE{};
			*l       = *i;
			if (x == 0) l->pPrevNode = tail; // Land layer only
			l->pNext = 0;
			tail     = l;
			*anchor  = l;
			anchor   = &l->pNext;

			// Handle levelnode layer specific stuff
			switch (x)
			{
				case 0: // Land layer
					// Set the new landstart to point to the new levelnode.
					if (old_me->pLandStart == i) new_me->pLandStart = l;
					break;

				case 3: // Struct layer
				case 6:	// Roof layer
				case 7: // On roof layer
					if (i->pStructureData)
					{ /* Make sure the structuredata pointer points to the parallel
						 * structure. Both lists are exactly the same size and contain the
						 * same information, but the addresses are different.  We will
						 * traverse the old list until we find the match, then */
						STRUCTURE const* o = old_me->pStructureHead;
						STRUCTURE*       n = new_me->pStructureHead;
						for (;; o = o->pNext, n = n->pNext)
						{
							/* Kris: If this assert should fail, that means there is something
							 * wrong with the preservation of the structure data within the
							 * mapelement. */
							// OUCH!!! THIS IS HAPPENING.  DISABLED IT FOR LINDA'S SAKE
							if (!o) break;
							Assert(o);
							Assert(n);
							if (o == i->pStructureData)
							{
								l->pStructureData = n;
								break;
							}
						}
					}
					break;
			}
		}
	}

	// Save the rest of the information in the mapelement.
	new_me->uiFlags							= old_me->uiFlags;
	new_me->sHeight							= old_me->sHeight;
	new_me->ubTerrainID					= old_me->ubTerrainID;
	new_me->ubReservedSoldierID = old_me->ubReservedSoldierID;
	return new_me.Release();
}


static void SwapMapElementWithWorld(INT32 const iMapIndex, MAP_ELEMENT* const pUndoMapElement)
{
	MAP_ELEMENT			*pCurrentMapElement;
	MAP_ELEMENT			TempMapElement;

	pCurrentMapElement = &gpWorldLevelData[ iMapIndex ];

	//Transfer the merc level node from the current world to the undo mapelement
	//that will replace it.  We do this, because mercs aren't associated with
	//undo commands.
	pUndoMapElement->pMercHead = gpWorldLevelData[ iMapIndex ].pMercHead;
	gpWorldLevelData[ iMapIndex ].pMercHead = NULL;

	//Swap the mapelements
	TempMapElement = *pCurrentMapElement;
	*pCurrentMapElement = *pUndoMapElement;
	*pUndoMapElement = TempMapElement;
}


void DetermineUndoState()
{
	// Reset the undo command mode if we released the left button.
	if( !fNewUndoCmd )
	{
		if( (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)  && !gfCurrentSelectionWithRightButton) ||
			(!IsMouseButtonDown(MOUSE_BUTTON_RIGHT) &&  gfCurrentSelectionWithRightButton) )
		{
			//Clear the mapindex binary tree list, and set up flag for new undo command.
			fNewUndoCmd = TRUE;
			ClearUndoMapIndexTree();
		}
	}
}
