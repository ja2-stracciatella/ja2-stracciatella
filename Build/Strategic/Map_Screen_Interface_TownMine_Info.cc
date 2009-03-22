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
static void AddTextToMineBox(PopUpBox*);
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
			// only show the mine info when mines button is selected, otherwise we need to see the sector's regular town info
			if (fShowMineFlag && IsThereAMineInThisSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY))
			{
				AddTextToMineBox(box);
			}
			else
			{
				// do we add text for the town box?
				const INT8 bTownId = GetTownIdForSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
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
		const SGPBox area = *GetBoxArea(box);

		RemoveBox(box);
		ghTownMineBox = NO_POPUP_BOX;

		RemoveInventoryButtonForMapPopUpBox();

		RestoreExternBackgroundRect(area.x, area.y, area.w, area.h + 3);
	}
}


// adds text to town info box
static void AddTextToTownBox(PopUpBox* const box)
{
	CHAR16 wString[ 64 ];
	UINT8 ubTownId = 0;
	UINT16 usTownSectorIndex;
	INT16 sMineSector = 0;

	// remember town id
	ubTownId = GetTownIdForSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY );
	Assert((ubTownId >= FIRST_TOWN) && (ubTownId < NUM_TOWNS));

	usTownSectorIndex = SECTOR( bCurrentTownMineSectorX, bCurrentTownMineSectorY );

	const wchar_t* title;
	switch( usTownSectorIndex )
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
	swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 8 ] );
	AddMonoString(box, wString);
	wcscpy(wString, L"");
	GetSectorFacilitiesFlags( bCurrentTownMineSectorX, bCurrentTownMineSectorY, wString, lengthof(wString));
	AddSecondColumnMonoString(box, wString);

	// the concept of control is only meaningful in sectors where militia can be trained
	if ( MilitiaTrainingAllowedInSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY, 0 ) )
	{
		// town control
		swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 2 ] );
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", GetTownSectorsUnderControl(ubTownId) * 100 / GetTownSectorSize(ubTownId));
		AddSecondColumnMonoString(box, wString);
	}

	// the concept of town loyalty is only meaningful in towns where loyalty is tracked
	if( gTownLoyalty[ ubTownId ].fStarted && gfTownUsesLoyalty[ ubTownId ])
	{
		// town loyalty
		swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 5 ] );
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", gTownLoyalty[ubTownId].ubRating);
		AddSecondColumnMonoString(box, wString);
	}

	// if the town has a mine
	sMineSector = GetMineSectorForTown( ubTownId );
	if( sMineSector != -1 )
	{
		// Associated Mine: Sector
	  swprintf( wString, lengthof(wString), L"%ls:",  pwTownInfoStrings[ 4 ] );
		AddMonoString(box, wString);
	  GetShortSectorString( ( INT16 )( sMineSector % MAP_WORLD_X ), ( INT16 )( sMineSector / MAP_WORLD_X ), wString, lengthof(wString));
		AddSecondColumnMonoString(box, wString);
	}
}


// adds text to mine info box
static void AddTextToMineBox(PopUpBox* const box)
{
	UINT8 ubMineIndex;
	UINT8 ubTown;
	CHAR16 wString[ 64 ];

	ubMineIndex = GetMineIndexForSector( bCurrentTownMineSectorX, bCurrentTownMineSectorY );

	// name of town followed by "mine"
	swprintf( wString, lengthof(wString), L"%ls %ls", pTownNames[ GetTownAssociatedWithMine( ubMineIndex ) ], pwMineStrings[ 0 ] );
	AddMonoString(box, wString);

	// blank line
	AddMonoString(box, L"");

	AddSectorToBox(box);

	// mine status
	swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 9 ]);
	AddMonoString(box, wString);

	// check if mine is empty (abandoned) or running out
	const wchar_t* MineStatus;
	if (gMineStatus[ ubMineIndex ].fEmpty)
	{
		// abandonded
		MineStatus = pwMineStrings[5];
	}
	else
	if (gMineStatus[ ubMineIndex ].fShutDown)
	{
		// shut down
		MineStatus = pwMineStrings[6];
	}
	else
	if (gMineStatus[ ubMineIndex ].fRunningOut)
	{
		// running out
		MineStatus = pwMineStrings[7];
	}
	else
	{
		// producing
		MineStatus = pwMineStrings[8];
	}
	AddSecondColumnMonoString(box, MineStatus);


	// if still producing
	if (!gMineStatus[ ubMineIndex ].fEmpty)
	{
		// current production
		swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 3 ]);
		AddMonoString(box, wString);

		SPrintMoney(wString, PredictDailyIncomeFromAMine(ubMineIndex));
		AddSecondColumnMonoString(box, wString);


		// potential production
		swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 4 ]);
		AddMonoString(box, wString);

		SPrintMoney(wString, GetMaxDailyRemovalFromMine(ubMineIndex));
		AddSecondColumnMonoString(box, wString);


		// if potential is not nil
		if (GetMaxPeriodicRemovalFromMine(ubMineIndex) > 0)
		{
			// production rate (current production as a percentage of potential production)
			swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 10 ]);
			AddMonoString(box, wString);
			swprintf(wString, lengthof(wString), L"%d%%", PredictDailyIncomeFromAMine(ubMineIndex) * 100 / GetMaxDailyRemovalFromMine(ubMineIndex));
			AddSecondColumnMonoString(box, wString);
		}


		// town control percentage
		swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 12 ]);
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", GetTownSectorsUnderControl(gMineLocation[ubMineIndex].bAssociatedTown) * 100 / GetTownSectorSize(gMineLocation[ubMineIndex].bAssociatedTown));
		AddSecondColumnMonoString(box, wString);

		ubTown = gMineLocation[ ubMineIndex ].bAssociatedTown;
		if( gTownLoyalty[ ubTown ].fStarted && gfTownUsesLoyalty[ ubTown ])
		{
			// town loyalty percentage
			swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 13 ]);
			AddMonoString(box, wString);
			swprintf(wString, lengthof(wString), L"%d%%", gTownLoyalty[gMineLocation[ubMineIndex].bAssociatedTown].ubRating);
			AddSecondColumnMonoString(box, wString);
		}

/* gradual monster infestation concept was ditched, now simply IN PRODUCTION or SHUT DOWN
		// percentage of miners working
		swprintf(wString, lengthof(wString), L"%ls:", pwMineStrings[14]);
		AddMonoString(box, wString);
		swprintf(wString, lengthof(wString), L"%d%%", gubMonsterMineInfestation[gMineStatus[ubMineIndex].bMonsters]);
		AddSecondColumnMonoString(box, wString);
*/

		// ore type (silver/gold
		swprintf( wString, lengthof(wString), L"%ls:", pwMineStrings[ 11 ]);
		AddMonoString(box, wString);
		AddSecondColumnMonoString(box, gMineStatus[ubMineIndex].ubMineType == SILVER_MINE ? pwMineStrings[1] : pwMineStrings[2]);
	}


#ifdef _DEBUG
	// dollar amount remaining in mine
	AddMonoString(box, L"Remaining (DEBUG):");

	SPrintMoney(wString, GetTotalLeftInMine(ubMineIndex));
	AddSecondColumnMonoString(box, wString);
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

		default: title = pLandTypeStrings[SectorInfo[usSectorValue].ubTraversability[4]]; break;
	}
	AddMonoString(box, title);

	// blank line
	AddMonoString(box, L"");

	AddSectorToBox(box);
}


// add "sector" line text to any popup box
static void AddSectorToBox(PopUpBox* const box)
{
	CHAR16 wString[ 64 ];
	CHAR16 wString2[ 10 ];

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
	CHAR16 wString[ 64 ];
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
		swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 11 ] );
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
		swprintf( wString, lengthof(wString), L"%ls:", pwTownInfoStrings[ 10 ] );
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
	CHAR16 wString[ 64 ];

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
	const SGPBox* const area = GetBoxArea(box);

	// now position box - the x axis
	INT16 x = sX;
	if (x           < MapScreenRect.iLeft)  x = MapScreenRect.iLeft + 5;
	if (x + area->w > MapScreenRect.iRight) x = MapScreenRect.iRight - area->w - 5;

	// position - the y axis
	INT16 y = sY;
	if (y           < MapScreenRect.iTop)    y = MapScreenRect.iTop + 5;
	if (y + area->h > MapScreenRect.iBottom) y = MapScreenRect.iBottom - area->h - 8;

	SetBoxXY(box, x, y);
}


static void MakeButton(UINT idx, const wchar_t* text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage("INTERFACE/mapinvbtns.sti", -1, idx, -1, idx + 2, -1);
	guiMapButtonInventoryImage[idx] = img;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, BLOCKFONT2, FONT_WHITE, FONT_BLACK, FONT_WHITE, FONT_BLACK, x, y, MSYS_PRIORITY_HIGHEST - 1, click);
	guiMapButtonInventory[idx] = btn;
}


static void AddInventoryButtonForMapPopUpBox(const PopUpBox* const box)
{
	// load the button
	AutoSGPVObject uiObject(AddVideoObjectFromFile("INTERFACE/mapinvbtns.sti"));

	// Calculate smily face positions...
	ETRLEObject const& pTrav   = uiObject->SubregionProperties(0);
	INT16       const  sWidthA = pTrav.usWidth;

	const SGPBox* const area = GetBoxArea(box);
	INT16         const dx   = (area->w - sTotalButtonWidth) / 3;
	INT16               x    = area->x + dx;
	INT16         const y    = area->y + area->h - (BOX_BUTTON_HEIGHT + 5);
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
	AutoSGPVObject uiObject(AddVideoObjectFromFile("INTERFACE/mapinvbtns.sti"));

	// Calculate smily face positions...
	INT16 const sWidthA = uiObject->SubregionProperties(0).usWidth;
	INT16 const sWidthB = uiObject->SubregionProperties(1).usWidth;
	sTotalButtonWidth = sWidthA + sWidthB;
}
