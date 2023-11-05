#include "Directories.h"
#include "HImage.h"
#include "MapScreen.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "SAM_Sites.h"
#include "StrategicMap.h"
#include "StrategicMap_Secrets.h"
#include "PopUpBox.h"
#include "Map_Screen_Interface.h"
#include "Queen_Command.h"
#include "Player_Command.h"
#include "Font_Control.h"
#include "Text.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface_Border.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Mines.h"
#include "Finances.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Town_Militia.h"
#include "HelpScreen.h"
#include "Tactical_Save.h"
#include "Button_System.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "VObject.h"
#include "VSurface.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "UILayout.h"

#include <string_theory/format>
#include <string_theory/string>


#define BOX_BUTTON_HEIGHT (g_ui.m_stdScreenScale * 20)


// flag to say if we are showing town/mine box at all
BOOLEAN fShowTownInfo = FALSE;

PopUpBox* ghTownMineBox;
SGPPoint TownMinePosition ={ 300, 150 };

SGPSector bCurrentTownMineSector;

// inventory button
static BUTTON_PICS* guiMapButtonInventoryImage[2];
static GUIButtonRef guiMapButtonInventory[2];

static UINT16 sTotalButtonWidth = 0;


// callback to turn on sector invneotry list
static void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, UINT32 reason);
static void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, UINT32 reason);


void DisplayTownInfo(const SGPSector& sMap)
{
	// will display town info for a particular town

	// set current sector
	if (bCurrentTownMineSector != sMap)
	{
		bCurrentTownMineSector = sMap;
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
		box = CreatePopUpBox(TownMinePosition, 0, FRAME_BUFFER, guiPOPUPBORDERS, guiPOPUPTEX,
			g_ui.m_stdScreenScale * 6, g_ui.m_stdScreenScale * 6, g_ui.m_stdScreenScale * 8 + g_ui.m_stdScreenScale * BOX_BUTTON_HEIGHT, g_ui.m_stdScreenScale * 6,
			g_ui.m_stdScreenScale * 2);
		ghTownMineBox = box;

		// decide what kind of text to add to display
		if (bCurrentTownMineSector.z == 0)
		{
			UINT8 const sector = bCurrentTownMineSector.AsByte();
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
		SetBoxSecondColumnBackground(   box, FONT_MCOLOR_TRANSPARENT);
		SetBoxSecondColumnHighLight(    box, FONT_WHITE);
		SetBoxSecondColumnShade(        box, FONT_BLACK);
		SetBoxSecondColumnMinimumOffset(box, 20);
		SetBoxForeground(               box, FONT_YELLOW);
		SetBoxBackground(               box, FONT_MCOLOR_TRANSPARENT);
		SetBoxShade(                    box, FONT_BLACK);

		// give title line (0) different color from the rest
		SetBoxLineForeground(box, 0, FONT_LTGREEN);

		MinWidthOfTownMineInfoBox();
		SpecifyBoxMinWidth(box, sTotalButtonWidth + g_ui.m_stdScreenScale * 30);
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
	ST::string wString;
	INT16 sMineSector = 0;

	UINT8 const sector   = bCurrentTownMineSector.AsByte();
	UINT8 const ubTownId = GetTownIdForSector(sector);
	Assert((ubTownId >= FIRST_TOWN) && (ubTownId < NUM_TOWNS));

	ST::string title = GetSectorLandTypeString(sector, 0, true);

	AddMonoString(box, title);
	// blank line
	AddMonoString(box, {});

	AddSectorToBox(box);

	// town size
	wString = ST::format("{}:", pwTownInfoStrings[ 0 ]);
	AddMonoString(box, wString);
	wString = ST::format("{}",  GetTownSectorSize( ubTownId ));
	AddSecondColumnMonoString(box, wString);

	// main facilities
	wString = ST::format("{}:", pwTownInfoStrings[4]);
	AddMonoString(box, wString);
	wString = GetSectorFacilitiesFlags(bCurrentTownMineSector);
	AddSecondColumnMonoString(box, wString);

	// the concept of control is only meaningful in sectors where militia can be trained
	if (MilitiaTrainingAllowedInSector(bCurrentTownMineSector))
	{
		// town control
		wString = ST::format("{}:", pwTownInfoStrings[1]);
		AddMonoString(box, wString);
		wString = ST::format("{}%", GetTownSectorsUnderControl(ubTownId) * 100 / GetTownSectorSize(ubTownId));
		AddSecondColumnMonoString(box, wString);
	}

	// the concept of town loyalty is only meaningful in towns where loyalty is tracked
	if( gTownLoyalty[ ubTownId ].fStarted && gfTownUsesLoyalty[ ubTownId ])
	{
		// town loyalty
		wString = ST::format("{}:", pwTownInfoStrings[3]);
		AddMonoString(box, wString);
		wString = ST::format("{}%", gTownLoyalty[ubTownId].ubRating);
		AddSecondColumnMonoString(box, wString);
	}

	// if the town has a mine
	sMineSector = GetMineSectorForTown( ubTownId );
	if( sMineSector != -1 )
	{
		// Associated Mine: Sector
		wString = ST::format("{}:",  pwTownInfoStrings[2]);
		AddMonoString(box, wString);
		const SGPSector& sMap = SGPSector::FromStrategicIndex(sMineSector);
		wString = sMap.AsShortString();
		AddSecondColumnMonoString(box, wString);
	}
}


// adds text to mine info box
static void AddTextToMineBox(PopUpBox* const box, INT8 const mine)
{
	UINT8            const  town   = GetTownAssociatedWithMine(mine);
	MINE_STATUS_TYPE const& status = gMineStatus[mine];
	ST::string buf;

	// Name of town followed by "mine"
	buf = ST::format("{} {}", GCM->getTownName(town), pwMineStrings[0]);
	AddMonoString(box, buf);

	AddMonoString(box, {}); // Blank line

	AddSectorToBox(box);

	// Mine status
	buf = ST::format("{}:", pwMineStrings[9]);
	AddMonoString(box, buf);
	ST::string const& status_txt =
		status.fEmpty      ? pwMineStrings[5] : // Abandonded
		status.fShutDown   ? pwMineStrings[6] : // Shut down
		status.fRunningOut ? pwMineStrings[7] : // Running out
		pwMineStrings[8];                       // Producing
	AddSecondColumnMonoString(box, status_txt);

	if (!status.fEmpty)
	{
		// Current production
		buf = ST::format("{}:", pwMineStrings[3]);
		AddMonoString(box, buf);
		UINT32 const predicted_income = PredictDailyIncomeFromAMine(mine);
		AddSecondColumnMonoString(box, SPrintMoney(predicted_income));

		// Potential production
		buf = ST::format("{}:", pwMineStrings[4]);
		AddMonoString(box, buf);
		UINT32 const max_removal = GetMaxDailyRemovalFromMine(mine);
		AddSecondColumnMonoString(box, SPrintMoney(max_removal));

		if (GetMaxPeriodicRemovalFromMine(mine) > 0)
		{ // Production rate (current production as a percentage of potential production)
			buf = ST::format("{}:", pwMineStrings[10]);
			AddMonoString(box, buf);
			buf = ST::format("{}%", predicted_income * 100 / max_removal);
			AddSecondColumnMonoString(box, buf);
		}

		// Town control percentage
		buf = ST::format("{}:", pwMineStrings[12]);
		AddMonoString(box, buf);
		buf = ST::format("{}%", GetTownSectorsUnderControl(town) * 100 / GetTownSectorSize(town));
		AddSecondColumnMonoString(box, buf);

		TOWN_LOYALTY const& loyalty = gTownLoyalty[town];
		if (loyalty.fStarted && gfTownUsesLoyalty[town])
		{ // Town loyalty percentage
			buf = ST::format("{}:", pwMineStrings[13]);
			AddMonoString(box, buf);
			buf = ST::format("{}%", loyalty.ubRating);
			AddSecondColumnMonoString(box, buf);
		}

		// Ore type (silver/gold)
		buf = ST::format("{}:", pwMineStrings[11]);
		AddMonoString(box, buf);
		AddSecondColumnMonoString(box, status.ubMineType == SILVER_MINE ? pwMineStrings[1] : pwMineStrings[2]);
	}

#ifdef _DEBUG
	// Dollar amount remaining in mine
	AddMonoString(box, "Remaining (DEBUG):");
	AddSecondColumnMonoString(box, SPrintMoney(GetTotalLeftInMine(mine)));
#endif
}


// add text to non-town/non-mine the other boxes
static void AddTextToBlankSectorBox(PopUpBox* const box)
{
	// get the sector value
	UINT8 const sector = bCurrentTownMineSector.AsByte();

	AddMonoString(box, GetSectorLandTypeString(sector, 0, true));

	// blank line
	AddMonoString(box, {});

	AddSectorToBox(box);
}


// add "sector" line text to any popup box
static void AddSectorToBox(PopUpBox* const box)
{
	ST::string wString;

	// sector
	wString = ST::format("{}:", pwMiscSectorStrings[ 1 ]);
	AddMonoString(box, wString);

	wString = bCurrentTownMineSector.AsShortString();
	if (bCurrentTownMineSector.z != 0)
	{
		wString = bCurrentTownMineSector.AsLongString();
	}

	AddSecondColumnMonoString(box, wString);
}


static void AddCommonInfoToBox(PopUpBox* const box)
{
	ST::string wString;
	UINT8 const sector = bCurrentTownMineSector.AsByte();

	// Add additional information to the box if:
	//   - we can train militia sector
	//   - and it is a town
	//   - or a SAM site the player already knows about
    // Info is shown if the SAM site both unknown and a town sector (Meduna)
	bool const showMoreInfo =
		MilitiaTrainingAllowedInSector(sector) &&
		(GetTownIdForSector(sector) != BLANK_SECTOR ||
		(IsThisSectorASAMSector(sector) && IsSecretFoundAt(sector)));


	// in sector where militia can be trained,
	// control of the sector matters, display who controls this sector.  Map brightness no longer gives this!
	if (showMoreInfo)
	{
		// controlled:
		wString = ST::format("{}:", pwMiscSectorStrings[ 4 ]);
		AddMonoString(box, wString);

		// No/Yes
		AddSecondColumnMonoString(box, pwMiscSectorStrings[StrategicMap[bCurrentTownMineSector.AsStrategicIndex()].fEnemyControlled ? 6 : 5]);

		// militia - is there any?
		wString = ST::format("{}:", pwTownInfoStrings[6]);
		AddMonoString(box, wString);

		UINT8 const ubMilitiaTotal = CountAllMilitiaInSector(bCurrentTownMineSector);
		if (ubMilitiaTotal > 0)
		{
			// some militia, show total & their breakdown by level
			wString = ST::format("{}  ({}/{}/{})", ubMilitiaTotal,
					MilitiaInSectorOfRank(bCurrentTownMineSector, GREEN_MILITIA),
					MilitiaInSectorOfRank(bCurrentTownMineSector, REGULAR_MILITIA),
					MilitiaInSectorOfRank(bCurrentTownMineSector, ELITE_MILITIA));
			AddSecondColumnMonoString(box, wString);
		}
		else
		{
			// no militia: don't bother displaying level breakdown
			AddSecondColumnMonoString(box, "0");
		}


		// percentage of current militia squad training completed
		wString = ST::format("{}:", pwTownInfoStrings[5]);
		AddMonoString(box, wString);
		wString = ST::format("{}%", SectorInfo[sector].ubMilitiaTrainingPercentDone);
		AddSecondColumnMonoString(box, wString);
	}


	// enemy forces
	wString = ST::format("{}:", pwMiscSectorStrings[ 0 ]);
	AddMonoString(box, wString);

	// how many are there, really?
	UINT8 const ubNumEnemies = NumEnemiesInSector(bCurrentTownMineSector);

	switch (WhatPlayerKnowsAboutEnemiesInSector(bCurrentTownMineSector))
	{
		case KNOWS_NOTHING:
			// show "Unknown"
			wString = pwMiscSectorStrings[ 3 ];
			break;

		case KNOWS_THEYRE_THERE:
			// if there are any there
			if ( ubNumEnemies > 0 )
			{
				// show "?", but not exactly how many
				wString = "?";
			}
			else
			{
				// we know there aren't any (or we'd be seing them on map, too)
				wString = "0";
			}
			break;

		case KNOWS_HOW_MANY:
			// show exactly how many
			wString = ST::format("{}", ubNumEnemies);
			break;
	}

	AddSecondColumnMonoString(box, wString);
}


static void AddItemsInSectorToBox(PopUpBox* const box)
{
	ST::string wString;

	// items in sector (this works even for underground)

	wString = ST::format("{}:", pwMiscSectorStrings[ 2 ]);
	AddMonoString(box, wString);

	wString = ST::format("{}", GetNumberOfVisibleWorldItemsFromSectorStructureForSector(bCurrentTownMineSector));
	AddSecondColumnMonoString(box, wString);
}


// position town/mine info box on the screen
static void PositionTownMineInfoBox(PopUpBox* const box)
{
	// position the box based on x and y of the selected sector
	INT16 sX = 0;
	INT16 sY = 0;
	GetScreenXYFromMapXY(bCurrentTownMineSector, &sX, &sY);
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


static void MakeButton(UINT idx, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
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


static void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
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


static void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
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
