#include <stdexcept>

#include "Font.h"
#include "HImage.h"
#include "Interface_Control.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "WorldDef.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "WCheck.h"
#include "Video.h"
#include "Faces.h"
#include "Overhead.h"
#include "Gap.h"
#include "Soldier_Profile.h"
#include "Sound_Control.h"
#include "TeamTurns.h"
#include "Soldier_Macros.h"
#include "Dialogue_Control.h"
#include "Font_Control.h"
#include "Assignments.h"
#include "Random.h"
#include "Line.h"
#include "GameSettings.h"
#include "Squads.h"
#include "Interface.h"
#include "Quests.h"
#include "Drugs_And_Alcohol.h"
#include "Interface_Items.h"
#include "Meanwhile.h"
#include "Map_Screen_Interface.h"
#include "SoundMan.h"
#include "JAScreens.h"
#include "ScreenIDs.h"


// Defines
#define		NUM_FACE_SLOTS					50


// GLOBAL FOR FACES LISTING
static FACETYPE gFacesData[NUM_FACE_SLOTS];
static UINT32   guiNumFaces = 0;


#define FOR_ALL_FACES(iter)                                    \
	for (FACETYPE*       iter        = gFacesData,               \
	             * const iter##__end = gFacesData + guiNumFaces; \
	     iter != iter##__end;                                    \
	     ++iter)                                                 \
		if (!iter->fAllocated) continue; else


struct RPC_SMALL_FACE_VALUES
{
	ProfileID profile;
	INT8      bEyesX;
	INT8      bEyesY;
	INT8      bMouthX;
	INT8      bMouthY;
};


// TODO0013
static const RPC_SMALL_FACE_VALUES gRPCSmallFaceValues[] =
{
	{ MIGUEL,    9, 8,  8, 24 },
	{ CARLOS,    8, 8,  7, 24 },
	{ IRA,      10, 8,  8, 26 },
	{ DIMITRI,   7, 8,  7, 26 },
	{ DEVIN,     6, 7,  7, 23 },
	{ ROBOT,     0, 0,  0,  0 }, // THE RAT  (  62 )
	{ HAMOUS,    8, 7,  8, 23 },
	{ SLAY,      8, 8,  8, 22 },
	{ RPC65,     0, 0,  0,  0 },
	{ DYNAMO,    9, 4,  7, 22 },
	{ SHANK,     8, 8,  8, 25 },
	{ IGGY,      4, 6,  5, 22 },
	{ VINCE,     8, 9,  7, 25 },
	{ CONRAD,    4, 7,  5, 25 },
	{ RPC71,     9, 7,  8, 22 }, // CARL     (  71 )
	{ MADDOG,    9, 7,  9, 25 },
	{ DARREL,    0, 0,  0,  0 },
	{ PERKO,     0, 0,  0,  0 },

	{ MARIA,     9, 3,  8, 23 },

	{ JOEY,      9, 3,  8, 25 },

	{ SKYRIDER, 11, 7,  9, 24 },
	{ FRED,      9, 5,  7, 23 }, // Miner    ( 106 )

	{ JOHN,      6, 4,  6, 24 },
	{ MARY,     12, 4, 10, 24 },
	{ MATT,      8, 6,  8, 23 }, // Miner    ( 148 )
	{ OSWALD,    6, 5,  6, 23 }, // Miner    ( 156 )
	{ CALVIN,   13, 7, 11, 24 }, // Miner    ( 157 )
	{ CARL,      9, 7,  8, 22 }  // Miner    ( 158 )
};


static FACETYPE* GetFreeFace(void)
{
	for (FACETYPE* i = gFacesData; i != gFacesData + guiNumFaces; ++i)
	{
		if (!i->fAllocated) return i;
	}
	if (guiNumFaces < NUM_FACE_SLOTS) return &gFacesData[guiNumFaces++];
	throw std::runtime_error("Out of face slots");
}


static void RecountFaces(void)
{
INT32 uiCount;

	for(uiCount=guiNumFaces-1; (uiCount >=0) ; uiCount--)
	{
		if( ( gFacesData[uiCount].fAllocated ) )
		{
			guiNumFaces=(UINT32)(uiCount+1);
			break;
		}
	}
}


void InitSoldierFace(SOLDIERTYPE& s)
{
	// Check if we have a face init already
	if (s.face) return;
	s.face = InitFace(s.ubProfile, &s, 0);
}


FACETYPE* InitFace(const ProfileID id, SOLDIERTYPE* const s, const UINT32 uiInitFlags)
{
	if (id == NO_PROFILE) throw std::logic_error("Tried to load face for invalid profile");
	MERCPROFILESTRUCT const& p = GetProfile(id);

	FACETYPE* const face = GetFreeFace();

	const char* face_file;
	// Check if we are a big-face....
	if (uiInitFlags & FACE_BIGFACE)
	{
		face_file = "FACES/b%02d.sti";
    // ATE: Check for profile - if elliot, use special face :)
		if (id == ELLIOT && p.bNPCData > 3)
		{
			if      (p.bNPCData <   7) face_file = "FACES/b%02da.sti";
			else if (p.bNPCData <  10) face_file = "FACES/b%02db.sti";
			else if (p.bNPCData <  13) face_file = "FACES/b%02dc.sti";
			else if (p.bNPCData <  16) face_file = "FACES/b%02dd.sti";
			else if (p.bNPCData == 17) face_file = "FACES/b%02de.sti";
		}
	}
	else
	{
		face_file = "FACES/%02d.sti";
	}

	// HERVE, PETER, ALBERTO and CARLO all use HERVE's portrait
	INT32 const face_id = HERVE <= id && id <= CARLO ? HERVE : p.ubFaceIndex;

	SGPFILENAME ImageFile;
	sprintf(ImageFile, face_file, face_id);
	SGPVObject* vo;
	try
	{
		vo = AddVideoObjectFromFile(ImageFile);
	}
	catch (...)
	{
		// If we are a big face, use placeholder...
		if (!(uiInitFlags & FACE_BIGFACE)) throw;
		vo = AddVideoObjectFromFile("FACES/placeholder.sti");
	}

	memset(face, 0, sizeof(*face));
	face->uiFlags               = uiInitFlags;
	face->fAllocated            = TRUE;
	face->fDisabled             = TRUE; // default to off!
	face->video_overlay         = NULL;
	face->soldier               = s;
	face->ubCharacterNum        = id;
	face->sEyeFrame             = 0;
	face->uiEyeDelay            = 50 + Random(30);

	UINT32 blink_freq = p.uiBlinkFrequency;
	blink_freq = (Random(2) ? blink_freq + Random(2000) : blink_freq - Random(2000));
	face->uiBlinkFrequency      = blink_freq;

	face->uiExpressionFrequency = p.uiExpressionFrequency;
	face->sMouthFrame           = 0;
	face->uiMouthDelay          = 120;
	face->uiVideoObject         = vo;

	// Set palette
	SGPPaletteEntry pal[256];
	// Build a grayscale palette! (for testing different looks)
	for (UINT32 i = 0; i < 256; ++i)
	{
		pal[i].r = 255;
		pal[i].g = 255;
		pal[i].b = 255;
	}

	const SGPPaletteEntry* const vo_pal = vo->Palette();
	vo->pShades[FLASH_PORTRAIT_NOSHADE   ] = Create16BPPPaletteShaded(vo_pal, 255, 255, 255, FALSE);
	vo->pShades[FLASH_PORTRAIT_STARTSHADE] = Create16BPPPaletteShaded(pal,    255, 255, 255, FALSE);
	vo->pShades[FLASH_PORTRAIT_ENDSHADE  ] = Create16BPPPaletteShaded(vo_pal, 250,  25,  25, TRUE );
	vo->pShades[FLASH_PORTRAIT_DARKSHADE ] = Create16BPPPaletteShaded(vo_pal, 100, 100, 100, TRUE );
	vo->pShades[FLASH_PORTRAIT_LITESHADE ] = Create16BPPPaletteShaded(vo_pal, 100, 100, 100, FALSE);

	for (UINT32 i = 0; i < 256; ++i)
	{
		pal[i].r = i % 128 + 128;
		pal[i].g = i % 128 + 128;
		pal[i].b = i % 128 + 128;
	}
	vo->pShades[FLASH_PORTRAIT_GRAYSHADE] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);

	// Get FACE height, width
	ETRLEObject const& face_gfx = vo->SubregionProperties(0);
	face->usFaceWidth  = face_gfx.usWidth;
	face->usFaceHeight = face_gfx.usHeight;

	// OK, check # of items
	if (vo->SubregionCount() == 8)
	{
		// Get EYE height, width
		ETRLEObject const& eyes_gfx = vo->SubregionProperties(1);
		face->usEyesWidth  = eyes_gfx.usWidth;
		face->usEyesHeight = eyes_gfx.usHeight;

		// Get Mouth height, width
		ETRLEObject const& mouth_gfx = vo->SubregionProperties(5);
		face->usMouthWidth  = mouth_gfx.usWidth;
		face->usMouthHeight = mouth_gfx.usHeight;

		face->fInvalidAnim = FALSE;
	}
	else
	{
		face->fInvalidAnim = TRUE;
	}

	return face;
}


void DeleteSoldierFace( SOLDIERTYPE *pSoldier )
{
	DeleteFace(pSoldier->face);
	pSoldier->face = NULL;
}


void DeleteFace(FACETYPE* const pFace)
{
	CHECKV(pFace != NULL);

	// Check for a valid slot!
	CHECKV(pFace->fAllocated);

  pFace->fCanHandleInactiveNow = TRUE;

	SetAutoFaceInActive(pFace);

	// If we are still talking, stop!
	if ( pFace->fTalking )
	{
		// Call dialogue handler function
		pFace->fTalking = FALSE;

		HandleDialogueEnd( pFace );
	}

	// Delete vo
	DeleteVideoObject(pFace->uiVideoObject);

	// Set uncallocated
	pFace->fAllocated = FALSE;

	RecountFaces( );
}


void SetAutoFaceActiveFromSoldier(SGPVSurface* const display, SGPVSurface* const restore, const SOLDIERTYPE* const s, const UINT16 usFaceX, const UINT16 usFaceY)
{
	CHECKV(s != NULL);
	SetAutoFaceActive(display, restore, s->face, usFaceX, usFaceY);
}


static void GetFaceRelativeCoordinates(const FACETYPE* const f, UINT16* const pusEyesX, UINT16* const pusEyesY, UINT16* const pusMouthX, UINT16* const pusMouthY)
{
	ProfileID         const  pid = f->ubCharacterNum;
	MERCPROFILESTRUCT const& p   = GetProfile(pid);

	// Take eyes x,y from profile unless we are an RPC and we are small faced
	// Are we a recruited merc or small?
	if (f->uiFlags & FACE_FORCE_SMALL ||
			(!(f->uiFlags & FACE_BIGFACE) && p.ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE)))
	{
		// Loop through all values of available profiles to find ours
		for (const RPC_SMALL_FACE_VALUES* i = gRPCSmallFaceValues; i != endof(gRPCSmallFaceValues); ++i)
		{
			if (i->profile != pid) continue;
			*pusEyesX  = i->bEyesX;
			*pusEyesY  = i->bEyesY;
			*pusMouthX = i->bMouthX;
			*pusMouthY = i->bMouthY;
			return;
		}
	}

	*pusEyesX  = p.usEyesX;
	*pusEyesY  = p.usEyesY;
	*pusMouthX = p.usMouthX;
	*pusMouthY = p.usMouthY;
}


static void InternalSetAutoFaceActive(SGPVSurface* display, SGPVSurface* restore, FACETYPE*, UINT16 usFaceX, UINT16 usFaceY, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY);


void SetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, FACETYPE* const face, const UINT16 usFaceX, const UINT16 usFaceY)
{
	CHECKV(face != NULL);
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	GetFaceRelativeCoordinates(face, &usEyesX, &usEyesY, &usMouthX, &usMouthY);
	InternalSetAutoFaceActive(display, restore, face, usFaceX, usFaceY, usEyesX, usEyesY, usMouthX, usMouthY);
}


static void InternalSetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, FACETYPE* const pFace, const UINT16 usFaceX, const UINT16 usFaceY, const UINT16 usEyesX, const UINT16 usEyesY, const UINT16 usMouthX, const UINT16 usMouthY)
{
	// IF we are already being contained elsewhere, return without doing anything!

	// ATE: Don't allow another activity from setting active....
	if ( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE )
	{
		return;
	}

	// Check if we are active already, remove if so!
	SetAutoFaceInActive(pFace);

	if (restore == FACE_AUTO_RESTORE_BUFFER)
	{
		pFace->fAutoRestoreBuffer  = TRUE;
		pFace->uiAutoRestoreBuffer = AddVideoSurface(pFace->usFaceWidth, pFace->usFaceHeight, PIXEL_DEPTH);
	}
	else
	{
		pFace->fAutoRestoreBuffer  = FALSE;
		pFace->uiAutoRestoreBuffer = restore;
	}

	if (display == FACE_AUTO_DISPLAY_BUFFER)
	{
		pFace->fAutoDisplayBuffer  = TRUE;
		pFace->uiAutoDisplayBuffer = AddVideoSurface(pFace->usFaceWidth, pFace->usFaceHeight, PIXEL_DEPTH);
	}
	else
	{
		pFace->fAutoDisplayBuffer  = FALSE;
		pFace->uiAutoDisplayBuffer = display;
	}


	pFace->usFaceX				= usFaceX;
	pFace->usFaceY				= usFaceY;
	pFace->fCanHandleInactiveNow = FALSE;


	//Take eyes x,y from profile unless we are an RPC and we are small faced.....
	pFace->usEyesX				= usEyesX + usFaceX;
	pFace->usEyesY				= usEyesY + usFaceY;
	pFace->usMouthY				=	usMouthY + usFaceY;
	pFace->usMouthX				= usMouthX + usFaceX;

	// Save offset values
	pFace->usEyesOffsetX				= usEyesX;
	pFace->usEyesOffsetY				= usEyesY;
	pFace->usMouthOffsetY				=	usMouthY;
	pFace->usMouthOffsetX				= usMouthX;


	if ( pFace->usEyesY == usFaceY || pFace->usMouthY == usFaceY )
	{
		pFace->fInvalidAnim = TRUE;
	}

	pFace->fDisabled			=	FALSE;
	pFace->uiLastBlink			=		GetJA2Clock();
	pFace->uiLastExpression	=		GetJA2Clock();
	pFace->uiEyelast				=		GetJA2Clock();

	// Are we a soldier?
	if (pFace->soldier != NULL)
	{
		pFace->bOldSoldierLife = pFace->soldier->bLife;
	}
}


void SetAutoFaceInActiveFromSoldier(const SOLDIERTYPE* const s)
{
	CHECKV(s != NULL);
	SetAutoFaceInActive(s->face);
}


void SetAutoFaceInActive(FACETYPE* const pFace)
{
	CHECKV(pFace != NULL);

	// Check for a valid slot!
	CHECKV(pFace->fAllocated);

	if (pFace->fDisabled) return;

	// Turn off some flags
	if ( pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE )
	{
		if ( !pFace->fCanHandleInactiveNow )
		{
			return;
		}
	}

	if ( pFace->uiFlags & FACE_MAKEACTIVE_ONCE_DONE )
	{
		const SOLDIERTYPE* const pSoldier = pFace->soldier;
		if (pSoldier != NULL)
		{
			// IF we are in tactical
			if ( pSoldier->bAssignment == iCurrentTacticalSquad && guiCurrentScreen == GAME_SCREEN )
			{
				// Make the interfac panel dirty..
				// This will dirty the panel next frame...
				gfRerenderInterfaceFromHelpText = TRUE;
			}
		}
	}

	if (pFace->fAutoRestoreBuffer)
	{
		DeleteVideoSurface(pFace->uiAutoRestoreBuffer);
		pFace->uiAutoRestoreBuffer = NULL;
	}
	if (pFace->fAutoDisplayBuffer)
	{
		DeleteVideoSurface(pFace->uiAutoDisplayBuffer);
		pFace->uiAutoDisplayBuffer = NULL;
	}

	if (pFace->video_overlay != NULL)
	{
		RemoveVideoOverlay(pFace->video_overlay);
		pFace->video_overlay = NULL;
	}

	// Turn off some flags
	pFace->uiFlags &= ( ~FACE_INACTIVE_HANDLED_ELSEWHERE );

	// Disable!
	pFace->fDisabled = TRUE;

}


void SetAllAutoFacesInactive(  )
{
	FOR_ALL_FACES(i)
	{
		SetAutoFaceInActive(i);
	}
}


static void NewEye(FACETYPE* pFace);
static void HandleRenderFaceAdjustments(FACETYPE* pFace, BOOLEAN fDisplayBuffer, SGPVSurface* buffer, INT16 sFaceX, INT16 sFaceY, UINT16 usEyesX, UINT16 usEyesY);
static void FaceRestoreSavedBackgroundRect(FACETYPE const*, INT16 sDestLeft, INT16 sDestTop, INT16 sSrcLeft, INT16 sSrcTop, INT16 sWidth, INT16 sHeight);


static void BlinkAutoFace(FACETYPE* const f)
{
	Assert(f->fAllocated);
	Assert(!f->fDisabled);

	if (f->fInvalidAnim) return;

	// CHECK IF BUDDY IS DEAD, UNCONSCIOUS, ASLEEP, OR POW!
	const SOLDIERTYPE* const s = f->soldier;
	if (s != NULL &&
			(
				s->fMercAsleep           ||
				s->bLife       <  OKLIFE ||
				s->bAssignment == ASSIGNMENT_POW
			))
	{
		return;
	}

	if (f->ubExpression == NO_EXPRESSION)
	{
		// Get Delay time, if the first frame, use a different delay
		if (GetJA2Clock() - f->uiLastBlink > f->uiBlinkFrequency)
		{
			f->uiLastBlink  = GetJA2Clock();
			f->ubExpression = BLINKING;
			f->uiEyelast    = GetJA2Clock();
		}

		if (f->fAnimatingTalking &&
				GetJA2Clock() - f->uiLastExpression > f->uiExpressionFrequency)
		{
			f->uiLastExpression = GetJA2Clock();
			f->ubExpression     = (Random(2) == 0 ? ANGRY : SURPRISED);
		}
	}

	if (f->ubExpression != NO_EXPRESSION &&
			GetJA2Clock() - f->uiEyelast > f->uiEyeDelay) // Are we going to blink?
	{
		f->uiEyelast = GetJA2Clock();

		NewEye(f);

		INT16 sFrame = f->sEyeFrame;
		if (sFrame > 4) sFrame = 4;

		if (sFrame > 0)
		{
			// Blit Accordingly!
			BltVideoObject(f->uiAutoDisplayBuffer, f->uiVideoObject, sFrame, f->usEyesX, f->usEyesY);

			if (f->uiAutoDisplayBuffer == FRAME_BUFFER)
			{
				InvalidateRegion(f->usEyesX, f->usEyesY, f->usEyesX + f->usEyesWidth, f->usEyesY + f->usEyesHeight);
			}
		}
		else
		{
			f->ubExpression = NO_EXPRESSION;
			// Update rects just for eyes

			if (f->uiAutoRestoreBuffer == guiSAVEBUFFER)
			{
				FaceRestoreSavedBackgroundRect(f, f->usEyesX, f->usEyesY, f->usEyesX, f->usEyesY, f->usEyesWidth, f->usEyesHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(f, f->usEyesX, f->usEyesY, f->usEyesOffsetX, f->usEyesOffsetY, f->usEyesWidth, f->usEyesHeight);
			}
		}

		HandleRenderFaceAdjustments(f, TRUE, 0, f->usFaceX, f->usFaceY, f->usEyesX, f->usEyesY);
	}
}


static void DrawFaceRect(const FACETYPE* const f, SGPVSurface* const buffer, const INT16 x, const INT16 y, const UINT32 colour)
{
	SGPVSurface::Lock l(buffer);
	UINT32 const uiDestPitchBYTES = l.Pitch();

	SetClippingRegionAndImageWidth(uiDestPitchBYTES, x - 2, y - 1, x + f->usFaceWidth + 4, y + f->usFaceHeight + 4);

	const UINT16 usLineColor = Get16BPPColor(colour);
	RectangleDraw(TRUE, x - 2, y - 1, x + f->usFaceWidth + 1, y + f->usFaceHeight, usLineColor, l.Buffer<UINT16>());

	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


static void HandleFaceHilights(const FACETYPE* const f, SGPVSurface* const uiBuffer, const INT16 sFaceX, const INT16 sFaceY)
{
	if (f->fDisabled) return;

	if (f->uiAutoDisplayBuffer == FRAME_BUFFER && guiCurrentScreen == GAME_SCREEN)
	{
		// If we are highlighted, do this now!
		if (f->uiFlags & FACE_SHOW_WHITE_HILIGHT)
		{
			DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, FROMRGB(255, 255, 255));
		}
		else if (f->uiFlags & FACE_SHOW_MOVING_HILIGHT)
		{
			const SOLDIERTYPE* const s = f->soldier;
			if (s != NULL && s->bLife >= OKLIFE)
			{
				const UINT32 color = (s->bStealthMode ? FROMRGB(158, 158, 12) : FROMRGB(8, 12, 118));
				DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, color);
			}
		}
		else
		{
			// ATE: Zero out any highlight boxzes....
			DrawFaceRect(f, f->uiAutoDisplayBuffer, f->usFaceX, f->usFaceY, FROMRGB(0, 0, 0));
		}
	}

	if (f->fCompatibleItems)
	{
		DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, FROMRGB(255, 0, 0));
	}
}


static void NewMouth(FACETYPE* pFace);


static void MouthAutoFace(FACETYPE* const f)
{
	Assert(f->fAllocated);
	Assert(!f->fDisabled);

	if (f->fTalking      &&
			!f->fInvalidAnim &&
			f->fAnimatingTalking)
	{
		// Check if we have an audio gap
		if (PollAudioGap(f->uiSoundID, &f->GapList))
		{
			f->sMouthFrame = 0;

			if (f->uiAutoRestoreBuffer == guiSAVEBUFFER)
			{
				FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthX, f->usMouthY, f->usMouthWidth, f->usMouthHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthOffsetX, f->usMouthOffsetY, f->usMouthWidth, f->usMouthHeight);
			}
		}
		else if (GetJA2Clock() - f->uiMouthlast > f->uiMouthDelay)
		{
			f->uiMouthlast = GetJA2Clock();

			NewMouth(f);

			const INT16 sFrame = f->sMouthFrame;
			if (sFrame > 0)
			{
				// Blit Accordingly!
				BltVideoObject(f->uiAutoDisplayBuffer, f->uiVideoObject, sFrame + 4, f->usMouthX, f->usMouthY);

				// Update rects
				if (f->uiAutoDisplayBuffer == FRAME_BUFFER)
				{
					InvalidateRegion(f->usMouthX, f->usMouthY, f->usMouthX + f->usMouthWidth, f->usMouthY + f->usMouthHeight);
				}
			}
			else
			{
				// Update rects just for Mouth
				if (f->uiAutoRestoreBuffer == guiSAVEBUFFER)
				{
					FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthX, f->usMouthY, f->usMouthWidth, f->usMouthHeight);
				}
				else
				{
					FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthOffsetX, f->usMouthOffsetY, f->usMouthWidth, f->usMouthHeight);
				}
			}

			HandleRenderFaceAdjustments(f, TRUE, 0, f->usFaceX, f->usFaceY, f->usEyesX, f->usEyesY);
		}
	}

	if  (!(f->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE))
	{
		HandleFaceHilights(f, f->uiAutoDisplayBuffer, f->usFaceX, f->usFaceY);
	}
}


static void SetupFinalTalkingDelay(FACETYPE* pFace);


static void HandleTalkingAutoFace(FACETYPE* const f)
{
	Assert(f->fAllocated);

	if (!f->fTalking) return;

	if (!f->fFinishTalking)
	{
		// Check if we are done talking
		if (f->fValidSpeech ?
		      !SoundIsPlaying(f->uiSoundID) :
		      GetJA2Clock() - f->uiTalkingTimer > f->uiTalkingDuration)
		{
			SetupFinalTalkingDelay(f);
		}
	}
	else
	{
		if (GetJA2Clock() - f->uiTalkingTimer > f->uiTalkingDuration)
		{
			// end of talking
			f->fTalking          = FALSE;
			f->fAnimatingTalking = FALSE;
			AudioGapListDone(&f->GapList); // Remove gap info
			HandleDialogueEnd(f);          // Call dialogue handler function
		}
	}
}


// Local function - uses these variables because they have already been validated
static void SetFaceShade(FACETYPE* const f, const BOOLEAN fExternBlit)
{
	const SOLDIERTYPE* const s = f->soldier;
	if (s == NULL) return;

	UINT32 shade;
	if (!fExternBlit && // ATE: Don't shade for damage if blitting extern face
			s->fFlashPortrait == FLASH_PORTRAIT_START)
	{
		shade = s->bFlashPortraitFrame;
	}
	else if (s->bLife < OKLIFE)
	{
		shade = FLASH_PORTRAIT_DARKSHADE;
	}
	else if (!fExternBlit                     &&
			f->video_overlay == NULL              &&
			s->bActionPoints == 0                 &&
			!(gTacticalStatus.uiFlags & REALTIME) &&
			gTacticalStatus.uiFlags & INCOMBAT)
	{
		shade = FLASH_PORTRAIT_LITESHADE;
	}
	else
	{
		shade = FLASH_PORTRAIT_NOSHADE; // Set to default
	}
	f->uiVideoObject->CurrentShade(shade);
}


void RenderAutoFaceFromSoldier(SOLDIERTYPE const* const s)
{
	// Check for valid soldier
	CHECKV(s);
	RenderAutoFace(s->face);
}


static void GetXYForIconPlacement(const FACETYPE* pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16* psX, INT16* psY)
{
	// Get height, width of icon...
	ETRLEObject const& pTrav    = guiPORTRAITICONS->SubregionProperties(ubIndex);
	UINT16      const  usHeight = pTrav.usHeight;
	UINT16      const  usWidth  = pTrav.usWidth;

	INT16 sX = sFaceX + pFace->usFaceWidth  - usWidth  - 1;
	INT16 sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}


static void GetXYForRightIconPlacement(const FACETYPE* pFace, UINT16 ubIndex, INT16 sFaceX, INT16 sFaceY, INT16* psX, INT16* psY, INT8 bNumIcons)
{
	// Get height, width of icon...
	ETRLEObject const& pTrav    = guiPORTRAITICONS->SubregionProperties(ubIndex);
	UINT16      const  usHeight = pTrav.usHeight;
	UINT16      const  usWidth  = pTrav.usWidth;

	INT16 sX = sFaceX + usWidth * bNumIcons + 1;
	INT16 sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}


static void DoRightIcon(SGPVSurface* const dst, FACETYPE* const pFace, const INT16 sFaceX, const INT16 sFaceY, const INT8 bNumIcons, const INT8 sIconIndex)
{
	INT16						sIconX, sIconY;

	// Find X, y for placement
	GetXYForRightIconPlacement( pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY, bNumIcons );
	BltVideoObject(dst, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);
}


static void HandleRenderFaceAdjustments(FACETYPE* const f, const BOOLEAN fDisplayBuffer, SGPVSurface* const buffer, const INT16 sFaceX, const INT16 sFaceY, const UINT16 usEyesX, const UINT16 usEyesY)
{
	// If we are using an extern buffer...
	SGPVSurface* uiRenderBuffer;
	if (buffer)
	{
		uiRenderBuffer = buffer;
	}
	else if (fDisplayBuffer)
	{
		uiRenderBuffer = f->uiAutoDisplayBuffer;
	}
	else
	{
		uiRenderBuffer = f->uiAutoRestoreBuffer;
	}

	// BLIT HATCH
	SOLDIERTYPE* const s = f->soldier;
	if (s != NULL)
	{
		if (s->bLife < CONSCIOUSNESS || s->fDeadPanel)
		{
			// Blit Closed eyes here!
			BltVideoObject(uiRenderBuffer, f->uiVideoObject, 1, usEyesX, usEyesY);

			// Blit hatch!
			BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
		}

		if (s->fMercAsleep)
		{
			// blit eyes closed
			BltVideoObject(uiRenderBuffer, f->uiVideoObject, 1, usEyesX, usEyesY);
		}

		if (s->uiStatusFlags & SOLDIER_DEAD)
		{
			// IF we are in the process of doing any deal/close animations, show face, not skill...
			if (!s->fClosePanel && !s->fDeadPanel && !s->fUIdeadMerc && !s->fUICloseMerc)
			{
				// Put close panel there
				BltVideoObject(uiRenderBuffer, guiDEAD, 5, sFaceX, sFaceY);

				// Blit hatch!
				BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
			}
		}

    // ATE: If talking in popup, don't do the other things.....
    if (f->fTalking && gTacticalStatus.uiFlags & IN_ENDGAME_SEQUENCE)
    {
      return;
    }

		// ATE: Only do this, because we can be talking during an interrupt....
		// Don't do this if we are being handled elsewhere and it's not an extern buffer...
		if (!(f->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) || buffer)
		{
			HandleFaceHilights(f, uiRenderBuffer, sFaceX, sFaceY);

#ifdef JA2BETAVERSION
			if (s->bOppCnt != 0)
#else
			if (s->bOppCnt > 0)
#endif
			{
				SetFontDestBuffer(uiRenderBuffer);

				wchar_t sString[32];
				swprintf(sString, lengthof(sString), L"%d", s->bOppCnt);

				SetFontAttributes(TINYFONT1, FONT_DKRED, DEFAULT_SHADOW, FONT_NEARBLACK);

				const INT16 sX1 = sFaceX;
				const INT16 sY1 = sFaceY;
				const INT16 sX2 = sX1 + StringPixLength(sString, TINYFONT1) + 1;
				const INT16 sY2 = sY1 + GetFontHeight(TINYFONT1) - 1;

				MPrint(sX1 + 1, sY1 - 1, sString);
				SetFontDestBuffer(FRAME_BUFFER);

				// Draw box
				SGPVSurface::Lock l(uiRenderBuffer);
				SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				const UINT16 usLineColor = Get16BPPColor(FROMRGB(105, 8, 9));
				RectangleDraw(TRUE, sX1, sY1, sX2, sY2, usLineColor, l.Buffer<UINT16>());
			}

			if ((s->bInSector && (gTacticalStatus.ubCurrentTeam != OUR_TEAM || !OK_INTERRUPT_MERC(s)) && !gfHiddenInterrupt) ||
					(gfSMDisableForItems && !gfInItemPickupMenu && gpSMCurrentMerc == s && gsCurInterfacePanel == SM_PANEL))
			{
				// Blit hatch!
				BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
			}

			// Render text above here if that's what was asked for
			if (!f->fDisabled    &&
					!f->fInvalidAnim &&
					f->fDisplayTextOver != FACE_NO_TEXT_OVER)
			{
				SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
				SetFontDestBuffer(uiRenderBuffer);

				INT16 sFontX;
				INT16 sFontY;
				FindFontCenterCoordinates(sFaceX, sFaceY, f->usFaceWidth, f->usFaceHeight, f->zDisplayText, TINYFONT1, &sFontX, &sFontY);

				if (f->fDisplayTextOver == FACE_DRAW_TEXT_OVER)
				{
					GPrintInvalidate(sFontX, sFontY, f->zDisplayText);
				}
				else if (f->fDisplayTextOver == FACE_ERASE_TEXT_OVER)
				{
					const INT16 w = StringPixLength(f->zDisplayText, TINYFONT1);
					const INT16 h = GetFontHeight(TINYFONT1);
					RestoreExternBackgroundRect(sFontX, sFontY, w, h);
					f->fDisplayTextOver = FACE_NO_TEXT_OVER;
				}

				SetFontDestBuffer(FRAME_BUFFER);
			}
		}

		INT16 sIconIndex = -1;

    // Check if a robot and is not controlled....
	  if (s->uiStatusFlags & SOLDIER_ROBOT && !CanRobotBeControlled(s)) sIconIndex = 5;

    if (ControllingRobot(s)) sIconIndex = 4;

		INT8 icon_pos = 0;
    if (s->bBlindedCounter > 0)               DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 6);
    if (s->bDrugEffect[DRUG_TYPE_ADRENALINE]) DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 7);
	  if (GetDrunkLevel(s) != SOBER)            DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 8);

		INT16   sPtsAvailable = 0;
		UINT16  usMaximumPts  = 0;
		BOOLEAN fShowNumber   = FALSE;
		switch (s->bAssignment)
		{
			case DOCTOR:
				sPtsAvailable = CalculateHealingPointsForDoctor(s, &usMaximumPts, FALSE);

				// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
				sPtsAvailable = (sPtsAvailable + 5) / 10;
				usMaximumPts  = (usMaximumPts  + 5) / 10;
				fShowNumber   = TRUE;
				sIconIndex    = 1;
				break;

			case PATIENT:
				// show current health / maximum health
				sPtsAvailable = s->bLife;
				usMaximumPts  = s->bLifeMax;
				fShowNumber   = TRUE;
				sIconIndex    = 2;
				break;

			case TRAIN_SELF:
			case TRAIN_TOWN:
			case TRAIN_TEAMMATE:
			case TRAIN_BY_OTHER:
			{
				// there could be bonus pts for training at gun range
				const BOOLEAN fAtGunRange =
					s->sSectorX == 13 && s->sSectorY == MAP_ROW_H && s->bSectorZ == 0;

				switch (s->bAssignment)
				{
					case TRAIN_SELF:
						sPtsAvailable = GetSoldierTrainingPts(s, s->bTrainStat, fAtGunRange, &usMaximumPts);
						break;

					case TRAIN_BY_OTHER:
						sPtsAvailable = GetSoldierStudentPts(s, s->bTrainStat, fAtGunRange, &usMaximumPts);
						break;

					case TRAIN_TOWN:
						sPtsAvailable = GetTownTrainPtsForCharacter(s, &usMaximumPts);
						// divide both amounts by 10 to make the displayed numbers a little more user-palatable (smaller)
						sPtsAvailable = (sPtsAvailable + 5) / 10;
						usMaximumPts  = (usMaximumPts  + 5) / 10;
						break;

					case TRAIN_TEAMMATE:
						sPtsAvailable = GetBonusTrainingPtsDueToInstructor(s, NULL , s->bTrainStat, fAtGunRange, &usMaximumPts);
						break;
				}
				fShowNumber = TRUE;
				sIconIndex  = 3;
				break;
			}

			case REPAIR:
				sPtsAvailable = CalculateRepairPointsForRepairman(s, &usMaximumPts, FALSE);
				// check if we are repairing a vehicle
				if (s->bVehicleUnderRepairID != -1)
				{
					// reduce to a multiple of VEHICLE_REPAIR_POINTS_DIVISOR.  This way skill too low will show up as 0 repair pts.
					sPtsAvailable -= sPtsAvailable % VEHICLE_REPAIR_POINTS_DIVISOR;
					usMaximumPts  -= usMaximumPts  % VEHICLE_REPAIR_POINTS_DIVISOR;
				}
				fShowNumber = TRUE;
				sIconIndex  = 0;
				break;
		}

		// Check for being serviced...
		if (s->service_partner != NULL) sIconIndex = 1; // Doctor
		if (s->ubServiceCount  != 0)    sIconIndex = 2; // Patient

		if (sIconIndex != -1)
		{
			// Find X, y for placement
			INT16 sIconX;
			INT16 sIconY;
			GetXYForIconPlacement(f, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY);
			BltVideoObject(uiRenderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);

      // ATE: Show numbers only in mapscreen
			if (fShowNumber)
			{
				SetFontDestBuffer(uiRenderBuffer);

				wchar_t sString[32];
				swprintf(sString, lengthof(sString), L"%d/%d", sPtsAvailable, usMaximumPts);

				SetFontAttributes(FONT10ARIAL, FONT_YELLOW);

				const UINT16 usTextWidth = StringPixLength(sString, FONT10ARIAL) + 1;
				MPrint(sFaceX + f->usFaceWidth - usTextWidth, sFaceY + 3, sString);
				SetFontDestBuffer(FRAME_BUFFER);
			}
		}
	}
  else if ((f->ubCharacterNum == FATHER || f->ubCharacterNum == MICKY) &&
			gMercProfiles[f->ubCharacterNum].bNPCData >= 5)
	{
		DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, 0, 8);
	}
}


void RenderAutoFace(FACETYPE* const f)
{
	CHECKV(f);
	CHECKV(f->fAllocated);
	CHECKV(!f->fDisabled);

	SetFaceShade(f, FALSE);

	INT32 x;
	INT32 y;
	if (f->uiAutoRestoreBuffer == guiSAVEBUFFER)
	{
		x = f->usFaceX;
		y = f->usFaceY;
	}
	else
	{
		x = 0;
		y = 0;
	}

	BltVideoObject(f->uiAutoRestoreBuffer, f->uiVideoObject, 0, x, y);
	HandleRenderFaceAdjustments(f, FALSE, 0, f->usFaceX, f->usFaceY, f->usEyesX, f->usEyesY);
	FaceRestoreSavedBackgroundRect(f, f->usFaceX, f->usFaceY, x, y, f->usFaceWidth, f->usFaceHeight);
}


static BOOLEAN ExternRenderFace(SGPVSurface* buffer, FACETYPE*, INT16 sX, INT16 sY);


BOOLEAN ExternRenderFaceFromSoldier(SGPVSurface* const buffer, const SOLDIERTYPE* s, const INT16 sX, const INT16 sY)
{
	// Check for valid soldier
	CHECKF(s != NULL);
	return ExternRenderFace(buffer, s->face, sX, sY);
}


/* To render an allocated face, but one that is independent of its active
 * status and does not require eye blinking or mouth movements, call */
static BOOLEAN ExternRenderFace(SGPVSurface* const buffer, FACETYPE* const pFace, const INT16 sX, const INT16 sY)
{
	UINT16						usEyesX;
	UINT16						usEyesY;
	UINT16						usMouthX;
	UINT16						usMouthY;

	CHECKF(pFace != NULL);

	// Check for a valid slot!
	CHECKF(pFace->fAllocated);

	// Here, any face can be rendered, even if disabled

	SetFaceShade(pFace, TRUE);

	// Blit face to save buffer!
	BltVideoObject(buffer, pFace->uiVideoObject, 0, sX, sY);

	GetFaceRelativeCoordinates( pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY );

	HandleRenderFaceAdjustments(pFace, FALSE, buffer, sX, sY, sX + usEyesX, sY + usEyesY);

	// Restore extern rect
	if (buffer == guiSAVEBUFFER)
	{
		RestoreExternBackgroundRect( sX, sY, pFace->usFaceWidth, pFace->usFaceWidth );
	}

	return( TRUE );
}


static void NewEye(FACETYPE* pFace)
{

 switch(pFace->sEyeFrame)
 {
  case 0 : //pFace->sEyeFrame = (INT16)Random(2);	// normal - can blink or frown
			if ( pFace->ubExpression == ANGRY )
			{
				pFace->ubEyeWait = 0;
				pFace->sEyeFrame = 3;
			}
			else if ( pFace->ubExpression == SURPRISED )
			{
				pFace->ubEyeWait = 0;
				pFace->sEyeFrame = 4;
			}
			else
	   //if (pFace->sEyeFrame && Talk.talking && Talk.expression != DYING)
	   ///    pFace->sEyeFrame = 3;
	   //else
	       pFace->sEyeFrame = 1;
	   break;
  case 1 : // starting to blink  - has to finish unless dying
	   //if (Talk.expression == DYING)
	   //    pFace->sEyeFrame = 1;
	   //else
	       pFace->sEyeFrame = 2;
	   break;
  case 2 : //pFace->sEyeFrame = (INT16)Random(2);	// finishing blink - can go normal or frown
	   //if (pFace->sEyeFrame && Talk.talking)
	   //    pFace->sEyeFrame = 3;
	   //else
	   //   if (Talk.expression == ANGRY)
		 // pFace->sEyeFrame = 3;
	   //   else
		  pFace->sEyeFrame = 0;
	   break;

  case 3 : //pFace->sEyeFrame = 4; break;	// frown

			pFace->ubEyeWait++;

			if ( pFace->ubEyeWait > 6 )
			{
				pFace->sEyeFrame = 0;
			}
			break;

  case 4 :

			pFace->ubEyeWait++;

			if ( pFace->ubEyeWait > 6 )
			{
				pFace->sEyeFrame = 0;
			}
			break;

  case 5 : pFace->sEyeFrame = 6;

		pFace->sEyeFrame = 0;
	   break;

  case 6 : pFace->sEyeFrame = 7; break;
  case 7 : pFace->sEyeFrame = (INT16)Random(2);	// can stop frowning or continue
	   //if (pFace->sEyeFrame && Talk.expression != DYING)
	    //   pFace->sEyeFrame = 8;
	   //else
	   //    pFace->sEyeFrame = 0;
	   //break;
  case 8 : pFace->sEyeFrame =  9; break;
  case 9 : pFace->sEyeFrame = 10; break;
  case 10: pFace->sEyeFrame = 11; break;
  case 11: pFace->sEyeFrame = 12; break;
  case 12: pFace->sEyeFrame =  0; break;
 }
}


static void NewMouth(FACETYPE* pFace)
{
	const UINT16 old_frame = pFace->sMouthFrame;
	UINT16       new_frame;
	do
	{
		new_frame = Random(6);
		if (new_frame > 3) new_frame = 0;
	}
	while (new_frame == old_frame);
	pFace->sMouthFrame = new_frame;
}


void HandleAutoFaces(void)
{
	FOR_ALL_FACES(f)
	{
		if (f->fDisabled) continue;

		SOLDIERTYPE* const s = f->soldier;
		if (s != NULL)
		{
			BOOLEAN render = FALSE;

			UINT32 new_flags = f->uiFlags & ~(FACE_SHOW_WHITE_HILIGHT | FACE_SHOW_MOVING_HILIGHT | FACE_REDRAW_WHOLE_FACE_NEXT_FRAME);
			if (s == gSelectedGuy)                                           new_flags |= FACE_SHOW_WHITE_HILIGHT;
			if (s->sGridNo != s->sFinalDestination && s->sGridNo != NOWHERE) new_flags |= FACE_SHOW_MOVING_HILIGHT;
			if (f->uiFlags != new_flags)                                     render     = TRUE;
			f->uiFlags = new_flags;

			if (s->bStealthMode != f->bOldStealthMode)
			{
				f->bOldStealthMode = s->bStealthMode;
				render             = TRUE;
			}

			const INT8 bLife = s->bLife;
			// Check if we have fallen below OKLIFE/CONSCIOUSNESS
			if ((bLife < OKLIFE)        != (f->bOldSoldierLife < OKLIFE))        render = TRUE;
			if ((bLife < CONSCIOUSNESS) != (f->bOldSoldierLife < CONSCIOUSNESS)) render = TRUE;
			f->bOldSoldierLife  = bLife;

			if (s->bOppCnt != f->bOldOppCnt)
			{
				f->bOldOppCnt = s->bOppCnt;
				render        = TRUE;
			}

			// Check if assignment is idfferent....
			if (s->bAssignment != f->bOldAssignment)
			{
				f->bOldAssignment = s->bAssignment;
				render            = TRUE;
			}

			const INT8 bAPs  = s->bActionPoints;
			if (bAPs == 0 && f->bOldActionPoints >  0) render = TRUE;
			if (bAPs >  0 && f->bOldActionPoints == 0) render = TRUE;
			f->bOldActionPoints = bAPs;

			if (f->ubOldServiceCount != s->ubServiceCount)
			{
				f->ubOldServiceCount = s->ubServiceCount;
				render               = TRUE;
			}

			if (f->fOldCompatibleItems != f->fCompatibleItems || gfInItemPickupMenu || gpItemPointer != NULL)
			{
				f->fOldCompatibleItems = f->fCompatibleItems;
				render                 = TRUE;
			}

			if (f->old_service_partner != s->service_partner)
			{
				f->old_service_partner = s->service_partner;
				render                 = TRUE;
			}

			switch (s->fFlashPortrait)
			{
				case FLASH_PORTRAIT_STOP:
					if (s->fGettingHit)
					{
						FlashSoldierPortrait(s);
						render = TRUE;
					}
					break;

				case FLASH_PORTRAIT_START:
					if (TIMECOUNTERDONE(s->PortraitFlashCounter, FLASH_PORTRAIT_DELAY))
					{
						RESETTIMECOUNTER(s->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);

						if (++s->bFlashPortraitFrame > FLASH_PORTRAIT_ENDSHADE)
						{
							s->bFlashPortraitFrame = FLASH_PORTRAIT_ENDSHADE;
							s->fFlashPortrait      = (s->fGettingHit ? FLASH_PORTRAIT_WAITING : FLASH_PORTRAIT_STOP);
						}
					}
					render = TRUE;
					break;

				case FLASH_PORTRAIT_WAITING:
					if (!s->fGettingHit)
					{
						s->fFlashPortrait = FLASH_PORTRAIT_STOP;
						render = TRUE;
					}
					break;
			}

			if (fInterfacePanelDirty == DIRTYLEVEL2 && guiCurrentScreen == GAME_SCREEN) render = TRUE;

			if (render) RenderAutoFace(f);
		}

		BlinkAutoFace(f);
		MouthAutoFace(f);
	}
}


void HandleTalkingAutoFaces( )
{
	FOR_ALL_FACES(f)
	{
		HandleTalkingAutoFace(f);
	}
}


static void FaceRestoreSavedBackgroundRect(FACETYPE const* const pFace, INT16 const sDestLeft, INT16 const sDestTop, INT16 const sSrcLeft, INT16 const sSrcTop, INT16 const sWidth, INT16 const sHeight)
{
	SGPBox const r = { sSrcLeft, sSrcTop, sWidth, sHeight };
	BltVideoSurface(pFace->uiAutoDisplayBuffer, pFace->uiAutoRestoreBuffer, sDestLeft, sDestTop, &r);

	// Add rect to frame buffer queue
	if ( pFace->uiAutoDisplayBuffer == FRAME_BUFFER )
	{
		InvalidateRegionEx(sDestLeft - 2, sDestTop - 2, sDestLeft + sWidth + 3, sDestTop + sHeight + 2);
	}
}


void SetFaceTalking(FACETYPE* const pFace, const char* const zSoundFile, const wchar_t* const zTextString)
{
	// Set face to talking
	pFace->fTalking = TRUE;
	pFace->fAnimatingTalking = TRUE;
	pFace->fFinishTalking = FALSE;

  if ( !AreInMeanwhile( ) )
  {
    TurnOnSectorLocator( pFace->ubCharacterNum );
  }

	// Play sample
	if( gGameSettings.fOptions[ TOPTION_SPEECH ] )
		pFace->uiSoundID = PlayJA2GapSample(zSoundFile, HIGHVOLUME, 1, MIDDLEPAN, &pFace->GapList);
	else
		pFace->uiSoundID = SOUND_ERROR;

	if ( pFace->uiSoundID != SOUND_ERROR )
	{
		pFace->fValidSpeech	= TRUE;

		pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock( );
	}
	else
	{
		pFace->fValidSpeech	= FALSE;

		// Set delay based on sound...
		pFace->uiTalkingTimer = pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock( );

		pFace->uiTalkingDuration = FindDelayForString( zTextString );
	}
}


void ExternSetFaceTalking(FACETYPE* const pFace, const UINT32 uiSoundID)
{
	// Set face to talki	ng
	pFace->fTalking = TRUE;
	pFace->fAnimatingTalking = TRUE;
	pFace->fFinishTalking = FALSE;
	pFace->fValidSpeech	= TRUE;

	pFace->uiSoundID = uiSoundID;
}


void InternalShutupaYoFace(FACETYPE* const pFace, const BOOLEAN fForce)
{
	CHECKV(pFace != NULL);

	if ( pFace->fTalking )
	{
		// OK, only do this if we have been talking for a min. amount fo time...
		if ( ( GetJA2Clock( ) - pFace->uiTalkingFromVeryBeginningTimer ) < 500 && !fForce )
		{
			return;
		}

		if ( pFace->uiSoundID != SOUND_ERROR )
		{
			SoundStop( pFace->uiSoundID );
		}

		// Remove gap info
		AudioGapListDone( &(pFace->GapList) );

		// Shutup mouth!
		pFace->sMouthFrame = 0;

		// ATE: Only change if active!
		if ( !pFace->fDisabled )
		{
			if ( pFace->uiAutoRestoreBuffer == guiSAVEBUFFER )
			{
				FaceRestoreSavedBackgroundRect(pFace, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(pFace, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY,  pFace->usMouthWidth, pFace->usMouthHeight);
			}
		}
		// OK, smart guy, make sure this guy has finished talking,
		// before attempting to end dialogue UI.
		pFace->fTalking = FALSE;

		// Call dialogue handler function
		HandleDialogueEnd( pFace );

		pFace->fTalking = FALSE;
		pFace->fAnimatingTalking = FALSE;

		gfUIWaitingForUserSpeechAdvance = FALSE;
	}
}


void ShutupaYoFace(FACETYPE* const face)
{
	InternalShutupaYoFace(face, TRUE);
}


static void SetupFinalTalkingDelay(FACETYPE* const f)
{
	f->fFinishTalking    = TRUE;
	f->fAnimatingTalking = FALSE;
	f->uiTalkingTimer    = GetJA2Clock();
	f->uiTalkingDuration = 300;
	f->sMouthFrame       = 0;

	// Close mouth!
	if (!f->fDisabled)
	{
		if (f->uiAutoRestoreBuffer == guiSAVEBUFFER)
		{
			FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthX, f->usMouthY, f->usMouthWidth, f->usMouthHeight);
		}
		else
		{
			FaceRestoreSavedBackgroundRect(f, f->usMouthX, f->usMouthY, f->usMouthOffsetX, f->usMouthOffsetY, f->usMouthWidth, f->usMouthHeight);
		}
	}

	// Setup flag to wait for advance (because we have no text!)
	if (gGameSettings.fOptions[TOPTION_KEY_ADVANCE_SPEECH] && f->uiFlags & FACE_POTENTIAL_KEYWAIT)
	{
		// Check if we have had valid speech!
		if (!f->fValidSpeech || gGameSettings.fOptions[TOPTION_SUBTITLES])
		{
			f->fFinishTalking = FALSE;
			// Set waiting for advance to true!
			gfUIWaitingForUserSpeechAdvance = TRUE;
		}
	}

	f->fValidSpeech = FALSE;
}
