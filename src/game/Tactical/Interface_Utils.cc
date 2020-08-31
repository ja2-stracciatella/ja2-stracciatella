#include "Interface_Utils.h"
#include "ContentManager.h"
#include "Directories.h"
#include "Faces.h"
#include "GameInstance.h"
#include "HImage.h"
#include "JAScreens.h"
#include "Line.h"
#include "MagazineModel.h"
#include "Overhead.h"
#include "Render_Dirty.h"
#include "Soldier_Macros.h"
#include "SysUtil.h"
#include "UILayout.h"
#include "Vehicles.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include <stdexcept>
#include <string_theory/format>
#include <string_theory/string>


#define LIFE_BAR_SHADOW		FROMRGB(108, 12, 12)
#define LIFE_BAR			FROMRGB(200, 0, 0)
#define BANDAGE_BAR_SHADOW		FROMRGB(156, 60, 60)
#define BANDAGE_BAR			FROMRGB(222, 132, 132)
#define BLEEDING_BAR_SHADOW		FROMRGB(128, 128, 60)
#define BLEEDING_BAR			FROMRGB(240,  240, 20)
#define CURR_BREATH_BAR_SHADOW		FROMRGB(8, 12, 118) // the MAX max breatth, always at 100%
#define CURR_BREATH_BAR		FROMRGB(8, 12, 160)
#define CURR_MAX_BREATH		FROMRGB(0, 0, 0) // the current max breath, black
#define CURR_MAX_BREATH_SHADOW		FROMRGB(0, 0, 0)
#define MORALE_BAR_SHADOW		FROMRGB(8, 112, 12)
#define MORALE_BAR			FROMRGB(8, 180, 12)
#define BREATH_BAR_SHADOW		FROMRGB(60, 108, 108) // the lt blue current breath
#define BREATH_BAR			FROMRGB(113, 178, 218)
#define BREATH_BAR_SHAD_BACK		FROMRGB(1, 1, 1)
#define FACE_WIDTH			48
#define FACE_HEIGHT			43


// car portraits
enum
{
	ELDORADO_PORTRAIT = 0,
	HUMMER_PORTRAIT,
	ICE_CREAM_TRUCK_PORTRAIT,
	JEEP_PORTRAIT,
	NUMBER_CAR_PORTRAITS
};

// the ids for the car portraits
static SGPVObject* giCarPortraits[NUMBER_CAR_PORTRAITS];

static SGPVObject* guiBrownBackgroundForTeamPanel; // backgrounds for breath max background


// Load in the portraits for the car faces that will be use in mapscreen
void LoadCarPortraitValues(void)
{
	// the car portrait file names
	static char const* const pbCarPortraitFileNames[] =
	{
		INTERFACEDIR "/eldorado.sti",
		INTERFACEDIR "/hummer.sti",
		INTERFACEDIR "/ice cream truck.sti",
		INTERFACEDIR "/jeep.sti"
	};

	if (giCarPortraits[0]) return;
	for (INT32 i = 0; i != NUMBER_CAR_PORTRAITS; ++i)
	{
		giCarPortraits[i] = AddVideoObjectFromFile(pbCarPortraitFileNames[i]);
	}
}


// get rid of the images we loaded for the mapscreen car portraits
void UnLoadCarPortraits(void)
{
	// car protraits loaded?
	if (!giCarPortraits[0]) return;
	for (INT32 i = 0; i != NUMBER_CAR_PORTRAITS; ++i)
	{
		DeleteVideoObject(giCarPortraits[i]);
		giCarPortraits[i] = 0;
	}
}


static void DrawBar(UINT32 const XPos, UINT32 const YPos, UINT32 const Height, UINT16 const Color, UINT16 const ShadowColor, UINT16* const DestBuf)
{
	LineDraw(TRUE, XPos + 0, YPos, XPos + 0, YPos - Height, ShadowColor, DestBuf);
	LineDraw(TRUE, XPos + 1, YPos, XPos + 1, YPos - Height, Color,       DestBuf);
	LineDraw(TRUE, XPos + 2, YPos, XPos + 2, YPos - Height, ShadowColor, DestBuf);
}


static void DrawLifeUIBar(SOLDIERTYPE const& s, UINT32 const XPos, UINT32 YPos, UINT32 const MaxHeight, UINT16* const pDestBuf)
{
	UINT32 Height;

	// FIRST DO MAX LIFE
	Height = MaxHeight * s.bLife / 100;
	DrawBar(XPos, YPos, Height, Get16BPPColor(LIFE_BAR), Get16BPPColor(LIFE_BAR_SHADOW), pDestBuf);

	// NOW DO BANDAGE
	// Calculate bandage
	UINT32 Bandage = s.bLifeMax - s.bLife - s.bBleeding;
	if (Bandage != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * Bandage / 100;
		DrawBar(XPos, YPos, Height, Get16BPPColor(BANDAGE_BAR), Get16BPPColor(BANDAGE_BAR_SHADOW), pDestBuf);
	}

	// NOW DO BLEEDING
	if (s.bBleeding != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * s.bBleeding / 100;
		DrawBar(XPos, YPos, Height, Get16BPPColor(BLEEDING_BAR), Get16BPPColor(BLEEDING_BAR_SHADOW), pDestBuf);
	}
}


static void DrawBreathUIBar(SOLDIERTYPE const& s, UINT32 const XPos, UINT32 const sYPos, UINT32 const MaxHeight, UINT16* const pDestBuf)
{
	UINT32 Height;

	if (s.bBreathMax <= 97)
	{
		Height = MaxHeight * (s.bBreathMax + 3) / 100;
		// the old background colors for breath max diff
		DrawBar(XPos, sYPos, Height, Get16BPPColor(BREATH_BAR_SHAD_BACK), Get16BPPColor(BREATH_BAR_SHAD_BACK), pDestBuf);
	}

	Height = MaxHeight * s.bBreathMax / 100;
	DrawBar(XPos, sYPos, Height, Get16BPPColor(CURR_MAX_BREATH), Get16BPPColor(CURR_MAX_BREATH_SHADOW), pDestBuf);

	// NOW DO BREATH
	Height = MaxHeight * s.bBreath / 100;
	DrawBar(XPos, sYPos, Height, Get16BPPColor(CURR_BREATH_BAR), Get16BPPColor(CURR_BREATH_BAR_SHADOW), pDestBuf);
}


static void DrawMoraleUIBar(SOLDIERTYPE const& s, UINT32 const XPos, UINT32 const YPos, UINT32 const MaxHeight, UINT16* const pDestBuf)
{
	UINT32 const Height = MaxHeight * s.bMorale / 100;
	DrawBar(XPos, YPos, Height, Get16BPPColor(MORALE_BAR), Get16BPPColor(MORALE_BAR_SHADOW), pDestBuf);
}


void DrawSoldierUIBars(SOLDIERTYPE const& s, INT16 const sXPos, INT16 const sYPos, BOOLEAN const fErase, SGPVSurface* const uiBuffer)
{
	const UINT32 BarWidth  =  3;
	const UINT32 BarHeight = 42;
	const UINT32 BreathOff =  6;
	const UINT32 MoraleOff = 12;

	// Erase what was there
	if (fErase)
	{
		RestoreExternBackgroundRect(sXPos, sYPos - BarHeight, MoraleOff + BarWidth, BarHeight + 1);
	}

	if (s.bLife == 0) return;

	if (!(s.uiStatusFlags & SOLDIER_ROBOT))
	{
		// DO MAX BREATH
		// brown guy
		UINT16 Region;
		if (guiCurrentScreen != MAP_SCREEN &&
			GetSelectedMan() == &s &&
			gTacticalStatus.ubCurrentTeam == OUR_TEAM &&
			OK_INTERRUPT_MERC(&s))
		{
			Region = 1; // gold, the second entry in the .sti
		}
		else
		{
			Region = 0; // brown, first entry
		}
		BltVideoObject(uiBuffer, guiBrownBackgroundForTeamPanel, Region, sXPos + BreathOff, sYPos - BarHeight);
	}

	SGPVSurface::Lock l(uiBuffer);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	UINT16* const pDestBuf = l.Buffer<UINT16>();

	DrawLifeUIBar(s, sXPos, sYPos, BarHeight, pDestBuf);
	if (!(s.uiStatusFlags & SOLDIER_ROBOT))
	{
		DrawBreathUIBar(s, sXPos + BreathOff, sYPos, BarHeight, pDestBuf);
		if (!(s.uiStatusFlags & SOLDIER_VEHICLE))
		{
			DrawMoraleUIBar(s, sXPos + MoraleOff, sYPos, BarHeight, pDestBuf);
		}
	}
}


void DrawItemUIBarEx(OBJECTTYPE const& o, const UINT8 ubStatus, const INT16 x, const INT16 y, INT16 max_h, const INT16 sColor1, const INT16 sColor2, SGPVSurface* const uiBuffer)
{
	INT16 value;
	// Adjust for ammo, other things
	const ItemModel * item = GCM->getItem(o.usItem);
	if (ubStatus >= DRAW_ITEM_STATUS_ATTACHMENT1)
	{
		value = o.bAttachStatus[ubStatus - DRAW_ITEM_STATUS_ATTACHMENT1];
	}
	else if (item->isKey())
	{
		value = 100;
	}
	else
	{
		if (ubStatus >= MAX_OBJECTS_PER_SLOT) 
			throw std::runtime_error(ST::format("invalid ubStatus value: {}", ubStatus).to_std_string());
		
		if (item->isAmmo())
		{
			value = 100 * o.ubShotsLeft[ubStatus] / (item->asAmmo()->capacity ? item->asAmmo()->capacity : 1);
			if (value > 100) value = 100;
		}
		else
		{
			value = o.bStatus[ubStatus];
		}
	}

	{ SGPVSurface::Lock l(uiBuffer);
		SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		UINT16* const pDestBuf = l.Buffer<UINT16>();

		--max_h; // LineDraw() includes the end point
		const INT h = max_h * value / 100;
		LineDraw(TRUE, x,     y, x,     y - h, sColor1, pDestBuf);
		LineDraw(TRUE, x + 1, y, x + 1, y - h, sColor2, pDestBuf);
	}

	if (uiBuffer == guiSAVEBUFFER)
	{
		RestoreExternBackgroundRect(x, y - max_h, 2, max_h + 1);
	}
	else
	{
		InvalidateRegion(x, y - max_h, x + 2, y + 1);
	}
}


void RenderSoldierFace(SOLDIERTYPE const& s, INT16 const sFaceX, INT16 const sFaceY)
{
	if (s.uiStatusFlags & SOLDIER_VEHICLE)
	{
		// just draw the vehicle
		const UINT8 vehicle_type = pVehicleList[s.bVehicleID].ubVehicleType;
		BltVideoObject(guiSAVEBUFFER, giCarPortraits[vehicle_type], 0, sFaceX, sFaceY);
		RestoreExternBackgroundRect(sFaceX, sFaceY, FACE_WIDTH, FACE_HEIGHT);
	}
	else if (s.face->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) // OK, check if this face actually went active
	{
		ExternRenderFace(guiSAVEBUFFER, *s.face, sFaceX, sFaceY);
	}
	else
	{
		SetAutoFaceActive(FRAME_BUFFER, guiSAVEBUFFER, *s.face, sFaceX, sFaceY);
		RenderAutoFace(*s.face);
	}
}


void LoadInterfaceUtilsGraphics()
{
	guiBrownBackgroundForTeamPanel = AddVideoObjectFromFile(INTERFACEDIR "/bars.sti");
}


void DeleteInterfaceUtilsGraphics()
{
	DeleteVideoObject(guiBrownBackgroundForTeamPanel);
}
