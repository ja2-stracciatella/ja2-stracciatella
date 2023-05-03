#include "Faces.h"
#include "Assignments.h"
#include "ContentManager.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Drugs_And_Alcohol.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Gap.h"
#include "Handle_UI.h"
#include "HImage.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "Interface_Items.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "JAScreens.h"
#include "Line.h"
#include "Local.h"
#include "Logger.h"
#include "Map_Screen_Interface.h"
#include "Meanwhile.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Quests.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "RPCSmallFaceModel.h"
#include "ScreenIDs.h"
#include "Soldier_Control.h"
#include "Soldier_Macros.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Squads.h"
#include "SysUtil.h"
#include "TeamTurns.h"
#include "Timer_Control.h"
#include "UILayout.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include "WCheck.h"
#include <stdexcept>
#include <string_theory/format>



// Defines
#define NUM_FACE_SLOTS 50


// GLOBAL FOR FACES LISTING
static FACETYPE gFacesData[NUM_FACE_SLOTS];
static UINT32   guiNumFaces = 0;


#define FOR_EACH_FACE(iter)                                    \
	for (FACETYPE* iter = gFacesData,               \
		* const iter##__end = gFacesData + guiNumFaces; \
		iter != iter##__end;                                    \
		++iter)                                                 \
		if (!iter->fAllocated) continue; else



static SGPVObject* guiPORTRAITICONS;


static FACETYPE& GetFreeFace(void)
{
	for (FACETYPE* i = gFacesData; i != gFacesData + guiNumFaces; ++i)
	{
		if (!i->fAllocated) return *i;
	}
	if (guiNumFaces < NUM_FACE_SLOTS) return gFacesData[guiNumFaces++];
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
	s.face = &InitFace(s.ubProfile, &s, 0);
}


FACETYPE& InitFace(const ProfileID id, SOLDIERTYPE* const s, const UINT32 uiInitFlags)
{
	if (id == NO_PROFILE) throw std::logic_error("Tried to load face for invalid profile");
	MERCPROFILESTRUCT const& p = GetProfile(id);

	FACETYPE& f = GetFreeFace();

	ST::string face_file;
	// Check if we are a big-face....
	if (uiInitFlags & FACE_BIGFACE)
	{
		face_file = FACESDIR "/b{02d}.sti";
		// ATE: Check for profile - if elliot, use special face :)
		if (id == ELLIOT && p.bNPCData > 3)
		{
			if      (p.bNPCData <   7) face_file = FACESDIR "/b{02d}a.sti";
			else if (p.bNPCData <  10) face_file = FACESDIR "/b{02d}b.sti";
			else if (p.bNPCData <  13) face_file = FACESDIR "/b{02d}c.sti";
			else if (p.bNPCData <  16) face_file = FACESDIR "/b{02d}d.sti";
			else if (p.bNPCData == 17) face_file = FACESDIR "/b{02d}e.sti";
		}
	}
	else
	{
		face_file = FACESDIR "/{02d}.sti";
	}

	// HERVE, PETER, ALBERTO and CARLO all use HERVE's portrait
	INT32 const face_id = HERVE <= id && id <= CARLO ? HERVE : p.ubFaceIndex;

	ST::string ImageFile = ST::format(face_file.c_str(), face_id);
	SGPVObject* const vo = AddVideoObjectFromFile(ImageFile);

	f = FACETYPE{};
	f.uiFlags               = uiInitFlags;
	f.fAllocated            = TRUE;
	f.fDisabled             = TRUE; // default to off!
	f.video_overlay         = NULL;
	f.soldier               = s;
	f.ubCharacterNum        = id;
	f.sEyeFrame             = 0;
	f.uiEyeDelay            = 50 + Random(30);

	UINT32 blink_freq = p.uiBlinkFrequency;
	blink_freq = (Random(2) ? blink_freq + Random(2000) : blink_freq - Random(2000));
	f.uiBlinkFrequency      = blink_freq;

	f.uiExpressionFrequency = p.uiExpressionFrequency;
	f.sMouthFrame           = 0;
	f.uiVideoObject         = vo;

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
	f.usFaceWidth  = face_gfx.usWidth;
	f.usFaceHeight = face_gfx.usHeight;

	// OK, check # of items
	if (vo->SubregionCount() == 8)
	{
		// Get EYE height, width
		ETRLEObject const& eyes_gfx = vo->SubregionProperties(1);
		f.usEyesWidth  = eyes_gfx.usWidth;
		f.usEyesHeight = eyes_gfx.usHeight;

		// Get Mouth height, width
		ETRLEObject const& mouth_gfx = vo->SubregionProperties(5);
		f.usMouthWidth  = mouth_gfx.usWidth;
		f.usMouthHeight = mouth_gfx.usHeight;

		f.fInvalidAnim = FALSE;
	}
	else
	{
		f.fInvalidAnim = TRUE;
	}

	return f;
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

	SetAutoFaceInActive(*pFace);

	// If we are still talking, stop!
	if ( pFace->fTalking )
	{
		// Call dialogue handler function
		pFace->fTalking = FALSE;

		HandleDialogueEnd(*pFace);
	}

	// Delete vo
	DeleteVideoObject(pFace->uiVideoObject);

	// Set uncallocated
	pFace->fAllocated = FALSE;

	RecountFaces( );
}


static void GetFaceRelativeCoordinates(FACETYPE const& f, UINT16* const pusEyesX, UINT16* const pusEyesY, UINT16* const pusMouthX, UINT16* const pusMouthY)
{
	ProfileID         const  pid = f.ubCharacterNum;
	MERCPROFILESTRUCT const& p   = GetProfile(pid);

	// Take eyes x,y from profile unless we are an RPC and we are small faced
	// Are we a recruited merc or small?
	if (f.uiFlags & FACE_FORCE_SMALL ||
			(!(f.uiFlags & FACE_BIGFACE) && p.ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE)))
	{
		const RPCSmallFaceModel* face = GCM->getRPCSmallFaceOffsets(pid);
		if (face)
		{
			*pusEyesX = face->bEyesX;
			*pusEyesY = face->bEyesY;
			*pusMouthX = face->bMouthX;
			*pusMouthY = face->bMouthY;
			return;
		}
	}

	*pusEyesX  = p.usEyesX;
	*pusEyesY  = p.usEyesY;
	*pusMouthX = p.usMouthX;
	*pusMouthY = p.usMouthY;
}


static void InternalSetAutoFaceActive(SGPVSurface* display, SGPVSurface* restore, FACETYPE&, UINT16 usFaceX, UINT16 usFaceY, UINT16 usEyesX, UINT16 usEyesY, UINT16 usMouthX, UINT16 usMouthY);


void SetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, FACETYPE& f, UINT16 const usFaceX, UINT16 const usFaceY)
{
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	GetFaceRelativeCoordinates(f, &usEyesX, &usEyesY, &usMouthX, &usMouthY);
	InternalSetAutoFaceActive(display, restore, f, usFaceX, usFaceY, usEyesX, usEyesY, usMouthX, usMouthY);
}


static void InternalSetAutoFaceActive(SGPVSurface* const display, SGPVSurface* const restore, FACETYPE& f, UINT16 const usFaceX, UINT16 const usFaceY, UINT16 const usEyesX, UINT16 const usEyesY, UINT16 const usMouthX, UINT16 const usMouthY)
{
	// IF we are already being contained elsewhere, return without doing anything!

	// ATE: Don't allow another activity from setting active....
	if (f.uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE)
	{
		return;
	}

	// Check if we are active already, remove if so!
	SetAutoFaceInActive(f);

	if (restore == FACE_AUTO_RESTORE_BUFFER)
	{
		f.fAutoRestoreBuffer  = TRUE;
		f.uiAutoRestoreBuffer = AddVideoSurface(f.usFaceWidth, f.usFaceHeight, PIXEL_DEPTH);
	}
	else
	{
		f.fAutoRestoreBuffer  = FALSE;
		f.uiAutoRestoreBuffer = restore;
	}

	if (display == FACE_AUTO_DISPLAY_BUFFER)
	{
		f.fAutoDisplayBuffer  = TRUE;
		f.uiAutoDisplayBuffer = AddVideoSurface(f.usFaceWidth, f.usFaceHeight, PIXEL_DEPTH);
	}
	else
	{
		f.fAutoDisplayBuffer  = FALSE;
		f.uiAutoDisplayBuffer = display;
	}

	f.usFaceX               = usFaceX;
	f.usFaceY               = usFaceY;
	f.fCanHandleInactiveNow = FALSE;

	//Take eyes x,y from profile unless we are an RPC and we are small faced.....
	f.usEyesX  = usEyesX  + usFaceX;
	f.usEyesY  = usEyesY  + usFaceY;
	f.usMouthY = usMouthY + usFaceY;
	f.usMouthX = usMouthX + usFaceX;

	// Save offset values
	f.usEyesOffsetX  = usEyesX;
	f.usEyesOffsetY  = usEyesY;
	f.usMouthOffsetY = usMouthY;
	f.usMouthOffsetX = usMouthX;

	if (f.usEyesY == usFaceY || f.usMouthY == usFaceY)
	{
		f.fInvalidAnim = TRUE;
	}

	f.fDisabled        = FALSE;
	f.uiLastBlink      = GetJA2Clock();
	f.uiLastExpression = GetJA2Clock();
	f.uiEyelast        = GetJA2Clock();

	// Are we a soldier?
	if (f.soldier) f.bOldSoldierLife = f.soldier->bLife;
}


void SetAutoFaceInActive(FACETYPE& f)
{
	// Check for a valid slot!
	CHECKV(f.fAllocated);

	if (f.fDisabled) return;

	// Turn off some flags
	if (f.uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE)
	{
		if (!f.fCanHandleInactiveNow)
		{
			return;
		}
	}

	if (f.uiFlags & FACE_MAKEACTIVE_ONCE_DONE)
	{
		SOLDIERTYPE const* const pSoldier = f.soldier;
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

	if (f.fAutoRestoreBuffer)
	{
		DeleteVideoSurface(f.uiAutoRestoreBuffer);
		f.uiAutoRestoreBuffer = 0;
	}
	if (f.fAutoDisplayBuffer)
	{
		DeleteVideoSurface(f.uiAutoDisplayBuffer);
		f.uiAutoDisplayBuffer = 0;
	}

	if (f.video_overlay)
	{
		RemoveVideoOverlay(f.video_overlay);
		f.video_overlay = 0;
	}

	// Turn off some flags
	f.uiFlags &= ~FACE_INACTIVE_HANDLED_ELSEWHERE;

	// Disable!
	f.fDisabled = TRUE;
}


void SetAllAutoFacesInactive(  )
{
	FOR_EACH_FACE(i)
	{
		SetAutoFaceInActive(*i);
	}
}


static void NewEye(FACETYPE&);
static void HandleRenderFaceAdjustments(FACETYPE&, BOOLEAN fDisplayBuffer, SGPVSurface* buffer, INT16 sFaceX, INT16 sFaceY, UINT16 usEyesX, UINT16 usEyesY);
static void FaceRestoreSavedBackgroundRect(FACETYPE const&, INT16 sDestLeft, INT16 sDestTop, UINT16 sSrcLeft, UINT16 sSrcTop, UINT16 sWidth, UINT16 sHeight);


static void BlinkAutoFace(FACETYPE& f)
{
	Assert(f.fAllocated);
	Assert(!f.fDisabled);

	if (f.fInvalidAnim) return;

	// CHECK IF BUDDY IS DEAD, UNCONSCIOUS, ASLEEP, OR POW!
	SOLDIERTYPE const* const s = f.soldier;
	if (s != NULL && (
		s->fMercAsleep ||
		s->bLife < OKLIFE ||
		s->bAssignment == ASSIGNMENT_POW))
	{
		return;
	}

	if (f.ubExpression == NO_EXPRESSION)
	{
		// Get Delay time, if the first frame, use a different delay
		if (GetJA2Clock() - f.uiLastBlink > f.uiBlinkFrequency)
		{
			f.uiLastBlink  = GetJA2Clock();
			f.ubExpression = BLINKING;
			f.uiEyelast    = GetJA2Clock();
		}

		if (f.fAnimatingTalking &&
			GetJA2Clock() - f.uiLastExpression > f.uiExpressionFrequency)
		{
			f.uiLastExpression = GetJA2Clock();
			f.ubExpression     = (Random(2) == 0 ? ANGRY : SURPRISED);
		}
	}

	if (f.ubExpression != NO_EXPRESSION &&
		GetJA2Clock() - f.uiEyelast > f.uiEyeDelay) // Are we going to blink?
	{
		f.uiEyelast = GetJA2Clock();

		NewEye(f);

		INT16 sFrame = f.sEyeFrame;
		if (sFrame > 0)
		{
			// Blit Accordingly!
			BltVideoObject(f.uiAutoDisplayBuffer, f.uiVideoObject, sFrame, f.usEyesX, f.usEyesY);

			if (f.uiAutoDisplayBuffer == FRAME_BUFFER)
			{
				InvalidateRegion(f.usEyesX, f.usEyesY, f.usEyesX + f.usEyesWidth, f.usEyesY + f.usEyesHeight);
			}
		}
		else
		{
			f.ubExpression = NO_EXPRESSION;
			// Update rects just for eyes

			if (f.uiAutoRestoreBuffer == guiSAVEBUFFER)
			{
				FaceRestoreSavedBackgroundRect(f, f.usEyesX, f.usEyesY, f.usEyesX, f.usEyesY, f.usEyesWidth, f.usEyesHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(f, f.usEyesX, f.usEyesY, f.usEyesOffsetX, f.usEyesOffsetY, f.usEyesWidth, f.usEyesHeight);
			}
		}

		HandleRenderFaceAdjustments(f, TRUE, 0, f.usFaceX, f.usFaceY, f.usEyesX, f.usEyesY);
	}
}


static void DrawFaceRect(FACETYPE const& f, SGPVSurface* const buffer, const INT16 x, const INT16 y, const UINT32 colour)
{
	SGPVSurface::Lock l(buffer);
	UINT32 const uiDestPitchBYTES = l.Pitch();

	SetClippingRegionAndImageWidth(uiDestPitchBYTES, x - 2, y - 1, f.usFaceWidth + 4, f.usFaceHeight + 4);

	const UINT16 usLineColor = Get16BPPColor(colour);
	RectangleDraw(TRUE, x - 2, y - 1, x + f.usFaceWidth + 1, y + f.usFaceHeight, usLineColor, l.Buffer<UINT16>());

	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


static void HandleFaceHilights(FACETYPE const& f, SGPVSurface* const uiBuffer, const INT16 sFaceX, const INT16 sFaceY)
{
	if (f.fDisabled) return;

	if (f.uiAutoDisplayBuffer == FRAME_BUFFER && guiCurrentScreen == GAME_SCREEN)
	{
		// If we are highlighted, do this now!
		if (f.uiFlags & FACE_SHOW_WHITE_HILIGHT)
		{
			DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, FROMRGB(255, 255, 255));
		}
		else if (f.uiFlags & FACE_SHOW_MOVING_HILIGHT)
		{
			SOLDIERTYPE const* const s = f.soldier;
			if (s != NULL && s->bLife >= OKLIFE)
			{
				const UINT32 color = (s->bStealthMode ? FROMRGB(158, 158, 12) : FROMRGB(8, 12, 118));
				DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, color);
			}
		}
		else
		{
			// ATE: Zero out any highlight boxzes....
			DrawFaceRect(f, f.uiAutoDisplayBuffer, f.usFaceX, f.usFaceY, FROMRGB(0, 0, 0));
		}
	}

	if (f.fCompatibleItems)
	{
		DrawFaceRect(f, uiBuffer, sFaceX, sFaceY, FROMRGB(255, 0, 0));
	}
}


static void NewMouth(FACETYPE&);


static void MouthAutoFace(FACETYPE& f)
{
	Assert(f.fAllocated);
	Assert(!f.fDisabled);

	if (f.fTalking && !f.fInvalidAnim && f.fAnimatingTalking)
	{
		// Check if we have an audio gap
		if (PollAudioGap(f.uiSoundID, &f.GapList))
		{
			f.sMouthFrame = 0;

			if (f.uiAutoRestoreBuffer == guiSAVEBUFFER)
			{
				FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthX, f.usMouthY, f.usMouthWidth, f.usMouthHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthOffsetX, f.usMouthOffsetY, f.usMouthWidth, f.usMouthHeight);
			}
		}
		else if (GetJA2Clock() - f.uiMouthlast > 120)
		{
			f.uiMouthlast = GetJA2Clock();

			NewMouth(f);

			INT16 const sFrame = f.sMouthFrame;
			if (sFrame > 0)
			{
				// Blit Accordingly!
				BltVideoObject(f.uiAutoDisplayBuffer, f.uiVideoObject, sFrame + 4, f.usMouthX, f.usMouthY);

				// Update rects
				if (f.uiAutoDisplayBuffer == FRAME_BUFFER)
				{
					InvalidateRegion(f.usMouthX, f.usMouthY, f.usMouthX + f.usMouthWidth, f.usMouthY + f.usMouthHeight);
				}
			}
			else
			{
				// Update rects just for Mouth
				if (f.uiAutoRestoreBuffer == guiSAVEBUFFER)
				{
					FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthX, f.usMouthY, f.usMouthWidth, f.usMouthHeight);
				}
				else
				{
					FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthOffsetX, f.usMouthOffsetY, f.usMouthWidth, f.usMouthHeight);
				}
			}

			HandleRenderFaceAdjustments(f, TRUE, 0, f.usFaceX, f.usFaceY, f.usEyesX, f.usEyesY);
		}
	}

	if  (!(f.uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE))
	{
		HandleFaceHilights(f, f.uiAutoDisplayBuffer, f.usFaceX, f.usFaceY);
	}
}


static void SetupFinalTalkingDelay(FACETYPE&);


static void HandleTalkingAutoFace(FACETYPE& f)
{
	Assert(f.fAllocated);

	if (!f.fTalking) return;

	if (!f.fFinishTalking)
	{
		// Check if we are done talking
		if (f.fValidSpeech ?
			!SoundIsPlaying(f.uiSoundID) :
			GetJA2Clock() - f.uiTalkingTimer > f.uiTalkingDuration)
		{
			SetupFinalTalkingDelay(f);
		}
	}
	else
	{
		if (GetJA2Clock() - f.uiTalkingTimer > f.uiTalkingDuration)
		{
			// end of talking
			f.fTalking          = FALSE;
			f.fAnimatingTalking = FALSE;
			AudioGapListDone(&f.GapList); // Remove gap info
			HandleDialogueEnd(f);          // Call dialogue handler function
		}
	}
}


// Local function - uses these variables because they have already been validated
static void SetFaceShade(FACETYPE const& f, BOOLEAN const fExternBlit)
{
	SOLDIERTYPE const* const s = f.soldier;
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
	else if (!fExternBlit &&
		!f.video_overlay &&
		s->bActionPoints == 0 &&
		gTacticalStatus.uiFlags & INCOMBAT)
	{
		shade = FLASH_PORTRAIT_LITESHADE;
	}
	else
	{
		shade = FLASH_PORTRAIT_NOSHADE; // Set to default
	}
	f.uiVideoObject->CurrentShade(shade);
}


static void GetXYForIconPlacement(FACETYPE const& f, UINT16 const ubIndex, INT16 const sFaceX, INT16 const sFaceY, INT16* const psX, INT16* const psY)
{
	// Get height, width of icon...
	ETRLEObject const& pTrav    = guiPORTRAITICONS->SubregionProperties(ubIndex);
	UINT16      const  usHeight = pTrav.usHeight;
	UINT16      const  usWidth  = pTrav.usWidth;

	INT16 const sX = sFaceX + f.usFaceWidth  - usWidth  - 1;
	INT16 const sY = sFaceY + f.usFaceHeight - usHeight - 1;

	*psX = sX;
	*psY = sY;
}


static void DoRightIcon(SGPVSurface* const dst, FACETYPE const& f, INT16 const face_x, INT16 const face_y, INT8 const n_icons, INT8 const icon_idx)
{
	ETRLEObject const& e = guiPORTRAITICONS->SubregionProperties(icon_idx);
	INT16       const  x = face_x + e.usWidth * n_icons + 1;
	INT16       const  y = face_y + f.usFaceHeight - e.usHeight - 1;
	BltVideoObject(dst, guiPORTRAITICONS, icon_idx, x, y);
}


static void HandleRenderFaceAdjustments(FACETYPE& f, BOOLEAN const fDisplayBuffer, SGPVSurface* const buffer, INT16 const sFaceX, INT16 const sFaceY, UINT16 const usEyesX, UINT16 const usEyesY)
{
	// If we are using an extern buffer...
	SGPVSurface* uiRenderBuffer;
	if (buffer)
	{
		uiRenderBuffer = buffer;
	}
	else if (fDisplayBuffer)
	{
		uiRenderBuffer = f.uiAutoDisplayBuffer;
	}
	else
	{
		uiRenderBuffer = f.uiAutoRestoreBuffer;
	}

	// BLIT HATCH
	SOLDIERTYPE* const s = f.soldier;
	if (s != NULL)
	{
		if (s->bLife < CONSCIOUSNESS || s->fDeadPanel)
		{
			// Blit Closed eyes here!
			BltVideoObject(uiRenderBuffer, f.uiVideoObject, 1, usEyesX, usEyesY);

			// Blit hatch!
			BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
		}

		if (s->fMercAsleep)
		{
			// blit eyes closed
			BltVideoObject(uiRenderBuffer, f.uiVideoObject, 1, usEyesX, usEyesY);
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
		if (f.fTalking && gTacticalStatus.uiFlags & IN_ENDGAME_SEQUENCE)
		{
			return;
		}

		// ATE: Only do this, because we can be talking during an interrupt....
		// Don't do this if we are being handled elsewhere and it's not an extern buffer...
		if (!(f.uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) || buffer)
		{
			HandleFaceHilights(f, uiRenderBuffer, sFaceX, sFaceY);

			if (s->bOppCnt > 0)
			{
				SetFontDestBuffer(uiRenderBuffer);

				ST::string sString = ST::format("{}", s->bOppCnt);

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

			if ((s->bInSector && (gTacticalStatus.ubCurrentTeam != OUR_TEAM || !OK_INTERRUPT_MERC(s)) &&
				!gfHiddenInterrupt) ||
				(gfSMDisableForItems && !gfInItemPickupMenu &&
				gpSMCurrentMerc == s && gsCurInterfacePanel == SM_PANEL))
			{
				// Blit hatch!
				BltVideoObject(uiRenderBuffer, guiHATCH, 0, sFaceX, sFaceY);
			}

			// Render text above here if that's what was asked for
			if (!f.fDisabled &&
				!f.fInvalidAnim &&
				f.fDisplayTextOver != FACE_NO_TEXT_OVER)
			{
				SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
				SetFontDestBuffer(uiRenderBuffer);

				INT16 sFontX;
				INT16 sFontY;
				FindFontCenterCoordinates(sFaceX, sFaceY, f.usFaceWidth, f.usFaceHeight, f.zDisplayText, TINYFONT1, &sFontX, &sFontY);

				if (f.fDisplayTextOver == FACE_DRAW_TEXT_OVER)
				{
					GPrintInvalidate(sFontX, sFontY, f.zDisplayText);
				}
				else if (f.fDisplayTextOver == FACE_ERASE_TEXT_OVER)
				{
					INT16 const w = StringPixLength(f.zDisplayText, TINYFONT1);
					INT16 const h = GetFontHeight(TINYFONT1);
					RestoreExternBackgroundRect(sFontX, sFontY, w, h);
					f.fDisplayTextOver = FACE_NO_TEXT_OVER;
				}

				SetFontDestBuffer(FRAME_BUFFER);
			}
		}

		INT16 sIconIndex = -1;

		// Check if a robot and is not controlled....
		if (s->uiStatusFlags & SOLDIER_ROBOT && !CanRobotBeControlled(s))
			sIconIndex = 5;

		if (ControllingRobot(s))
			sIconIndex = 4;

		INT8 icon_pos = 0;
		if (s->bBlindedCounter > 0)
			DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 6);
		if (s->bDrugEffect[DRUG_TYPE_ADRENALINE])
			DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 7);
		if (GetDrunkLevel(s) != SOBER)
			DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, icon_pos++, 8);

		INT16   sPtsAvailable = 0;
		UINT16  usMaximumPts  = 0;
		BOOLEAN fShowNumber   = FALSE;
		static const SGPSector gunRange(GUN_RANGE_X, GUN_RANGE_Y, GUN_RANGE_Z);
		switch (s->bAssignment)
		{
			case DOCTOR:
				sPtsAvailable = CalculateHealingPointsForDoctor(s, &usMaximumPts, FALSE);

				// divide both amounts by 10 to make the displayed numbers a
				// little more user-palatable (smaller)
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
				const BOOLEAN fAtGunRange = s->sSector == gunRange;

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
						// divide both amounts by 10 to make the displayed numbers a
						// little more user-palatable (smaller)
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
					// reduce to a multiple of VEHICLE_REPAIR_POINTS_DIVISOR.
					// This way skill too low will show up as 0 repair pts.
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

				ST::string sString = ST::format("{}/{}", sPtsAvailable, usMaximumPts);

				SetFontAttributes(FONT10ARIAL, FONT_YELLOW);

				const UINT16 usTextWidth = StringPixLength(sString, FONT10ARIAL) + 1;
				MPrint(sFaceX + f.usFaceWidth - usTextWidth, sFaceY + 3, sString);
				SetFontDestBuffer(FRAME_BUFFER);
			}
		}
	}
	else if ((f.ubCharacterNum == FATHER || f.ubCharacterNum == MICKY) &&
			gMercProfiles[f.ubCharacterNum].bNPCData >= 5)
	{
		DoRightIcon(uiRenderBuffer, f, sFaceX, sFaceY, 0, 8);
	}
}


void RenderAutoFace(FACETYPE& f)
{
	CHECKV(f.fAllocated);
	CHECKV(!f.fDisabled);

	SetFaceShade(f, FALSE);

	INT32 x;
	INT32 y;
	if (f.uiAutoRestoreBuffer == guiSAVEBUFFER)
	{
		x = f.usFaceX;
		y = f.usFaceY;
	}
	else
	{
		x = 0;
		y = 0;
	}

	BltVideoObject(f.uiAutoRestoreBuffer, f.uiVideoObject, 0, x, y);
	HandleRenderFaceAdjustments(f, FALSE, 0, f.usFaceX, f.usFaceY, f.usEyesX, f.usEyesY);
	FaceRestoreSavedBackgroundRect(f, f.usFaceX, f.usFaceY, x, y, f.usFaceWidth, f.usFaceHeight);
}


void ExternRenderFace(SGPVSurface* const buffer, FACETYPE& f, INT16 const sX, INT16 const sY)
{
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;

	// Check for a valid slot!
	CHECKV(f.fAllocated);

	// Here, any face can be rendered, even if disabled

	SetFaceShade(f, TRUE);

	// Blit face to save buffer!
	BltVideoObject(buffer, f.uiVideoObject, 0, sX, sY);

	GetFaceRelativeCoordinates(f, &usEyesX, &usEyesY, &usMouthX, &usMouthY);

	HandleRenderFaceAdjustments(f, FALSE, buffer, sX, sY, sX + usEyesX, sY + usEyesY);

	// Restore extern rect
	if (buffer == guiSAVEBUFFER)
	{
		RestoreExternBackgroundRect(sX, sY, f.usFaceWidth, f.usFaceWidth);
	}
}


static void NewEye(FACETYPE& f)
{
	switch(f.sEyeFrame)
	{
		case 0: // normal
			if ( f.ubExpression == ANGRY )
			{
				f.ubEyeWait = 0;
				f.sEyeFrame = 3;
			}
			else if ( f.ubExpression == SURPRISED )
			{
				f.ubEyeWait = 0;
				f.sEyeFrame = 4;
			}
			else
				f.sEyeFrame = 1;
			break;

		case 1 : // blink (eyelid down)
			f.sEyeFrame = 2;
			break;
		case 2 : // blink (eyelid up)
			f.sEyeFrame = 0;
			break;

		case 3 : // frown (eyebrown down)
			f.ubEyeWait++;
			if ( f.ubEyeWait > 6 )
			{
				f.sEyeFrame = 0;
			}
			break;

		case 4 : // surprise (eyebrow up)
			f.ubEyeWait++;
			if ( f.ubEyeWait > 6 )
			{
				f.sEyeFrame = 0;
			}
			break;

		default:
			SLOGW("unexpected eye frame ({})", f.sEyeFrame);
			f.sEyeFrame = 0;
			break;
	}
}


static void NewMouth(FACETYPE& f)
{
	UINT16 const old_frame = f.sMouthFrame;
	UINT16       new_frame;
	do
	{
		new_frame = Random(6);
		if (new_frame > 3) new_frame = 0;
	}
	while (new_frame == old_frame);
	f.sMouthFrame = new_frame;
}


void HandleAutoFaces(void)
{
	FOR_EACH_FACE(i)
	{
		FACETYPE& f = *i;
		if (f.fDisabled) continue;

		SOLDIERTYPE* const s = f.soldier;
		if (s != NULL)
		{
			BOOLEAN render = FALSE;

			UINT32 new_flags = f.uiFlags & ~(FACE_SHOW_WHITE_HILIGHT | FACE_SHOW_MOVING_HILIGHT | FACE_REDRAW_WHOLE_FACE_NEXT_FRAME);
			if (s == gSelectedGuy)
				new_flags |= FACE_SHOW_WHITE_HILIGHT;
			if (s->sGridNo != s->sFinalDestination && s->sGridNo != NOWHERE)
				new_flags |= FACE_SHOW_MOVING_HILIGHT;
			if (f.uiFlags != new_flags)
				render = TRUE;
			f.uiFlags = new_flags;

			if (s->bStealthMode != f.bOldStealthMode)
			{
				f.bOldStealthMode = s->bStealthMode;
				render             = TRUE;
			}

			const INT8 bLife = s->bLife;
			// Check if we have fallen below OKLIFE/CONSCIOUSNESS
			if ((bLife < OKLIFE)        != (f.bOldSoldierLife < OKLIFE))        render = TRUE;
			if ((bLife < CONSCIOUSNESS) != (f.bOldSoldierLife < CONSCIOUSNESS)) render = TRUE;
			f.bOldSoldierLife  = bLife;

			if (s->bOppCnt != f.bOldOppCnt)
			{
				f.bOldOppCnt = s->bOppCnt;
				render       = TRUE;
			}

			// Check if assignment is idfferent....
			if (s->bAssignment != f.bOldAssignment)
			{
				f.bOldAssignment = s->bAssignment;
				render           = TRUE;
			}

			const INT8 bAPs  = s->bActionPoints;
			if (bAPs == 0 && f.bOldActionPoints >  0) render = TRUE;
			if (bAPs >  0 && f.bOldActionPoints == 0) render = TRUE;
			f.bOldActionPoints = bAPs;

			if (f.ubOldServiceCount != s->ubServiceCount)
			{
				f.ubOldServiceCount = s->ubServiceCount;
				render              = TRUE;
			}

			if (f.fOldCompatibleItems != f.fCompatibleItems || gfInItemPickupMenu || gpItemPointer != NULL)
			{
				f.fOldCompatibleItems = f.fCompatibleItems;
				render                = TRUE;
			}

			if (f.old_service_partner != s->service_partner)
			{
				f.old_service_partner = s->service_partner;
				render                = TRUE;
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

			if (fInterfacePanelDirty == DIRTYLEVEL2 && guiCurrentScreen == GAME_SCREEN)
				render = TRUE;

			if (render)
				RenderAutoFace(f);
		}

		BlinkAutoFace(f);
		MouthAutoFace(f);
	}
}


void HandleTalkingAutoFaces( )
{
	FOR_EACH_FACE(i)
	{
		HandleTalkingAutoFace(*i);
	}
}


static void FaceRestoreSavedBackgroundRect(FACETYPE const& f, INT16 const sDestLeft, INT16 const sDestTop, UINT16 const sSrcLeft, UINT16 const sSrcTop, UINT16 const sWidth, UINT16 const sHeight)
{
	SGPBox const r = { sSrcLeft, sSrcTop, sWidth, sHeight };
	BltVideoSurface(f.uiAutoDisplayBuffer, f.uiAutoRestoreBuffer, sDestLeft, sDestTop, &r);

	// Add rect to frame buffer queue
	if (f.uiAutoDisplayBuffer == FRAME_BUFFER)
	{
		InvalidateRegionEx(sDestLeft - 2, sDestTop - 2, sDestLeft + sWidth + 3, sDestTop + sHeight + 2);
	}
}


void SetFaceTalking(FACETYPE& f, const ST::string& zSoundFile, const ST::string& zTextString)
{
	// Set face to talking
	f.fTalking          = TRUE;
	f.fAnimatingTalking = TRUE;
	f.fFinishTalking    = FALSE;

	if ( !AreInMeanwhile( ) )
	{
		TurnOnSectorLocator(f.ubCharacterNum);
	}

	// Play sample
	if( gGameSettings.fOptions[ TOPTION_SPEECH ] )
		f.uiSoundID = PlayJA2GapSample(zSoundFile, HIGHVOLUME, 1, MIDDLEPAN, &f.GapList);
	else
		f.uiSoundID = SOUND_ERROR;

	if (f.uiSoundID != SOUND_ERROR)
	{
		f.fValidSpeech                    = TRUE;
		f.uiTalkingFromVeryBeginningTimer = GetJA2Clock();
	}
	else
	{
		f.fValidSpeech      = FALSE;
		f.uiTalkingTimer    = f.uiTalkingFromVeryBeginningTimer = GetJA2Clock();
		f.uiTalkingDuration = FindDelayForString(zTextString);
	}
}


void ExternSetFaceTalking(FACETYPE& f, UINT32 const sound_id)
{
	f.fTalking          = TRUE;
	f.fAnimatingTalking = TRUE;
	f.fFinishTalking    = FALSE;
	f.fValidSpeech      = TRUE;
	f.uiSoundID         = sound_id;
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
				FaceRestoreSavedBackgroundRect(*pFace, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight);
			}
			else
			{
				FaceRestoreSavedBackgroundRect(*pFace, pFace->usMouthX, pFace->usMouthY, pFace->usMouthOffsetX, pFace->usMouthOffsetY,  pFace->usMouthWidth, pFace->usMouthHeight);
			}
		}
		// OK, smart guy, make sure this guy has finished talking,
		// before attempting to end dialogue UI.
		pFace->fTalking = FALSE;

		// Call dialogue handler function
		HandleDialogueEnd(*pFace);

		pFace->fTalking = FALSE;
		pFace->fAnimatingTalking = FALSE;

		gfUIWaitingForUserSpeechAdvance = FALSE;
	}
}


void ShutupaYoFace(FACETYPE* const face)
{
	InternalShutupaYoFace(face, TRUE);
}


static void SetupFinalTalkingDelay(FACETYPE& f)
{
	f.fFinishTalking    = TRUE;
	f.fAnimatingTalking = FALSE;
	f.uiTalkingTimer    = GetJA2Clock();
	f.uiTalkingDuration = 300;
	f.sMouthFrame       = 0;

	// Close mouth!
	if (!f.fDisabled)
	{
		if (f.uiAutoRestoreBuffer == guiSAVEBUFFER)
		{
			FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthX, f.usMouthY, f.usMouthWidth, f.usMouthHeight);
		}
		else
		{
			FaceRestoreSavedBackgroundRect(f, f.usMouthX, f.usMouthY, f.usMouthOffsetX, f.usMouthOffsetY, f.usMouthWidth, f.usMouthHeight);
		}
	}

	// Setup flag to wait for advance (because we have no text!)
	if (gGameSettings.fOptions[TOPTION_KEY_ADVANCE_SPEECH] && f.uiFlags & FACE_POTENTIAL_KEYWAIT)
	{
		// Check if we have had valid speech!
		if (!f.fValidSpeech || gGameSettings.fOptions[TOPTION_SUBTITLES])
		{
			f.fFinishTalking = FALSE;
			// Set waiting for advance to true!
			gfUIWaitingForUserSpeechAdvance = TRUE;
		}
	}

	f.fValidSpeech = FALSE;
}


void LoadFacesGraphics()
{
	guiPORTRAITICONS = AddVideoObjectFromFile(INTERFACEDIR "/portraiticons.sti");
}


void DeleteFacesGraphics()
{
	DeleteVideoObject(guiPORTRAITICONS);
}
