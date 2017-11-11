#include "Directories.h"
#include "HImage.h"
#include "MapScreen.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "StrategicMap.h"
#include "PopUpBox.h"
#include "Map_Screen_Interface.h"
#include "Queen_Command.h"
#include "Player_Command.h"
#include "Font_Control.h"
#include "Text.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface_Border.h"
#include "Handle_UI.h"
#include "NPC.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Mines.h"
#include "Finances.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Town_Militia.h"
#include "HelpScreen.h"
#include "Map_Screen_Helicopter.h"
#include "Tactical_Save.h"
#include "Button_System.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "VObject.h"
#include "VSurface.h"


#define BOX_BUTTON_HEIGHT 20


// flag to say if we are showing town/mine box at all
BOOLEAN fShowTownInfo = FALSE;

PopUpBox* ghTownMineBox;
SGPPoint TownMinePosition ={ 300, 150 };

INT8 bCurrentTownMineSectorX = 0;
INT8 bCurrentTownMineSectorY = 0;
INT8 bCurrentTownMineSectorZ = 0;

// inventory button
static BUTTON_PICS* guiMapButtonInventoryImage[2];
static GUIButtonRef guiMapButtonInventory[2];

static UINT16 sTotalButtonWidth = 0;


// callback to turn on sector invneotry list
static void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, INT32 reason);
static void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, INT32 reason);


void DisplayTownInfo( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	// will display town info for a particular town

	// set current sector
	if( ( bCurrentTownMineSectorX != sMapX ) || ( bCurrentTownMineSectorY != sMapY ) || ( bCurrentTownMineSectorZ != bMapZ ) )
	{
		bCurrentTownMineSectorX = ( INT8 )sMapX;
		bCurrentTownMineSectorY = ( INT8 )sMapY;
		bCurrentTownMineSectorZ = bMapZ;
	}

	//create destroy the box
	CreateDestroyTownInfoBox( );

}


static void AddCommonInfoToBox(PopUpBox*);
static void AddInventoryButtonForMapPopUpBox(const PopUpBox*);
static void AddItemsInSectorToBox(PopUpBox*);
static void AddSectorToBox(PopUpBox*);
static void AddTextToBlankSectorBox(PopUpBox*);
static void AddTextToMineBox(PopUpBox*, INT8 mine);
static void AddTextToTownBox(PopUpBox*);
static void MinWidthOfTownMineInfoBox(void);
static void PositionTownMineInfoBox(PopUpBox*);
static void RemoveInventoryButtonForMapPopUpBox(void);


void CreateDestroyTownInfoBox(void)
{
	PopUpBox* box = ghTownMineBox;
	if (box == NO_POPUP_BOX && fShowTownInfo)
	{
		box = CreatePopUpBox(TownMinePosition, 0, FRAME_BUFFER, guiPOPUPBORDERS, guiPOPUPTEX, 6, 6, 8 + BOX_BUTTON_HEIGHT, 6, 2);
		ghTownMineBox = box;

		// decide what kind of text to add to display
		if (bCurrentTownMineSectorZ == 0)
		{
			UINT8 const sector = SECTOR(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
			// only show the mine info when mines button is selected, otherwise we need to see the sector's regular town info
			if (fShowMineFlag)
			{
				INT8 const mine = GetMineIndexForSector(sector);
				if (mine == -1) goto no_mine;
				AddTextToMineBox(box, mine);
			}
			else
			{
no_mine:
				// do we add text for the town box?
				INT8 const bTownId = GetTownIdForSector(sector);
				if (bTownId != BLANK_SECTOR)
				{
					// add text for town box
					AddTextToTownBox(box);
				}
				else
				{
					// just a blank sector (handles SAM sites if visible)
					AddTextToBlankSectorBox(box);
				}
			}

			// add "militia", "militia training", "control" "enemy forces", etc. lines text to any popup box
			AddCommonInfoToBox(box);
		}
		else // underground
		{
			AddSectorToBox(box);
		}

		AddItemsInSectorToBox(box);

		SetBoxFont(                     box, BLOCKFONT2);
		SetBoxHighLight(                box, FONT_WHITE);
		SetBoxSecondColumnForeground(   box, FONT_WHITE);
		SetBoxSecondColumnBackground(   box, FONT_BLACK);
		SetBoxSecondColumnHighLight(    box, FONT_WHITE);
		SetBoxSecondColumnShade(        box, FONT_BLACK);
		SetBoxSecondColumnMinimumOffset(box, 20);
		SetBoxForeground(               box, FONT_YELLOW);
		SetBoxBackground(               box, FONT_BLACK);
		SetBoxShade(                    box, FONT_BLACK);

		// give title line (0) different color from the rest
		SetBoxLineForeground(box, 0, FONT_LTGREEN);

		MinWidthOfTownMineInfoBox();
		SpecifyBoxMinWidth(box, sTotalButtonWidth + 30);
		ResizeBoxToText(box);

		ShowBox(box);

		PositionTownMineInfoBox(box);
		AddInventoryButtonForMapPopUpBox(box);
	}
	else if (box != NO_POPUP_BOX && !fShowTownInfo)
	{
		SGPBox const area = GetBoxArea(box);

		RemoveBox(box);
		ghTownMineBox = NO_POPUP_BOX;

		RemoveInventoryButtonForMapPopUpBox();

		RestoreExternBackgroundRect(area.x, area.y, area.w, area.h + 3);
	}
}


// adds text to town info box
static void AddTextToTownBox(PopUpBox* const box)
{
	wchar_t wString[ 64 ];
	INT16 sMineSector = 0;

	UINT8 const sector   = SECTOR(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
	UINT8 const ubTownId = GetTownIdForSector(sector);
	Assert((ubTownId >= FIRST_TOWN) && (ubTownId < NUM_TOWNS));

	const wchar_t* title;
	switch (sector)
	{
		case SEC_B13: title = pLandTypeStrings[DRASSEN_AIRPORT_SITE];                    break;
		case SEC_F8:  title = pLandTypeStrings[CAMBRIA_HOSPITAL_SITE];                   break;
		case SEC_J9:  title = (fFoundTixa ? pTownNames[TIXA] : pLandTypeStrings[SAND]);  break;
		case SEC_K4:  title = (fFoundOrta ? pTownNames[ORTA] : pLandTypeStrings[SWAMP]); break;
		case SEC_N3:  title = pLandTypeStrings[MEDUNA_AIRPORT_SITE];                     break;

		case SEC_N4:
			if (fSamSiteFound[SAM_SITE_FOUR])
			{
				title = pLandTypeStrings[MEDUNA_SAM_SITE];
				break;
			}
			/* FALLTHROUGH */

		default: title = pTownNames[ubTownId]; break;
	}
	AddMonoString(box, title);
	// blank line
	AddMonoString(box, L"");

	AddSectorToBox(box);

	// town size
	swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 0 ] );
	AddMonoString(box, wString);
	swprintf( wString, lengthof(wString), L"%d",  GetTownSectorSize( ubTownId ) );
	AddSecondColumnMonoString(box, wString);

	// main facilities
	swprintf(wString, lengthof(wString), L"%ls:", pwTownInfoStrings[4]);
	AddMonoString(box, wString);
	GetSectorFacilitiesFlags( bCurrentTownMineSectorX, bCurrentTownMineSectorY, wString, lengthof(wString));
	AddSecondColumnMonoString(box, wString);

	// the concept of control is only meaningful in sectors where militia can be trained
	if ( MilitiaTrainingAllowedInSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY, 0 ) )
	{
		// town control
		swprintf(wString, lengthof(wString), L"%ls:", pwTownInfoStrings[1]);
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", GetTownSectorsUnderControl(ubTownId) * 100 / GetTownSectorSize(ubTownId));
		AddSecondColumnMonoString(box, wString);
	}

	// the concept of town loyalty is only meaningful in towns where loyalty is tracked
	if( gTownLoyalty[ ubTownId ].fStarted && gfTownUsesLoyalty[ ubTownId ])
	{
		// town loyalty
		swprintf(wString, lengthof(wString), L"%ls:", pwTownInfoStrings[3]);
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", gTownLoyalty[ubTownId].ubRating);
		AddSecondColumnMonoString(box, wString);
	}

	// if the town has a mine
	sMineSector = GetMineSectorForTown( ubTownId );
	if( sMineSector != -1 )
	{
		// Associated Mine: Sector
		swprintf(wString, lengthof(wString), L"%ls:",  pwTownInfoStrings[2]);
		AddMonoString(box, wString);
		GetShortSectorString( ( INT16 )( sMineSector % MAP_WORLD_X ), ( INT16 )( sMineSector / MAP_WORLD_X ), wString, lengthof(wString));
		AddSecondColumnMonoString(box, wString);
	}
}


// adds text to mine info box
static void AddTextToMineBox(PopUpBox* const box, INT8 const mine)
{
	UINT8            const  town   = GetTownAssociatedWithMine(mine);
	MINE_STATUS_TYPE const& status = gMineStatus[mine];
	wchar_t                 buf[64];

	// Name of town followed by "mine"
	swprintf(buf, lengthof(buf), L"%ls %ls", pTownNames[town], pwMineStrings[0]);
	AddMonoString(box, buf);

	AddMonoString(box, L""); // Blank line

	AddSectorToBox(box);

	// Mine status
	swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[9]);
	AddMonoString(box, buf);
	wchar_t const* const status_txt =
		status.fEmpty      ? pwMineStrings[5] : // Abandonded
		status.fShutDown   ? pwMineStrings[6] : // Shut down
		status.fRunningOut ? pwMineStrings[7] : // Running out
		pwMineStrings[8];                       // Producing
	AddSecondColumnMonoString(box, status_txt);

	if (!status.fEmpty)
	{
		// Current production
		swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[3]);
		AddMonoString(box, buf);
		UINT32 const predicted_income = PredictDailyIncomeFromAMine(mine);
		SPrintMoney(buf, predicted_income);
		AddSecondColumnMonoString(box, buf);

		// Potential production
		swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[4]);
		AddMonoString(box, buf);
		UINT32 const max_removal = GetMaxDailyRemovalFromMine(mine);
		SPrintMoney(buf, max_removal);
		AddSecondColumnMonoString(box, buf);

		if (GetMaxPeriodicRemovalFromMine(mine) > 0)
		{ // Production rate (current production as a percentage of potential production)
			swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[10]);
			AddMonoString(box, buf);
			swprintf(buf, lengthof(buf), L"%d%%", predicted_income * 100 / max_removal);
			AddSecondColumnMonoString(box, buf);
		}

		// Town control percentage
		swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[12]);
		AddMonoString(box, buf);
		swprintf(buf, lengthof(buf), L"%d%%", GetTownSectorsUnderControl(town) * 100 / GetTownSectorSize(town));
		AddSecondColumnMonoString(box, buf);

		TOWN_LOYALTY const& loyalty = gTownLoyalty[town];
		if (loyalty.fStarted && gfTownUsesLoyalty[town])
		{ // Town loyalty percentage
			swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[13]);
			AddMonoString(box, buf);
			swprintf(buf, lengthof(buf), L"%d%%", loyalty.ubRating);
			AddSecondColumnMonoString(box, buf);
		}

		// Ore type (silver/gold)
		swprintf(buf, lengthof(buf), L"%ls:", pwMineStrings[11]);
		AddMonoString(box, buf);
		AddSecondColumnMonoString(box, status.ubMineType == SILVER_MINE ? pwMineStrings[1] : pwMineStrings[2]);
	}

#ifdef _DEBUG
	// Dollar amount remaining in mine
	AddMonoString(box, L"Remaining (DEBUG):");
	SPrintMoney(buf, GetTotalLeftInMine(mine));
	AddSecondColumnMonoString(box, buf);
#endif
}


// add text to non-town/non-mine the other boxes
static void AddTextToBlankSectorBox(PopUpBox* const box)
{
	UINT16 usSectorValue = 0;

	// get the sector value
	usSectorValue = SECTOR( bCurrentTownMineSectorX, bCurrentTownMineSectorY );

	const wchar_t* title;
	switch( usSectorValue )
	{
		case SEC_D2:  title = (fSamSiteFound[SAM_SITE_ONE]   ? pLandTypeStrings[TROPICS_SAM_SITE] : pLandTypeStrings[TROPICS]); break; // Chitzena SAM
		case SEC_D15: title = (fSamSiteFound[SAM_SITE_TWO]   ? pLandTypeStrings[SPARSE_SAM_SITE]  : pLandTypeStrings[SPARSE]);  break; // Drassen SAM
		case SEC_I8:  title = (fSamSiteFound[SAM_SITE_THREE] ? pLandTypeStrings[SAND_SAM_SITE]    : pLandTypeStrings[SAND]);    break; // Cambria SAM

		// SAM Site 4 in Meduna is within town limits, so it's handled in AddTextToTownBox()

		default: title = pLandTypeStrings[SectorInfo[usSectorValue].ubTraversability[THROUGH_STRATEGIC_MOVE]]; break;
	}
	AddMonoString(box, title);

	// blank line
	AddMonoString(box, L"");

	AddSectorToBox(box);
}


// add "sector" line text to any popup box
static void AddSectorToBox(PopUpBox* const box)
{
	wchar_t wString[ 64 ];
	wchar_t wString2[ 10 ];

	// sector
	swprintf( wString, lengthof(wString), L"%ls:", pwMiscSectorStrings[ 1 ]);
	AddMonoString(box, wString);

	GetShortSectorString( bCurrentTownMineSectorX, bCurrentTownMineSectorY, wString, lengthof(wString));
	if (bCurrentTownMineSectorZ != 0 )
	{
		swprintf( wString2, lengthof(wString2), L"-%d", bCurrentTownMineSectorZ );
		wcscat( wString, wString2 );
	}

	AddSecondColumnMonoString(box, wString);
}


static void AddCommonInfoToBox(PopUpBox* const box)
{
	wchar_t wString[ 64 ];
	BOOLEAN fUnknownSAMSite = FALSE;
	UINT8 ubMilitiaTotal = 0;
	UINT8 ubNumEnemies;


	switch( SECTOR( bCurrentTownMineSectorX, bCurrentTownMineSectorY ) )
	{
		case SEC_D2: //Chitzena SAM
			if( !fSamSiteFound[ SAM_SITE_ONE ] )
				fUnknownSAMSite = TRUE;
			break;
		case SEC_D15: //Drassen SAM
			if( !fSamSiteFound[ SAM_SITE_TWO ] )
				fUnknownSAMSite = TRUE;
			break;
		case SEC_I8: //Cambria SAM
			if( !fSamSiteFound[ SAM_SITE_THREE ] )
				fUnknownSAMSite = TRUE;
			break;
		// SAM Site 4 in Meduna is within town limits, so it's always controllable
		default:
			break;
	}

	// in sector where militia can be trained,
	// control of the sector matters, display who controls this sector.  Map brightness no longer gives this!
	if ( MilitiaTrainingAllowedInSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY, 0 ) && !fUnknownSAMSite )
	{
		// controlled:
		swprintf( wString, lengthof(wString), L"%ls:", pwMiscSectorStrings[ 4 ] );
		AddMonoString(box, wString);

		// No/Yes
		AddSecondColumnMonoString(box, pwMiscSectorStrings[StrategicMap[CALCULATE_STRATEGIC_INDEX(bCurrentTownMineSectorX, bCurrentTownMineSectorY)].fEnemyControlled ? 6 : 5]);

		// militia - is there any?
		swprintf(wString, lengthof(wString), L"%ls:", pwTownInfoStrings[6]);
		AddMonoString(box, wString);

		ubMilitiaTotal = CountAllMilitiaInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
		if (ubMilitiaTotal > 0)
		{
			// some militia, show total & their breakdown by level
			swprintf( wString, lengthof(wString), L"%d  (%d/%d/%d)", ubMilitiaTotal,
					MilitiaInSectorOfRank(bCurrentTownMineSectorX, bCurrentTownMineSectorY, GREEN_MILITIA),
					MilitiaInSectorOfRank(bCurrentTownMineSectorX, bCurrentTownMineSectorY, REGULAR_MILITIA),
					MilitiaInSectorOfRank(bCurrentTownMineSectorX, bCurrentTownMineSectorY, ELITE_MILITIA));
			AddSecondColumnMonoString(box, wString);
		}
		else
		{
			// no militia: don't bother displaying level breakdown
			AddSecondColumnMonoString(box, L"0");
		}


		// percentage of current militia squad training completed
		swprintf(wString, lengthof(wString), L"%ls:", pwTownInfoStrings[5]);
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", SectorInfo[SECTOR(bCurrentTownMineSectorX, bCurrentTownMineSectorY)].ubMilitiaTrainingPercentDone);
		AddSecondColumnMonoString(box, wString);
	}


	// enemy forces
	swprintf( wString, lengthof(wString), L"%ls:", pwMiscSectorStrings[ 0 ] );
	AddMonoString(box, wString);

	// how many are there, really?
	ubNumEnemies = NumEnemiesInSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY );

	switch ( WhatPlayerKnowsAboutEnemiesInSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY ) )
	{
		case KNOWS_NOTHING:
			// show "Unknown"
			wcscpy(wString, pwMiscSectorStrings[ 3 ] );
			break;

		case KNOWS_THEYRE_THERE:
			// if there are any there
			if ( ubNumEnemies > 0 )
			{
				// show "?", but not exactly how many
				wcscpy(wString, L"?" );
			}
			else
			{
				// we know there aren't any (or we'd be seing them on map, too)
				wcscpy(wString, L"0" );
			}
			break;

		case KNOWS_HOW_MANY:
			// show exactly how many
			swprintf( wString, lengthof(wString), L"%d", ubNumEnemies );
			break;
	}

	AddSecondColumnMonoString(box, wString);
}


static void AddItemsInSectorToBox(PopUpBox* const box)
{
	wchar_t wString[ 64 ];

	// items in sector (this works even for underground)

	swprintf( wString, lengthof(wString), L"%ls:", pwMiscSectorStrings[ 2 ] );
	AddMonoString(box, wString);

	swprintf( wString, lengthof(wString), L"%d", GetNumberOfVisibleWorldItemsFromSectorStructureForSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY, bCurrentTownMineSectorZ ));
	AddSecondColumnMonoString(box, wString);
}


// position town/mine info box on the screen
static void PositionTownMineInfoBox(PopUpBox* const box)
{
	// position the box based on x and y of the selected sector
	INT16 sX = 0;
	INT16 sY = 0;
	GetScreenXYFromMapXY(bCurrentTownMineSectorX, bCurrentTownMineSectorY, &sX, &sY);
	SGPBox const& area = GetBoxArea(box);

	// now position box - the x axis
	INT16 x = sX;
	if (x          < MapScreenRect.iLeft)  x = MapScreenRect.iLeft + 5;
	if (x + area.w > MapScreenRect.iRight) x = MapScreenRect.iRight - area.w - 5;

	// position - the y axis
	INT16 y = sY;
	if (y          < MapScreenRect.iTop)    y = MapScreenRect.iTop + 5;
	if (y + area.h > MapScreenRect.iBottom) y = MapScreenRect.iBottom - area.h - 8;

	SetBoxXY(box, x, y);
}


static void MakeButton(UINT idx, const wchar_t* text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(INTERFACEDIR "/mapinvbtns.sti", idx, idx + 2);
	guiMapButtonInventoryImage[idx] = img;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, BLOCKFONT2, FONT_WHITE, FONT_BLACK, FONT_WHITE, FONT_BLACK, x, y, MSYS_PRIORITY_HIGHEST - 1, click);
	guiMapButtonInventory[idx] = btn;
}


static void AddInventoryButtonForMapPopUpBox(const PopUpBox* const box)
{
	// load the button
	AutoSGPVObject uiObject(AddVideoObjectFromFile(INTERFACEDIR "/mapinvbtns.sti"));

	// Calculate smily face positions...
	ETRLEObject const& pTrav   = uiObject->SubregionProperties(0);
	INT16       const  sWidthA = pTrav.usWidth;

	SGPBox const& area = GetBoxArea(box);
	INT16  const  dx   = (area.w - sTotalButtonWidth) / 3;
	INT16         x    = area.x + dx;
	INT16  const  y    = area.y + area.h - (BOX_BUTTON_HEIGHT + 5);
	MakeButton(0, pMapPopUpInventoryText[0], x, y, MapTownMineInventoryButtonCallBack);

	x += sWidthA + dx;
	MakeButton(1, pMapPopUpInventoryText[1], x, y, MapTownMineExitButtonCallBack);

/*
	// if below ground disable
	if( iCurrentMapSectorZ )
	{
		DisableButton( guiMapButtonInventory[ 0 ] );
	}
*/
}


static void RemoveInventoryButtonForMapPopUpBox(void)
{

	// get rid of button
	RemoveButton( guiMapButtonInventory[0] );
	UnloadButtonImage( guiMapButtonInventoryImage[0] );

	RemoveButton( guiMapButtonInventory[1] );
	UnloadButtonImage( guiMapButtonInventoryImage[1] );
}


static void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fShowMapInventoryPool = TRUE;
		fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		fShowTownInfo = FALSE;

		//since we are bring up the sector inventory, check to see if the help screen should come up
		if (ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY, FALSE))
		{
			//normally this is handled in the screen handler, we have to set up a little different this time around
			ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY, TRUE);
		}
	}
}


static void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		fShowTownInfo = FALSE;
	}
}


// get the min width of the town mine info pop up box
static void MinWidthOfTownMineInfoBox(void)
{
	AutoSGPVObject uiObject(AddVideoObjectFromFile(INTERFACEDIR "/mapinvbtns.sti"));

	// Calculate smily face positions...
	INT16 const sWidthA = uiObject->SubregionProperties(0).usWidth;
	INT16 const sWidthB = uiObject->SubregionProperties(1).usWidth;
	sTotalButtonWidth = sWidthA + sWidthB;
}
