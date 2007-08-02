#include "Local.h"
#include "Types.h"
#include "Render_Dirty.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Slider.h"
#include "SysUtil.h"
#include "Line.h"
#include "Debug.h"
#include "Video.h"
#include "MemMan.h"
#include "Button_System.h"
#include "VSurface.h"


#define		DEFUALT_SLIDER_SIZE					7


#define		STEEL_SLIDER_WIDTH					42
#define		STEEL_SLIDER_HEIGHT					25


typedef struct TAG_SLIDER
{
	UINT32				uiSliderID;

	UINT8					ubStyle;
	UINT16				usPosX;
	UINT16				usPosY;
	UINT16				usWidth;
	UINT16				usHeight;
	UINT16				usNumberOfIncrements;
	SLIDER_CHANGE_CALLBACK SliderChangeCallback;

	UINT16				usCurrentIncrement;

	UINT16				usBackGroundColor;

	MOUSE_REGION  ScrollAreaMouseRegion;

	UINT32				uiSliderBoxImage;
	UINT16				usCurrentSliderBoxPosition;

	SGPRect				LastRect;

	UINT32				uiFlags;

	UINT8					ubSliderWidth;
	UINT8					ubSliderHeight;

	struct TAG_SLIDER		*pNext;
	struct TAG_SLIDER		*pPrev;

} SLIDER;


SLIDER *pSliderHead = NULL;
UINT32	guiCurrentSliderID=1;

BOOLEAN	gfSliderInited=FALSE;

BOOLEAN	gfCurrentSliderIsAnchored=FALSE;		//if true, the current selected slider mouse button is down
SLIDER	*gpCurrentSlider=NULL;

UINT32	guiSliderBoxImage=0;


BOOLEAN InitSlider()
{
	// load Slider Box Graphic graphic and add it
	guiSliderBoxImage = AddVideoObjectFromFile("INTERFACE/SliderBox.sti");
	CHECKF(guiSliderBoxImage != NO_VOBJECT);

	gfSliderInited = TRUE;

	return( TRUE );
}

void ShutDownSlider()
{
	SLIDER *pRemove = NULL;
	SLIDER *pTemp = NULL;

	AssertMsg( gfSliderInited, "Trying to ShutDown the Slider System when it was never inited");

	//Do a cehck to see if there are still active nodes
	pTemp = pSliderHead;
	while( pTemp )
	{
		pRemove = pTemp;
		pTemp = pTemp->pNext;
		RemoveSliderBar( pRemove->uiSliderID );

		//Report an error
	}

	//if so report an errror
	gfSliderInited = 0;
	DeleteVideoObjectFromIndex( guiSliderBoxImage );
}


static void CalculateNewSliderBoxPosition(SLIDER* pSlider);
static void SelectedSliderButtonCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedSliderMovementCallBack(MOUSE_REGION* pRegion, INT32 reason);


INT32	AddSlider( UINT8 ubStyle, UINT16 usCursor, UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT16 usNumberOfIncrements, INT8 sPriority, SLIDER_CHANGE_CALLBACK SliderChangeCallback, UINT32 uiFlags )
{
	SLIDER *pTemp = NULL;
	SLIDER *pNewSlider = NULL;

	AssertMsg( gfSliderInited, "Trying to Add a Slider Bar when the Slider System was never inited");

	//checks
	if( ubStyle >= NUM_SLIDER_STYLES )
		return( -1 );


	pNewSlider = MemAlloc( sizeof( SLIDER ) );
	if( pNewSlider == NULL )
	{
		return( -1 );
	}
	memset( pNewSlider, 0, sizeof( SLIDER ) );


	//Assign the settings to the current slider
	pNewSlider->ubStyle = ubStyle;
	pNewSlider->usPosX = usPosX;
	pNewSlider->usPosY = usPosY;
//	pNewSlider->usWidth = usWidth;
	pNewSlider->usNumberOfIncrements = usNumberOfIncrements;
	pNewSlider->SliderChangeCallback = SliderChangeCallback;
	pNewSlider->usCurrentIncrement = 0;
	pNewSlider->usBackGroundColor = Get16BPPColor( FROMRGB( 255, 255, 255 ) );
	pNewSlider->uiFlags = uiFlags;

	//Get a new Identifier for the slider
	//Temp just increment for now
	pNewSlider->uiSliderID = guiCurrentSliderID;

	//increment counter
	guiCurrentSliderID++;


	//
	// Create the mouse regions for each increment in the slider
	//

	//add the region
	usPosX = pNewSlider->usPosX;
	usPosY = pNewSlider->usPosY;

	//Add the last one, the width will be whatever is left over
	switch( ubStyle )
	{
		case SLIDER_VERTICAL_STEEL:

			pNewSlider->uiFlags |= SLIDER_VERTICAL;
			pNewSlider->usWidth = STEEL_SLIDER_WIDTH;
			pNewSlider->usHeight = usWidth;
			pNewSlider->ubSliderWidth = STEEL_SLIDER_WIDTH;
			pNewSlider->ubSliderHeight = STEEL_SLIDER_HEIGHT;


			MSYS_DefineRegion( &pNewSlider->ScrollAreaMouseRegion, (UINT16)(usPosX-pNewSlider->usWidth/2), usPosY, (UINT16)(usPosX+pNewSlider->usWidth/2), (UINT16)(pNewSlider->usPosY+pNewSlider->usHeight), sPriority,
									 usCursor, SelectedSliderMovementCallBack, SelectedSliderButtonCallBack );
			MSYS_SetRegionUserData( &pNewSlider->ScrollAreaMouseRegion, 1, pNewSlider->uiSliderID );
			break;

		case SLIDER_DEFAULT_STYLE:
		default:

			pNewSlider->uiFlags |= SLIDER_HORIZONTAL;
			pNewSlider->usWidth = usWidth;
			pNewSlider->usHeight = DEFUALT_SLIDER_SIZE;

			MSYS_DefineRegion( &pNewSlider->ScrollAreaMouseRegion, usPosX, (UINT16)(usPosY-DEFUALT_SLIDER_SIZE), (UINT16)(pNewSlider->usPosX+pNewSlider->usWidth), (UINT16)(usPosY+DEFUALT_SLIDER_SIZE), sPriority,
									 usCursor, SelectedSliderMovementCallBack, SelectedSliderButtonCallBack );
			MSYS_SetRegionUserData( &pNewSlider->ScrollAreaMouseRegion, 1, pNewSlider->uiSliderID );
		break;
	}


	//
	//	Load the graphic image for the slider box
	//

	//add the slider into the list
	pTemp = pSliderHead;


	//if its the first time in
	if( pSliderHead == NULL )
	{
		pSliderHead = pNewSlider;
		pNewSlider->pNext = NULL;
	}
	else
	{
		while( pTemp->pNext != NULL )
		{
			pTemp = pTemp->pNext;
		}

		pTemp->pNext = pNewSlider;
		pNewSlider->pPrev = pTemp;
		pNewSlider->pNext = NULL;
	}

	CalculateNewSliderBoxPosition( pNewSlider );

	return( pNewSlider->uiSliderID );
}


static void CalculateNewSliderIncrement(UINT32 uiSliderID, UINT16 usPos);
static void RenderSelectedSliderBar(SLIDER* pSlider);


void RenderAllSliderBars()
{
	SLIDER *pTemp = NULL;

	// set the currently selectd slider bar
	if( gfLeftButtonState && gpCurrentSlider != NULL )
	{
		UINT16 usPosY = 0;

		if( gusMouseYPos < gpCurrentSlider->usPosY )
			usPosY = 0;
		else
			usPosY = gusMouseYPos - gpCurrentSlider->usPosY;

		//if the mouse
		CalculateNewSliderIncrement( gpCurrentSlider->uiSliderID,  usPosY );
	}
	else
	{
		gpCurrentSlider = NULL;
	}



	pTemp = pSliderHead;

	while( pTemp )
	{
		RenderSelectedSliderBar( pTemp );

		pTemp = pTemp->pNext;
	}
}


static void OptDisplayLine(UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY, INT16 iColor);
static void RenderSliderBox(SLIDER* pSlider);


static void RenderSelectedSliderBar(SLIDER* pSlider)
{


	if( pSlider->uiFlags & SLIDER_VERTICAL )
	{
	}
	else
	{
		//display the background ( the bar )
		OptDisplayLine( (UINT16)(pSlider->usPosX+1), (UINT16)(pSlider->usPosY-1), (UINT16)(pSlider->usPosX + pSlider->usWidth-1), (UINT16)(pSlider->usPosY-1), pSlider->usBackGroundColor );
		OptDisplayLine( pSlider->usPosX, pSlider->usPosY, (UINT16)(pSlider->usPosX + pSlider->usWidth), pSlider->usPosY, pSlider->usBackGroundColor );
		OptDisplayLine( (UINT16)(pSlider->usPosX+1), (UINT16)(pSlider->usPosY+1), (UINT16)(pSlider->usPosX + pSlider->usWidth-1), (UINT16)(pSlider->usPosY+1), pSlider->usBackGroundColor );

		//invalidate the area
		InvalidateRegion( pSlider->usPosX, pSlider->usPosY-2, pSlider->usPosX+pSlider->usWidth+1, pSlider->usPosY+2 );
	}

	RenderSliderBox( pSlider );
}


static void RenderSliderBox(SLIDER* pSlider)
{
	SGPRect		DestRect;


	if( pSlider->uiFlags & SLIDER_VERTICAL )
	{
		//fill out the settings for the current dest rect
		DestRect.iLeft = pSlider->usPosX - pSlider->ubSliderWidth / 2;
		DestRect.iTop = pSlider->usCurrentSliderBoxPosition - pSlider->ubSliderHeight/2;
		DestRect.iRight = DestRect.iLeft + pSlider->ubSliderWidth;
		DestRect.iBottom = DestRect.iTop + pSlider->ubSliderHeight;


		//If it is not the first time to render the slider
		if( !( pSlider->LastRect.iLeft == 0 && pSlider->LastRect.iRight == 0 ) )
		{
			//Restore the old rect
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, pSlider->LastRect.iLeft, pSlider->LastRect.iTop, pSlider->ubSliderWidth, pSlider->ubSliderHeight);

			//invalidate the old area
			InvalidateRegion( pSlider->LastRect.iLeft, pSlider->LastRect.iTop, pSlider->LastRect.iRight, pSlider->LastRect.iBottom );
		}

		//Blit the new rect
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.iLeft, DestRect.iTop, pSlider->ubSliderWidth, pSlider->ubSliderHeight);
	}
	else
	{
		//fill out the settings for the current dest rect
		DestRect.iLeft = pSlider->usCurrentSliderBoxPosition;
		DestRect.iTop = pSlider->usPosY-DEFUALT_SLIDER_SIZE;
		DestRect.iRight = DestRect.iLeft + pSlider->ubSliderWidth;
		DestRect.iBottom = DestRect.iTop + pSlider->ubSliderHeight;

		//If it is not the first time to render the slider
		if( !( pSlider->LastRect.iLeft == 0 && pSlider->LastRect.iRight == 0 ) )
		{
			//Restore the old rect
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, pSlider->LastRect.iLeft, pSlider->LastRect.iTop, 8, 15);
		}

		//save the new rect
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.iLeft, DestRect.iTop, 8, 15);
	}


	//Save the new rect location
	pSlider->LastRect = DestRect;


	if( pSlider->uiFlags & SLIDER_VERTICAL )
	{
		BltVideoObjectFromIndex(FRAME_BUFFER, guiSliderBoxImage, 0, pSlider->LastRect.iLeft, pSlider->LastRect.iTop);

		//invalidate the area
		InvalidateRegion( pSlider->LastRect.iLeft, pSlider->LastRect.iTop, pSlider->LastRect.iRight, pSlider->LastRect.iBottom );
	}
	else
	{
		BltVideoObjectFromIndex(FRAME_BUFFER, guiSliderBoxImage, 0, pSlider->usCurrentSliderBoxPosition, pSlider->usPosY - DEFUALT_SLIDER_SIZE);

		//invalidate the area
		InvalidateRegion( pSlider->usCurrentSliderBoxPosition, pSlider->usPosY-DEFUALT_SLIDER_SIZE, pSlider->usCurrentSliderBoxPosition+9, pSlider->usPosY+DEFUALT_SLIDER_SIZE );
	}
}


void RemoveSliderBar( UINT32 uiSliderID )
{
	SLIDER *pTemp = NULL;
	SLIDER *pNodeToRemove = NULL;
//	UINT32	cnt;

	pTemp = pSliderHead;

	//Get the required slider
	while( pTemp && pTemp->uiSliderID != uiSliderID )
	{
		pTemp = pTemp->pNext;
	}

	//if we could not find the required slider
	if( pTemp == NULL )
	{
		//return an error
		return;
	}

	pNodeToRemove = pTemp;

	if( pTemp == pSliderHead )
		pSliderHead = pSliderHead->pNext;

	//Detach the node.
	if( pTemp->pNext )
		pTemp->pNext->pPrev = pTemp->pPrev;

	if( pTemp->pPrev )
		pTemp->pPrev->pNext = pTemp->pNext;

	MSYS_RemoveRegion( &pNodeToRemove->ScrollAreaMouseRegion );

	//if its the last node
	if( pNodeToRemove == pSliderHead )
		pSliderHead = NULL;

	//Remove the slider node
	MemFree( pNodeToRemove );
	pNodeToRemove = NULL;
}


static SLIDER* GetSliderFromID(UINT32 uiSliderID);


static void SelectedSliderMovementCallBack(MOUSE_REGION* pRegion, INT32 reason)
{
	UINT32	uiSelectedSlider;
	SLIDER *pSlider=NULL;


	//if we already have an anchored slider bar
	if( gpCurrentSlider != NULL )
		return;

	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		pRegion->uiFlags &= (~BUTTON_CLICKED_ON );

		if( gfLeftButtonState )
		{
			uiSelectedSlider = MSYS_GetRegionUserData( pRegion, 1 );
			pSlider = GetSliderFromID( uiSelectedSlider );
			if( pSlider == NULL )
				return;

			// set the currently selectd slider bar
			if( gfLeftButtonState )
			{
				gpCurrentSlider = pSlider;
			}


			if( pSlider->uiFlags & SLIDER_VERTICAL )
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeYPos );
			}
			else
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeXPos );
			}
		}
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		pRegion->uiFlags |= BUTTON_CLICKED_ON ;

		if( gfLeftButtonState )
		{
			uiSelectedSlider = MSYS_GetRegionUserData( pRegion, 1 );
			pSlider = GetSliderFromID( uiSelectedSlider );
			if( pSlider == NULL )
				return;

			// set the currently selectd slider bar
//			gpCurrentSlider = pSlider;


			if( pSlider->uiFlags & SLIDER_VERTICAL )
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeYPos );
			}
			else
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeXPos );
			}

		}
	}

	else if( reason & MSYS_CALLBACK_REASON_MOVE )
	{
		pRegion->uiFlags |= BUTTON_CLICKED_ON ;

		if( gfLeftButtonState )
		{
			uiSelectedSlider = MSYS_GetRegionUserData( pRegion, 1 );
			pSlider = GetSliderFromID( uiSelectedSlider );
			if( pSlider == NULL )
				return;

			// set the currently selectd slider bar
//			gpCurrentSlider = pSlider;

			if( pSlider->uiFlags & SLIDER_VERTICAL )
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeYPos );
			}
			else
			{
				CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeXPos );
			}
		}
	}
}


static void SelectedSliderButtonCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32	uiSelectedSlider;
	SLIDER *pSlider=NULL;

	//if we already have an anchored slider bar
	if( gpCurrentSlider != NULL )
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
 		uiSelectedSlider = MSYS_GetRegionUserData( pRegion, 1 );

		pSlider = GetSliderFromID( uiSelectedSlider );
		if( pSlider == NULL )
			return;


/*		// set the currently selectd slider bar
		if( gfLeftButtonState )
		{
			gpCurrentSlider = pSlider;
		}
*/

		if( pSlider->uiFlags & SLIDER_VERTICAL )
		{
			CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeYPos );
		}
		else
		{
			CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeXPos );
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		uiSelectedSlider = MSYS_GetRegionUserData( pRegion, 1 );

		pSlider = GetSliderFromID( uiSelectedSlider );
		if( pSlider == NULL )
			return;

		// set the currently selectd slider bar
/*		if( gfLeftButtonState )
		{
			gpCurrentSlider = pSlider;
		}
*/

		if( pSlider->uiFlags & SLIDER_VERTICAL )
		{
			CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeYPos );
		}
		else
		{
			CalculateNewSliderIncrement( uiSelectedSlider, pRegion->RelativeXPos );
		}
	}
}


static void CalculateNewSliderIncrement(UINT32 uiSliderID, UINT16 usPos)
{
	FLOAT		dNewIncrement=0.0;
	SLIDER *pSlider;
	UINT16	usOldIncrement;
	BOOLEAN	fLastSpot=FALSE;
	BOOLEAN	fFirstSpot=FALSE;

	pSlider = GetSliderFromID( uiSliderID );
	if( pSlider == NULL )
		return;

	usOldIncrement = pSlider->usCurrentIncrement;

	if( pSlider->uiFlags & SLIDER_VERTICAL )
	{
		if( usPos >= (UINT16)(pSlider->usHeight * (FLOAT).99 ) )
			fLastSpot = TRUE;

		if( usPos <= (UINT16)(pSlider->usHeight * (FLOAT).01 ) )
			fFirstSpot = TRUE;


		//pSlider->usNumberOfIncrements
		if( fFirstSpot )
			dNewIncrement = 0;
		else if( fLastSpot )
			dNewIncrement = pSlider->usNumberOfIncrements;
		else
			dNewIncrement = ( usPos / (FLOAT)pSlider->usHeight ) * pSlider->usNumberOfIncrements;
	}
	else
	{
		dNewIncrement = ( usPos / (FLOAT)pSlider->usWidth ) * pSlider->usNumberOfIncrements;
	}


	pSlider->usCurrentIncrement = (UINT16)( dNewIncrement + .5 );

	CalculateNewSliderBoxPosition( pSlider );


	//if the the new value is different
	if( usOldIncrement != pSlider->usCurrentIncrement )
	{
		if( pSlider->uiFlags & SLIDER_VERTICAL )
		{
			//Call the call back for the slider
			(*(pSlider->SliderChangeCallback) )( pSlider->usNumberOfIncrements - pSlider->usCurrentIncrement );
		}
		else
		{
			//Call the call back for the slider
			(*(pSlider->SliderChangeCallback) )( pSlider->usCurrentIncrement );
		}
	}

}


static void OptDisplayLine(UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY, INT16 iColor)
{
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );

	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  // draw the line
	LineDraw(FALSE, usStartX, usStartY, EndX, EndY, iColor, pDestBuf);

	// unlock frame buffer
	UnLockVideoSurface( FRAME_BUFFER );
}


static void CalculateNewSliderBoxPosition(SLIDER* pSlider)
{
	UINT16	usMaxPos;

	if( pSlider->uiFlags & SLIDER_VERTICAL )
	{
		//if the box is in the last position
		if( pSlider->usCurrentIncrement >= ( pSlider->usNumberOfIncrements ) )
		{
			pSlider->usCurrentSliderBoxPosition = pSlider->usPosY + pSlider->usHeight;// - pSlider->ubSliderHeight / 2;	// - minus box width
		}

		//else if the box is in the first position
		else if( pSlider->usCurrentIncrement == 0 )
		{
			pSlider->usCurrentSliderBoxPosition = pSlider->usPosY;// - pSlider->ubSliderHeight / 2;
		}
		else
		{
			pSlider->usCurrentSliderBoxPosition = pSlider->usPosY + (UINT16)( ( pSlider->usHeight / (FLOAT)pSlider->usNumberOfIncrements ) * pSlider->usCurrentIncrement );
		}

		usMaxPos = pSlider->usPosY + pSlider->usHeight;// - pSlider->ubSliderHeight//2 + 1;

		//if the box is past the edge, move it back
		if( pSlider->usCurrentSliderBoxPosition > usMaxPos )
			pSlider->usCurrentSliderBoxPosition = usMaxPos;
	}
	else
	{
		//if the box is in the last position
		if( pSlider->usCurrentIncrement == ( pSlider->usNumberOfIncrements ) )
		{
			pSlider->usCurrentSliderBoxPosition = pSlider->usPosX + pSlider->usWidth - 8 + 1;	// - minus box width
		}
		else
		{
			pSlider->usCurrentSliderBoxPosition = pSlider->usPosX + (UINT16)( ( pSlider->usWidth / (FLOAT)pSlider->usNumberOfIncrements ) * pSlider->usCurrentIncrement );
		}
		usMaxPos = pSlider->usPosX + pSlider->usWidth - 8+1;

		//if the box is past the edge, move it back
		if( pSlider->usCurrentSliderBoxPosition > usMaxPos )
			pSlider->usCurrentSliderBoxPosition = usMaxPos;
	}
}


static SLIDER* GetSliderFromID(UINT32 uiSliderID)
{
	SLIDER *pTemp = NULL;

	pTemp = pSliderHead;

	//Get the required slider
	while( pTemp && pTemp->uiSliderID != uiSliderID )
	{
		pTemp = pTemp->pNext;
	}

	// if we couldnt find the right slider
	if( pTemp == NULL )
		return( NULL );

	return( pTemp );
}


void SetSliderValue( UINT32 uiSliderID, UINT32 uiNewValue )
{
	SLIDER *pSlider = NULL;

	pSlider = GetSliderFromID( uiSliderID );
	if( pSlider == NULL )
		return;

	if( uiNewValue >= pSlider->usNumberOfIncrements )
		return;

	if( pSlider->uiFlags & SLIDER_VERTICAL )
		pSlider->usCurrentIncrement = pSlider->usNumberOfIncrements - (UINT16)uiNewValue;
	else
		pSlider->usCurrentIncrement = (UINT16)uiNewValue;

	CalculateNewSliderBoxPosition( pSlider );
}
