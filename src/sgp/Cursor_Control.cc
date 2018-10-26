#include "ContentManager.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Debug.h"
#include "GameInstance.h"
#include "HImage.h"
#include "Input.h"
#include "Timer.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include "VideoScale.h"
#include "UILayout.h"

#include "policy/GamePolicy.h"

#include <optional>


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Database
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static BOOLEAN gfCursorDatabaseInit = FALSE;

static CursorFileData* gpCursorFileDatabase;
static CursorData*     gpCursorDatabase;
INT16  gsGlobalCursorYOffset = 0;
UINT16 gsCurMouseHeight = 0;
UINT16 gsCurMouseWidth = 0;
static UINT16 gusNumDataFiles = 0;
static SGPVObject const* guiExternVo;
static UINT16            gusExternVoSubIndex;
static UINT32 guiOldSetCursor = 0;
static UINT32 guiDelayTimer = 0;

static MOUSEBLT_HOOK gMouseBltOverride = NULL;

std::optional<SGPPoint> gManualCursorPos = std::nullopt;

void GetCursorPos(SGPPoint& point)
{
	if (gManualCursorPos) {
		point.iX = gManualCursorPos->iX;
		point.iY = gManualCursorPos->iY;
		return;
	}
	GetMousePos(&point);
}

void SetManualCursorPos(SGPPoint point) {
	gManualCursorPos = std::make_optional(point);
}

void ClearManualCursorPos() {
	gManualCursorPos = std::nullopt;
}

static void EraseMouseCursor(void)
{
	MOUSE_BUFFER->Fill(0x000000FF);
}


static void BltToMouseCursorFromVObject(HVOBJECT hVObject, UINT16 usVideoObjectSubIndex, UINT16 usXPos, UINT16 usYPos)
{
	BltVideoObject(MOUSE_BUFFER, hVObject, usVideoObjectSubIndex, usXPos, usYPos);
}


static void BltToMouseCursorFromVObjectWithOutline(HVOBJECT hVObject, UINT16 usVideoObjectSubIndex, UINT16 usXPos, UINT16 usYPos)
{
	// Center and adjust for offsets
	ETRLEObject const& pTrav = hVObject->SubregionProperties(usVideoObjectSubIndex);
	INT16       const  sXPos = (gsCurMouseWidth  - pTrav.usWidth)  / 2 - pTrav.sOffsetX;
	INT16       const  sYPos = (gsCurMouseHeight - pTrav.usHeight) / 2 - pTrav.sOffsetY;
	BltVideoObjectOutline(MOUSE_BUFFER, hVObject, usVideoObjectSubIndex, sXPos, sYPos, RGB(0, 255, 0));
}


// THESE TWO PARAMETERS MUST POINT TO STATIC OR GLOBAL DATA, NOT AUTOMATIC VARIABLES
void InitCursorDatabase(CursorFileData* pCursorFileData, CursorData* pCursorData, UINT16 suNumDataFiles)
{
	// Set global values!
	gpCursorFileDatabase = pCursorFileData;
	gpCursorDatabase     = pCursorData;
	gusNumDataFiles      = suNumDataFiles;
	gfCursorDatabaseInit = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Handlers
//
///////////////////////////////////////////////////////////////////////////////////////////////////

static void LoadCursorData(UINT32 uiCursorIndex)
{
	// Load cursor data will load all data required for the cursor specified by this index
	CursorData* pCurData = &gpCursorDatabase[uiCursorIndex];

	INT16 sMaxHeight = -1;
	INT16 sMaxWidth  = -1;
	for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
	{
		const CursorImage* pCurImage = &pCurData->Composites[cnt];
		CursorFileData*    CFData    = &gpCursorFileDatabase[pCurImage->uiFileIndex];

		if (CFData->hVObject == NULL)
		{
			// The file containing the video object hasn't been loaded yet. Let's load it now
			// First load as an SGPImage so we can get aux data!
			Assert(CFData->Filename != NULL);

			AutoSGPImage img(CreateImage(CFData->Filename, IMAGE_ALLDATA | IMAGE_REMOVE_PAL254));
			AutoSGPImage hImage(ScaleImage(img.get(), g_ui.m_stdScreenScale));

			CFData->hVObject = AddVideoObjectFromHImage(hImage.get());

			// Check for animated tile
			if (hImage->uiAppDataSize > 0)
			{
				// Valid auxiliary data, so get # od frames from data
				AuxObjectData const* const pAuxData = (AuxObjectData const*)(UINT8 const*)hImage->pAppData;
				if (pAuxData->fFlags & AUX_ANIMATED_TILE)
				{
					CFData->ubNumberOfFrames = pAuxData->ubNumberOfFrames;
				}
			}
		}

		// Get ETRLE Data for this video object
		ETRLEObject const& pTrav = CFData->hVObject->SubregionProperties(pCurImage->uiSubIndex);

		if (pTrav.usHeight > sMaxHeight) sMaxHeight = pTrav.usHeight;
		if (pTrav.usWidth  > sMaxWidth)  sMaxWidth  = pTrav.usWidth;
	}


	pCurData->usHeight = sMaxHeight;
	pCurData->usWidth = sMaxWidth;


	switch (pCurData->sOffsetX)
	{
		case CENTER_CURSOR: pCurData->sOffsetX = pCurData->usWidth / 2; break;
		case RIGHT_CURSOR:  pCurData->sOffsetX = pCurData->usWidth;     break;
		case LEFT_CURSOR:   pCurData->sOffsetX = 0;                     break;
	}

	switch (pCurData->sOffsetY)
	{
		case CENTER_CURSOR: pCurData->sOffsetY = pCurData->usHeight / 2; break;
		case BOTTOM_CURSOR: pCurData->sOffsetY = pCurData->usHeight;     break;
		case TOP_CURSOR:    pCurData->sOffsetY = 0;                      break;
	}

	gsCurMouseHeight = pCurData->usHeight;
	gsCurMouseWidth  = pCurData->usWidth;

	// Adjust relative offsets
	for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
	{
		CursorImage* pCurImage = &pCurData->Composites[cnt];

		// Get ETRLE Data for this video object
		ETRLEObject const& pTrav = gpCursorFileDatabase[pCurImage->uiFileIndex].hVObject->SubregionProperties(pCurImage->uiSubIndex);

		if (pCurImage->usPosX == CENTER_SUBCURSOR)
		{
			pCurImage->usPosX = pCurData->sOffsetX - pTrav.usWidth / 2;
		}

		if (pCurImage->usPosY == CENTER_SUBCURSOR)
		{
			pCurImage->usPosY = pCurData->sOffsetY - pTrav.usHeight / 2;
		}
	}
}


static void UnLoadCursorData(UINT32 uiCursorIndex)
{
	// This function will unload add data used for this cursor
	//
	// Ok, first we make sure that the video object file is indeed loaded. Once this is verified, we will
	// move on to the deletion
	const CursorData* pCurData = &gpCursorDatabase[uiCursorIndex];

	for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
	{
		const CursorImage* pCurImage = &pCurData->Composites[cnt];
		CursorFileData* CFData = &gpCursorFileDatabase[pCurImage->uiFileIndex];

		if (CFData->hVObject != NULL && CFData->Filename != NULL)
		{
			DeleteVideoObject(CFData->hVObject);
			CFData->hVObject = NULL;
		}
	}
}


void CursorDatabaseClear(void)
{
	for (UINT32 uiIndex = 0; uiIndex < gusNumDataFiles; uiIndex++)
	{
		CursorFileData* CFData = &gpCursorFileDatabase[uiIndex];
		if (CFData->hVObject != NULL && CFData->Filename != NULL)
		{
			DeleteVideoObject(CFData->hVObject);
			CFData->hVObject = NULL;
		}
	}
}


BOOLEAN SetCurrentCursorFromDatabase(UINT32 uiCursorIndex)
{
	uiCursorIndex = ModifyCursorIndex(uiCursorIndex);

	if (uiCursorIndex == VIDEO_NO_CURSOR)
	{
		SetMouseCursorProperties(0, 0, 0, 0);
	}
	else if (gfCursorDatabaseInit)
	{
		// CHECK FOR EXTERN CURSOR
		if (uiCursorIndex == EXTERN_CURSOR)
		{
			// Erase old cursor
			EraseMouseCursor();

			ETRLEObject const& pTrav       = guiExternVo->SubregionProperties(gusExternVoSubIndex);
			UINT16      const  usEffHeight = pTrav.usHeight + pTrav.sOffsetY;
			UINT16      const  usEffWidth  = pTrav.usWidth  + pTrav.sOffsetX;

			BltVideoObjectOutline(MOUSE_BUFFER, guiExternVo, gusExternVoSubIndex, 0, 0, SGP_TRANSPARENT);

			// Hook into hook function
			if (gMouseBltOverride != NULL) gMouseBltOverride();

			SetMouseCursorProperties(usEffWidth / 2, usEffHeight / 2, usEffHeight, usEffWidth);
		}
		else
		{
			const CursorData* pCurData = &gpCursorDatabase[uiCursorIndex];

			// Disable specific pointing cursors on touch devices
			if (pCurData->fHideOnTouch && IsUsingTouch()) {
				uiCursorIndex = VIDEO_NO_CURSOR;
				guiOldSetCursor = VIDEO_NO_CURSOR;
				SetMouseCursorProperties(0, 0, 0, 0);
				return TRUE;
			}

			// First check if we are a differnet curosr...
			if (uiCursorIndex != guiOldSetCursor)
			{
				// OK, check if we are a delay cursor...
				if (pCurData->bFlags & DELAY_START_CURSOR)
				{
					guiDelayTimer = GetClock();
				}
			}

			guiOldSetCursor = uiCursorIndex;

			// Olny update if delay timer has elapsed...
			if (pCurData->bFlags & DELAY_START_CURSOR)
			{
				if (GetClock() - guiDelayTimer < 1000)
				{
					SetMouseCursorProperties(0, 0, 0, 0);
					return TRUE;
				}
			}

			// Call LoadCursorData to make sure that the video object is loaded
			LoadCursorData(uiCursorIndex);

			// Erase old cursor
			EraseMouseCursor();
			// NOW ACCOMODATE COMPOSITE CURSORS
			pCurData = &gpCursorDatabase[uiCursorIndex];

			for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
			{
				// Check if we are a flashing cursor!
				if (pCurData->bFlags & CURSOR_TO_FLASH &&
						cnt <= 1 &&
						pCurData->bFlashIndex != cnt)
				{
					continue;
				}
				// Check if we are a sub cursor!
				// IN this case, do all frames but
				// skip the 1st or second!

				if (pCurData->bFlags & CURSOR_TO_SUB_CONDITIONALLY)
				{
					if (pCurData->bFlags & CURSOR_TO_FLASH2)
					{
						if (0 < cnt && cnt <= 2 && pCurData->bFlashIndex != cnt)
						{
							continue;
						}
					}
					else
					{
						if (cnt <= 1 && pCurData->bFlashIndex != cnt)
						{
							continue;
						}
					}
				}

				const CursorImage* pCurImage = &pCurData->Composites[cnt];
				CursorFileData*    CFData    = &gpCursorFileDatabase[pCurImage->uiFileIndex];

				// Adjust sub-index if cursor is animated
				UINT16 usSubIndex;
				if (CFData->ubNumberOfFrames != 0)
				{
					usSubIndex = pCurImage->uiCurrentFrame;
				}
				else
				{
					usSubIndex = pCurImage->uiSubIndex;
				}

				if (pCurImage->usPosX != HIDE_SUBCURSOR && pCurImage->usPosY != HIDE_SUBCURSOR)
				{
					// Blit cursor at position in mouse buffer
					if (CFData->ubFlags & USE_OUTLINE_BLITTER)
					{
						BltToMouseCursorFromVObjectWithOutline(CFData->hVObject, usSubIndex, pCurImage->usPosX, pCurImage->usPosY);
					}
					else
					{
						BltToMouseCursorFromVObject(CFData->hVObject, usSubIndex, pCurImage->usPosX, pCurImage->usPosY);
					}
				}
			}

			// Hook into hook function
			if (gMouseBltOverride != NULL) gMouseBltOverride();

			INT16 sCenterValX = pCurData->sOffsetX;
			INT16 sCenterValY = pCurData->sOffsetY;
			SetMouseCursorProperties(sCenterValX, sCenterValY + gsGlobalCursorYOffset, pCurData->usHeight, pCurData->usWidth);
		}
	}

	return TRUE;
}


void SetMouseBltHook(MOUSEBLT_HOOK pMouseBltOverride)
{
	gMouseBltOverride = pMouseBltOverride;
}


// Sets an external video object as cursor file data....
void SetExternVOData(UINT32 uiCursorIndex, HVOBJECT hVObject, UINT16 usSubIndex)
{
	CursorData* pCurData = &gpCursorDatabase[uiCursorIndex];

	for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
	{
		CursorImage*    pCurImage = &pCurData->Composites[cnt];
		CursorFileData* CFData    = &gpCursorFileDatabase[pCurImage->uiFileIndex];

		if (CFData->Filename == NULL)
		{
			// OK, set Video Object here....

			// If loaded, unload...
			UnLoadCursorData(uiCursorIndex);

			// Set extern vo
			CFData->hVObject      = hVObject;
			pCurImage->uiSubIndex = usSubIndex;

			// Reload....
			LoadCursorData(uiCursorIndex);
		}
	}
}


void SetExternMouseCursor(SGPVObject const& vo, UINT16 const region_idx)
{
	guiExternVo         = &vo;
	gusExternVoSubIndex = region_idx;
}

// Checks if uiCursorIndex is VIDEO_DEFAULT_TO_NO_CURSOR. If so, we parse it as VIDEO_NO_CURSOR or 0 (generic cursor)
UINT32 ModifyCursorIndex(UINT32 uiCursorIndex)
{
	if (uiCursorIndex == VIDEO_DEFAULT_TO_NO_CURSOR)
	{
		if (gamepolicy(always_show_cursor_in_tactical))
			return CURSOR_NORMAL;
		else
			return VIDEO_NO_CURSOR;
	}
	return uiCursorIndex;
}
