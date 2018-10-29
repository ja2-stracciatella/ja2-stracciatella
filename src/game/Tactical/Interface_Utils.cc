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


#define LIFE_BAR_SHADOW		RGB(108, 12, 12)
#define LIFE_BAR			RGB(200, 0, 0)
#define BANDAGE_BAR_SHADOW		RGB(156, 60, 60)
#define BANDAGE_BAR			RGB(222, 132, 132)
#define BLEEDING_BAR_SHADOW		RGB(128, 128, 60)
#define BLEEDING_BAR			RGB(240,  240, 20)
#define CURR_BREATH_BAR_SHADOW		RGB(8, 12, 118) // the MAX max breatth, always at 100%
#define CURR_BREATH_BAR		RGB(8, 12, 160)
#define CURR_MAX_BREATH		RGB(0, 0, 0) // the current max breath, black
#define CURR_MAX_BREATH_SHADOW		RGB(0, 0, 0)
#define MORALE_BAR_SHADOW		RGB(8, 112, 12)
#define MORALE_BAR			RGB(8, 180, 12)
#define BREATH_BAR_SHADOW		RGB(60, 108, 108) // the lt blue current breath
#define BREATH_BAR			RGB(113, 178, 218)
#define BREATH_BAR_SHAD_BACK		RGB(1, 1, 1)
#define FACE_WIDTH			(g_ui.m_stdScreenScale * 48)
#define FACE_HEIGHT			(g_ui.m_stdScreenScale * 43)


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


static void DrawBar(const UINT32 XPos, const UINT32 YPos, const UINT32 Width, const UINT32 Height, const UINT32 Color, const UINT32 ShadowColor, UINT32 *DestBuf)
{
	LineDraw(TRUE, XPos + 0, YPos, XPos + 0, YPos - Height, ShadowColor, DestBuf);
	for(int x = 2; x < Width; x++)
		LineDraw(TRUE, XPos + x - 1, YPos, XPos + x - 1, YPos - Height, Color, DestBuf);
	LineDraw(TRUE, XPos + Width - 1, YPos, XPos + Width - 1, YPos - Height, ShadowColor, DestBuf);
}


static void DrawLifeUIBar(const SOLDIERTYPE &s, const UINT32 XPos, UINT32 YPos, const UINT32 Width, const UINT32 MaxHeight, UINT32 *pDestBuf)
{
	UINT32 Height;

	// FIRST DO MAX LIFE
	Height = MaxHeight * s.bLife / 100;
	DrawBar(XPos, YPos, Width, Height, LIFE_BAR, LIFE_BAR_SHADOW, pDestBuf);

	// NOW DO BANDAGE
	// Calculate bandage
	UINT32 Bandage = s.bLifeMax - s.bLife - s.bBleeding;
	if (Bandage != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * Bandage / 100;
		DrawBar(XPos, YPos, Width, Height, BANDAGE_BAR, BANDAGE_BAR_SHADOW, pDestBuf);
	}

	// NOW DO BLEEDING
	if (s.bBleeding != 0)
	{
		YPos   -= Height;
		Height  = MaxHeight * s.bBleeding / 100;
		DrawBar(XPos, YPos, Width, Height, BLEEDING_BAR, BLEEDING_BAR_SHADOW, pDestBuf);
	}
}


static void DrawBreathUIBar(const SOLDIERTYPE &s, const UINT32 XPos, UINT32 YPos, const UINT32 Width, const UINT32 MaxHeight, UINT32 *pDestBuf)
{
	UINT32 Height;

	if (s.bBreathMax <= 97)
	{
		Height = MaxHeight * (s.bBreathMax + 3) / 100;
		// the old background colors for breath max diff
		DrawBar(XPos, YPos, Width, Height, BREATH_BAR_SHAD_BACK, BREATH_BAR_SHAD_BACK, pDestBuf);
	}

	Height = MaxHeight * s.bBreathMax / 100;
	DrawBar(XPos, YPos, Width, Height, CURR_MAX_BREATH, CURR_MAX_BREATH_SHADOW, pDestBuf);

	// NOW DO BREATH
	Height = MaxHeight * s.bBreath / 100;
	DrawBar(XPos, YPos, Width, Height, CURR_BREATH_BAR, CURR_BREATH_BAR_SHADOW, pDestBuf);
}


static void DrawMoraleUIBar(const SOLDIERTYPE &s, const UINT32 XPos, UINT32 YPos, const UINT32 Width, const UINT32 MaxHeight, UINT32 *pDestBuf)
{
	UINT32 const Height = MaxHeight * s.bMorale / 100;
	DrawBar(XPos, YPos, Width, Height, MORALE_BAR, MORALE_BAR_SHADOW, pDestBuf);
}


void DrawSoldierUIBars(const SOLDIERTYPE &s, const INT16 sXPos, const INT16 sYPos, const BOOLEAN fErase, SGPVSurface *uiBuffer)
{
	const UINT32 BarWidth  = g_ui.m_stdScreenScale * 3;
	const UINT32 BarHeight = g_ui.m_stdScreenScale * 42;
	const UINT32 BreathOff = g_ui.m_stdScreenScale * 6;
	const UINT32 MoraleOff = g_ui.m_stdScreenScale * 12;

	// Erase what was there
	if (fErase)
		RestoreExternBackgroundRect(sXPos, sYPos - BarHeight, MoraleOff + BarWidth, BarHeight + 1);

	if (s.bLife == 0) return;

	if (!(s.uiStatusFlags & SOLDIER_ROBOT))
	{
		// DO MAX BREATH
		// brown guy
		UINT16 Region;
		if (guiCurrentScreen != MAP_SCREEN && GetSelectedMan() == &s && gTacticalStatus.ubCurrentTeam == OUR_TEAM && OK_INTERRUPT_MERC(&s))
			Region = 1; // gold, the second entry in the .sti
		else
			Region = 0; // brown, first entry
		BltVideoObject(uiBuffer, guiBrownBackgroundForTeamPanel, Region, sXPos + BreathOff, sYPos - BarHeight);
	}

	SGPVSurface::Lock l(uiBuffer);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	UINT32* const pDestBuf = l.Buffer<UINT32>();

	DrawLifeUIBar(s, sXPos, sYPos, BarWidth, BarHeight, pDestBuf);
	if (!(s.uiStatusFlags & SOLDIER_ROBOT))
	{
		DrawBreathUIBar(s, sXPos + BreathOff, sYPos, BarWidth, BarHeight, pDestBuf);
		if (!(s.uiStatusFlags & SOLDIER_VEHICLE))
			DrawMoraleUIBar(s, sXPos + MoraleOff, sYPos, BarWidth, BarHeight, pDestBuf);
	}
}

void DrawItemUIBarEx(const OBJECTTYPE &o, const UINT8 ubStatus, const INT16 x, const INT16 y, const INT16 w, INT16 max_h, const UINT32 sColor1, const UINT32 sColor2, SGPVSurface *uiBuffer)
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
		UINT32* const pDestBuf = l.Buffer<UINT32>();

		--max_h; // LineDraw() includes the end point
		const INT h = max_h * value / 100;
		for(int i = 1; i < w; i++)
			LineDraw(TRUE, x + i - 1, y, x + i - 1, y - h, sColor1, pDestBuf);
		LineDraw(TRUE, w - 1, y, w - 1, y - h, sColor2, pDestBuf);
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
