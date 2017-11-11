#include <stdint.h>
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Debug.h"
#include "English.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameSettings.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Handle_UI.h"
#include "Interactive_Tiles.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Overhead.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "Render_Fun.h"
#include "RenderWorld.h"
#include "Rotting_Corpses.h"
#include "Shading.h"
#include "Soldier_Find.h"
#include "Sound_Control.h"
#include "Structure.h"
#include "SysUtil.h"
#include "Sys_Globals.h"
#include "TileDef.h"
#include "Tile_Cache.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "WCheck.h"
#include <math.h>
#include "UILayout.h"
#include "GameState.h"
#include "slog/slog.h"

UINT16* gpZBuffer = NULL;

static INT16 gsCurrentGlowFrame     = 0;
static INT16 gsCurrentItemGlowFrame = 0;


// VIEWPORT OFFSET VALUES
// NOTE: THESE VALUES MUST BE MULTIPLES OF TILE SIZES!
#define VIEWPORT_XOFFSET_S        (WORLD_TILE_X * 1)
#define VIEWPORT_YOFFSET_S        (WORLD_TILE_Y * 2)
#define LARGER_VIEWPORT_XOFFSET_S (VIEWPORT_XOFFSET_S * 3)
#define LARGER_VIEWPORT_YOFFSET_S (VIEWPORT_YOFFSET_S * 5)

enum RenderTilesFlags
{
	TILES_NONE                      = 0,
	TILES_DYNAMIC_CHECKFOR_INT_TILE = 0x00000400,
	TILES_DIRTY                     = 0x80000000,
	TILES_MARKED                    = 0x10000000,
	TILES_OBSCURED                  = 0x01000000
};
ENUM_BITSET(RenderTilesFlags)


#define MAX_RENDERED_ITEMS 2


// RENDERER FLAGS FOR DIFFERENT RENDER LEVELS
enum RenderLayerID
{
	RENDER_STATIC_LAND,
	RENDER_STATIC_OBJECTS,
	RENDER_STATIC_SHADOWS,
	RENDER_STATIC_STRUCTS,
	RENDER_STATIC_ROOF,
	RENDER_STATIC_ONROOF,
	RENDER_STATIC_TOPMOST,
	RENDER_DYNAMIC_LAND,
	RENDER_DYNAMIC_OBJECTS,
	RENDER_DYNAMIC_SHADOWS,
	RENDER_DYNAMIC_STRUCT_MERCS,
	RENDER_DYNAMIC_MERCS,
	RENDER_DYNAMIC_STRUCTS,
	RENDER_DYNAMIC_ROOF,
	RENDER_DYNAMIC_HIGHMERCS,
	RENDER_DYNAMIC_ONROOF,
	RENDER_DYNAMIC_TOPMOST,
	NUM_RENDER_FX_TYPES
};


#define NUM_ITEM_CYCLE_COLORS 20

static UINT16 us16BPPItemCycleWhiteColors[NUM_ITEM_CYCLE_COLORS];
static UINT16 us16BPPItemCycleRedColors[NUM_ITEM_CYCLE_COLORS];
static UINT16 us16BPPItemCycleYellowColors[NUM_ITEM_CYCLE_COLORS];


static INT16 gusNormalItemOutlineColor;
static INT16 gusYellowItemOutlineColor;

INT16   gsRenderHeight = 0;
BOOLEAN gfRenderFullThisFrame = 0;


UINT8   gubCurScrollSpeedID = 1;
BOOLEAN gfDoVideoScroll     = TRUE;
BOOLEAN gfScrollPending     = FALSE;

static RenderLayerFlags uiLayerUsedFlags         = TILES_LAYER_ALL;
static RenderLayerFlags uiAdditiveLayerUsedFlags = TILES_LAYER_ALL;


static const UINT8 gsGlowFrames[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 4, 6, 8, 9, 8, 6, 4, 2, 0
};


INT16 gsTopLeftWorldX;                                  /**< Top left corner of the visible portion of the map (in map coordiantes). */
INT16 gsTopLeftWorldY;                                  /**< Top left corner of the visible portion of the map (in map coordiantes). */
INT16 gsBottomRightWorldX;                              /**< Bottom right corner of the visible portion of the map (in map coordiantes). */
INT16 gsBottomRightWorldY;                              /**< Bottom right corner of the visible portion of the map (in map coordiantes). */
BOOLEAN gfIgnoreScrolling = FALSE;

BOOLEAN gfIgnoreScrollDueToCenterAdjust = FALSE;

// Map coordiante system
// ---------------------
//   Center of the map independently of its size is always (0, 1625).
//

// GLOBAL SCROLLING PARAMS
INT16 gCenterWorldX;
INT16 gCenterWorldY;
INT16 gsTLX;                            /**< Top left corner of the current map in screen coordinates. */
INT16 gsTLY;                            /**< Top left corner of the current map in screen coordinates. */
INT16 gsTRX;                            /**< Top right corner of the current map in screen coordinates. */
INT16 gsTRY;                            /**< Top right corner of the current map in screen coordinates. */
INT16 gsBLX;                            /**< Bottom left corner of the current map in screen coordinates. */
INT16 gsBLY;                            /**< Bottom left corner of the current map in screen coordinates. */
INT16 gsBRX;                            /**< Bottom right corner of the current map in screen coordinates. */
INT16 gsBRY;                            /**< Bottom right corner of the current map in screen coordinates. */
INT16 gsCX;                             /**< Center of the map in screen coordinates (seems to be always 0). */
INT16 gsCY;                             /**< Center of the map in screen coordinates (seems to be always 1625). */
double gdScaleX;
double gdScaleY;

#define FASTMAPROWCOLTOPOS(r, c) ((r) * WORLD_COLS + (c))


bool g_scroll_inertia = false;


// GLOBALS FOR CALCULATING STARTING PARAMETERS
static INT16 gsStartPointX_W;
static INT16 gsStartPointY_W;
static INT16 gsStartPointX_S;
static INT16 gsStartPointY_S;
static INT16 gsStartPointX_M;
static INT16 gsStartPointY_M;
static INT16 gsEndXS;
static INT16 gsEndYS;
// LARGER OFFSET VERSION FOR GIVEN LAYERS
static INT16 gsLStartPointX_W;
static INT16 gsLStartPointY_W;
static INT16 gsLStartPointX_S;
static INT16 gsLStartPointY_S;
static INT16 gsLStartPointX_M;
static INT16 gsLStartPointY_M;
static INT16 gsLEndXS;
static INT16 gsLEndYS;


INT16 gsScrollXIncrement;
INT16 gsScrollYIncrement;


// Rendering flags (full, partial, etc.)
static RenderFlags gRenderFlags = RENDER_FLAG_NONE;

#define gClippingRect (g_ui.m_wordlClippingRect)
static SGPRect gOldClipRect;
INT16   gsRenderCenterX;
INT16   gsRenderCenterY;
INT16   gsRenderWorldOffsetX = 0;
INT16   gsRenderWorldOffsetY = 10;


struct RenderFXType
{
	BOOLEAN fDynamic;
	BOOLEAN fZWrite;
	BOOLEAN fZBlitter;
	BOOLEAN fShadowBlitter;
	BOOLEAN fLinkedListDirection;
	BOOLEAN fMerc;
	BOOLEAN fCheckForRedundency;
	BOOLEAN fObscured;
};


static const RenderFXType RenderFX[] =
{
	{ FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,  FALSE }, // STATIC LAND
	{ FALSE, TRUE,  TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE }, // STATIC OBJECTS
	{ FALSE, TRUE,  TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE }, // STATIC SHADOWS
	{ FALSE, TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE, TRUE  }, // STATIC STRUCTS
	{ FALSE, TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }, // STATIC ROOF
	{ FALSE, TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE, TRUE  }, // STATIC ONROOF
	{ FALSE, TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }, // STATIC TOPMOST
	{ TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE, TRUE,  FALSE }, // DYNAMIC LAND
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE }, // DYNAMIC OBJECT
	{ TRUE,  FALSE, FALSE, TRUE,  TRUE,  FALSE, FALSE, FALSE }, // DYNAMIC SHADOW
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  TRUE,  FALSE, FALSE }, // DYNAMIC STRUCT MERCS
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  TRUE,  FALSE, FALSE }, // DYNAMIC MERCS
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }, // DYNAMIC STRUCT
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }, // DYNAMIC ROOF
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  TRUE,  FALSE, FALSE }, // DYNAMIC HIGHMERCS
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }, // DYNAMIC ONROOF
	{ TRUE,  FALSE, TRUE,  FALSE, TRUE,  FALSE, FALSE, FALSE }  // DYNAMIC TOPMOST
};


static const UINT8 RenderFXStartIndex[] =
{
	LAND_START_INDEX,    // STATIC LAND
	OBJECT_START_INDEX,  // STATIC OBJECTS
	SHADOW_START_INDEX,  // STATIC SHADOWS
	STRUCT_START_INDEX,  // STATIC STRUCTS
	ROOF_START_INDEX,    // STATIC ROOF
	ONROOF_START_INDEX,  // STATIC ONROOF
	TOPMOST_START_INDEX, // STATIC TOPMOST
	LAND_START_INDEX,    // DYNAMIC LAND
	OBJECT_START_INDEX,  // DYNAMIC OBJECT
	SHADOW_START_INDEX,  // DYNAMIC SHADOW
	MERC_START_INDEX,    // DYNAMIC STRUCT MERCS
	MERC_START_INDEX,    // DYNAMIC MERCS
	STRUCT_START_INDEX,  // DYNAMIC STRUCT
	ROOF_START_INDEX,    // DYNAMIC ROOF
	MERC_START_INDEX,    // DYNAMIC HIGHMERCS
	ONROOF_START_INDEX,  // DYNAMIC ONROOF
	TOPMOST_START_INDEX, // DYNAMIC TOPMOST
};


static RenderLayerFlags const g_render_fx_layer_flags[] =
{
	TILES_STATIC_LAND,
	TILES_STATIC_OBJECTS,
	TILES_STATIC_SHADOWS,
	TILES_STATIC_STRUCTURES,
	TILES_STATIC_ROOF,
	TILES_STATIC_ONROOF,
	TILES_STATIC_TOPMOST,
	TILES_DYNAMIC_LAND,
	TILES_DYNAMIC_OBJECTS,
	TILES_DYNAMIC_SHADOWS,
	TILES_DYNAMIC_STRUCT_MERCS,
	TILES_DYNAMIC_MERCS,
	TILES_DYNAMIC_STRUCTURES,
	TILES_DYNAMIC_ROOF,
	TILES_DYNAMIC_HIGHMERCS,
	TILES_DYNAMIC_ONROOF,
	TILES_DYNAMIC_TOPMOST
};


#ifdef _DEBUG

extern UINT8 gubFOVDebugInfoInfo[WORLD_MAX];
extern UINT8 gubGridNoMarkers[WORLD_MAX];
extern UINT8 gubGridNoValue;

extern BOOLEAN gfDisplayCoverValues;
static BOOLEAN gfDisplayGridNoVisibleValues = 0;
extern INT16   gsCoverValue[WORLD_MAX];
extern INT16   gsBestCover;

static void RenderFOVDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);
static void RenderCoverDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);
static void RenderGridNoVisibleDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);

#endif


static void ResetLayerOptimizing(void)
{
	uiLayerUsedFlags         = TILES_LAYER_ALL;
	uiAdditiveLayerUsedFlags = TILES_LAYER_NONE;
}


void ResetSpecificLayerOptimizing(RenderLayerFlags const uiRowFlag)
{
	uiLayerUsedFlags |= uiRowFlag;
}


static void SumAdditiveLayerOptimization()
{
	uiLayerUsedFlags = uiAdditiveLayerUsedFlags;
}


void SetRenderFlags(RenderFlags const uiFlags)
{
	gRenderFlags |= uiFlags;
}


void ClearRenderFlags(RenderFlags const uiFlags)
{
	gRenderFlags &= ~uiFlags;
}


void RenderSetShadows(BOOLEAN fShadows)
{
	if (fShadows)
	{
		gRenderFlags |= RENDER_FLAG_SHADOWS;
	}
	else
	{
		gRenderFlags &= ~RENDER_FLAG_SHADOWS;
	}
}


static inline INT16 GetMapXYWorldY(INT32 WorldCellX, INT32 WorldCellY)
{
	INT16 RDistToCenterX = WorldCellX * CELL_X_SIZE - gCenterWorldX;
	INT16 RDistToCenterY = WorldCellY * CELL_Y_SIZE - gCenterWorldY;
	INT16 RScreenCenterY = RDistToCenterX + RDistToCenterY;
	return RScreenCenterY + gsCY - gsTLY;
}


static void Blt8BPPDataTo16BPPBufferTransZIncClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion);
static void Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion);
static void Blt8BPPDataTo16BPPBufferTransZIncObscureClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion);
static void Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, INT16 sZIndex, const UINT16* p16BPPPalette);
static void Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, INT16 sZIndex, const UINT16* p16BPPPalette);


static void RenderTiles(RenderTilesFlags const uiFlags, INT32 const iStartPointX_M, INT32 const iStartPointY_M, INT32 const iStartPointX_S, INT32 const iStartPointY_S, INT32 const iEndXS, INT32 const iEndYS, UINT8 const ubNumLevels, RenderLayerID const* const psLevelIDs)
{
	static UINT8        ubLevelNodeStartIndex[NUM_RENDER_FX_TYPES];
	static RenderFXType RenderFXList[NUM_RENDER_FX_TYPES];

	HVOBJECT hVObject = NULL; // XXX HACK000E
	BOOLEAN fPixelate = FALSE;
	INT16 sMultiTransShadowZBlitterIndex = -1;

	INT16 sZOffsetX = -1;
	INT16 sZOffsetY = -1;
	const ROTTING_CORPSE* pCorpse = NULL;
	UINT32 uiTileElemFlags = 0;

	UINT16          usImageIndex = 0;
	INT16           sZLevel      = 0;
	BackgroundFlags uiDirtyFlags = BGND_FLAG_NONE;
	UINT16 const*   pShadeTable  = 0;

	INT32 iAnchorPosX_M = iStartPointX_M;
	INT32 iAnchorPosY_M = iStartPointY_M;
	INT32 iAnchorPosX_S = iStartPointX_S;
	INT32 iAnchorPosY_S = iStartPointY_S;

	UINT32                uiDestPitchBYTES = 0;
	UINT16*               pDestBuf         = 0;
	SGPVSurface::Lockable lock;
	if  (!(uiFlags & TILES_DIRTY))
	{
		lock.Lock(FRAME_BUFFER);
		pDestBuf         = lock.Buffer<UINT16>();
		uiDestPitchBYTES = lock.Pitch();
	}

	bool check_for_mouse_detections = false;
	if (uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE &&
			ShouldCheckForMouseDetections())
	{
		BeginCurInteractiveTileCheck();
		// If we are in edit mode, don't do this
		check_for_mouse_detections = !gfEditMode;
	}

	for (UINT32 i = 0; i < ubNumLevels; i++)
	{
		ubLevelNodeStartIndex[i] = RenderFXStartIndex[psLevelIDs[i]];
		RenderFXList[i]          = RenderFX[psLevelIDs[i]];
	}

	INT8 bXOddFlag = 0;
	do
	{
		static INT32 iTileMapPos[500];

		{
			INT32 iTempPosX_M = iAnchorPosX_M;
			INT32 iTempPosY_M = iAnchorPosY_M;
			INT32 iTempPosX_S = iAnchorPosX_S;
			UINT32 uiMapPosIndex = 0;

			// Build tile index list
			do
			{
				iTileMapPos[uiMapPosIndex] = FASTMAPROWCOLTOPOS(iTempPosY_M, iTempPosX_M);

				iTempPosX_S += 40;
				iTempPosX_M++;
				iTempPosY_M--;

				uiMapPosIndex++;
			}
			while (iTempPosX_S < iEndXS);
		}

		for (UINT32 cnt = 0; cnt < ubNumLevels; cnt++)
		{
			RenderLayerFlags const uiRowFlags = g_render_fx_layer_flags[psLevelIDs[cnt]];

			if (uiRowFlags & TILES_ALL_DYNAMICS && !(uiLayerUsedFlags & uiRowFlags) && !(uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE)) continue;

			INT32 iTempPosX_M = iAnchorPosX_M;
			INT32 iTempPosY_M = iAnchorPosY_M;
			INT32 iTempPosX_S = iAnchorPosX_S;
			INT32 iTempPosY_S = iAnchorPosY_S;
			UINT32 uiMapPosIndex = 0;

			if (bXOddFlag) iTempPosX_S += 20;

			do
			{
				const UINT32 uiTileIndex = iTileMapPos[uiMapPosIndex];
				uiMapPosIndex++;

				if (uiTileIndex < GRIDSIZE)
				{
					MAP_ELEMENT const& me = gpWorldLevelData[uiTileIndex];

					/* OK, we're searching through this loop anyway, might as well check
					 * for mouse position over objects. Experimental! */
					if (check_for_mouse_detections && me.pStructHead)
					{
						LogMouseOverInteractiveTile(uiTileIndex);
					}

					if (uiFlags & TILES_MARKED && !(me.uiFlags & MAPELEMENT_REDRAW)) goto next_tile;

					INT8             n_visible_items = 0;
					ITEM_POOL const* item_pool       = 0;
					for (LEVELNODE* pNode = me.pLevelNodes[ubLevelNodeStartIndex[cnt]]; pNode;)
					{
						const RenderFXType RenderingFX = RenderFXList[cnt];
						const BOOLEAN fObscured            = RenderingFX.fObscured;
						const BOOLEAN fDynamic             = RenderingFX.fDynamic;
						BOOLEAN       fMerc                = RenderingFX.fMerc;
						BOOLEAN       fZWrite              = RenderingFX.fZWrite;
						BOOLEAN       fZBlitter            = RenderingFX.fZBlitter;
						BOOLEAN       fShadowBlitter       = RenderingFX.fShadowBlitter;
						const BOOLEAN fLinkedListDirection = RenderingFX.fLinkedListDirection;
						const BOOLEAN fCheckForRedundency  = RenderingFX.fCheckForRedundency;

						BOOLEAN fMultiZBlitter            = FALSE;
						BOOLEAN fIntensityBlitter         = FALSE;
						BOOLEAN fSaveZ                    = FALSE;
						BOOLEAN fWallTile                 = FALSE;
						BOOLEAN fMultiTransShadowZBlitter = FALSE;
						BOOLEAN fObscuredBlitter          = FALSE;
						UINT32 uiAniTileFlags = 0;
						INT16 gsForceSoldierZLevel = 0;

						const UINT32 uiLevelNodeFlags = pNode->uiFlags;

						if (fCheckForRedundency                              &&
								me.uiFlags & MAPELEMENT_REDUNDENT                &&
								!(me.uiFlags & MAPELEMENT_REEVALUATE_REDUNDENCY) && // If we donot want to re-evaluate first
								!(gTacticalStatus.uiFlags & NOHIDE_REDUNDENCY))
						{
							break;
						}

						// Force z-buffer blitting for marked tiles (even ground!)
						if (uiFlags & TILES_MARKED) fZBlitter = TRUE;

						//Looking up height every time here is alot better than doing it above!
						INT16 const sTileHeight = me.sHeight;

						INT16 sModifiedTileHeight = (sTileHeight / 80 - 1) * 80;
						if (sModifiedTileHeight < 0) sModifiedTileHeight = 0;

						BOOLEAN fRenderTile = TRUE;
						if (!(uiLevelNodeFlags & LEVELNODE_REVEAL))
						{
							fPixelate = FALSE;
						}
						else if (fDynamic)
						{
							fPixelate = TRUE;
						}
						else
						{
							fRenderTile = FALSE;
						}

						// non-type specific setup
						INT16 sXPos = iTempPosX_S;
						INT16 sYPos = iTempPosY_S;

						TILE_ELEMENT const* TileElem  = 0;
						// setup for any tile type except mercs
						if (!fMerc)
						{
							if (uiLevelNodeFlags & (LEVELNODE_ROTTINGCORPSE | LEVELNODE_CACHEDANITILE))
							{
								if (fDynamic)
								{
									if (!(uiLevelNodeFlags & LEVELNODE_DYNAMIC) && !(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC))
									{
										fRenderTile = FALSE;
									}
								}
								else if (uiLevelNodeFlags & LEVELNODE_DYNAMIC)
								{
									fRenderTile = FALSE;
								}
							}
							else
							{
								TileElem =
									uiLevelNodeFlags & LEVELNODE_REVEALTREES ? &gTileDatabase[pNode->usIndex + 2] :
									&gTileDatabase[pNode->usIndex];

								// Handle independent-per-tile animations (i.e.: doors, exploding things, etc.)
								if (fDynamic && uiLevelNodeFlags & LEVELNODE_ANIMATION && pNode->sCurrentFrame != -1)
								{
									Assert(TileElem->pAnimData);
									TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
								}

								// Set Tile elem flags here!
								uiTileElemFlags = TileElem->uiFlags;

								if (!fPixelate)
								{
									if (fDynamic)
									{
										if (!(uiLevelNodeFlags & LEVELNODE_DYNAMIC) && !(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC) && !(uiTileElemFlags & DYNAMIC_TILE))
										{
											if (uiTileElemFlags & ANIMATED_TILE)
											{
												Assert(TileElem->pAnimData);
												TileElem        = &gTileDatabase[TileElem->pAnimData->pusFrames[TileElem->pAnimData->bCurrentFrame]];
												uiTileElemFlags = TileElem->uiFlags;
											}
											else
											{
												fRenderTile = FALSE;
											}
										}
									}
									else
									{
										if (uiTileElemFlags & ANIMATED_TILE ||
												((uiTileElemFlags & DYNAMIC_TILE || uiLevelNodeFlags & LEVELNODE_DYNAMIC) && !(uiFlags & TILES_OBSCURED)))
										{
											fRenderTile = FALSE;
										}
									}
								}
							}

							// OK, ATE, CHECK FOR AN OBSCURED TILE AND MAKE SURE IF LEVELNODE IS SET
							// WE DON'T RENDER UNLESS WE HAVE THE RENDER FLAG SET!
							if (fObscured)
							{
								if (uiFlags & TILES_OBSCURED)
								{
									if (uiLevelNodeFlags & LEVELNODE_SHOW_THROUGH)
									{
										fObscuredBlitter = TRUE;
									}
									else
									{
										// Do not render if we are not on this render loop!
										fRenderTile = FALSE;
									}
								}
								else
								{
									if (uiLevelNodeFlags & LEVELNODE_SHOW_THROUGH)
									{
										fRenderTile = FALSE;
									}
								}
							}

							if (fRenderTile)
							{
								// Set flag to set layer as used
								if (fDynamic || fPixelate)
								{
									uiAdditiveLayerUsedFlags |= uiRowFlags;
								}

								if (uiLevelNodeFlags & LEVELNODE_DYNAMICZ)
								{
									fSaveZ  = TRUE;
									fZWrite = TRUE;
								}

								if (uiLevelNodeFlags & LEVELNODE_CACHEDANITILE)
								{
									ANITILE const& a = *pNode->pAniTile;
									hVObject         = gpTileCache[a.sCachedTileID].pImagery->vo;
									usImageIndex     = a.sCurrentFrame;
									uiAniTileFlags   = a.uiFlags;

									float dOffsetX;
									float dOffsetY;
									// Position corpse based on it's float position
									if (uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)
									{
										pCorpse     = ID2CORPSE(a.v.user.uiData);
										pShadeTable = pCorpse->pShades[pNode->ubShadeLevel];

										// OK, if this is a corpse.... stop if not visible
										if (pCorpse->def.bVisible != 1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) goto next_prev_node;

										INT16 x;
										INT16 y;
										ConvertGridNoToCenterCellXY(pCorpse->def.sGridNo, &x, &y);
										dOffsetX = x - gsRenderCenterX;
										dOffsetY = y - gsRenderCenterY;
									}
									else
									{
										dOffsetX = a.sRelativeX - gsRenderCenterX;
										dOffsetY = a.sRelativeY - gsRenderCenterY;
									}

									// Calculate guy's position
									float dTempX_S;
									float dTempY_S;
									FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

									sXPos = g_ui.m_tacticalMapCenterX + (INT16)dTempX_S;
									sYPos = g_ui.m_tacticalMapCenterY + (INT16)dTempY_S - sTileHeight;

									// Adjust for offset position on screen
									sXPos -= gsRenderWorldOffsetX;
									sYPos -= gsRenderWorldOffsetY;
								}
								else
								{
									hVObject     = TileElem->hTileSurface;
									usImageIndex = TileElem->usRegionIndex;

									if (TileElem->uiFlags & IGNORE_WORLD_HEIGHT)
									{
										sYPos -= sModifiedTileHeight;
									}
									else if (!(uiLevelNodeFlags & LEVELNODE_IGNOREHEIGHT))
									{
										sYPos -= sTileHeight;
									}

									if (!(uiFlags & TILES_DIRTY))
									{
										hVObject->CurrentShade(pNode->ubShadeLevel);
									}
								}


								//ADJUST FOR RELATIVE OFFSETS
								if (uiLevelNodeFlags & LEVELNODE_USERELPOS)
								{
									sXPos += pNode->sRelativeX;
									sYPos += pNode->sRelativeY;
								}

								if (uiLevelNodeFlags & LEVELNODE_USEZ)
								{
									sYPos -= pNode->sRelativeZ;
								}

								//ADJUST FOR ABSOLUTE POSITIONING
								if (uiLevelNodeFlags & LEVELNODE_USEABSOLUTEPOS)
								{
									float dOffsetX = pNode->sRelativeX - gsRenderCenterX;
									float dOffsetY = pNode->sRelativeY - gsRenderCenterY;

									// OK, DONT'T ASK... CONVERSION TO PROPER Y NEEDS THIS...
									dOffsetX -= CELL_Y_SIZE;

									float dTempX_S;
									float dTempY_S;
									FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

									sXPos = g_ui.m_tacticalMapCenterX + (INT16)dTempX_S;
									sYPos = g_ui.m_tacticalMapCenterY + (INT16)dTempY_S;

									// Adjust for offset position on screen
									sXPos -= gsRenderWorldOffsetX;
									sYPos -= gsRenderWorldOffsetY;

									sYPos -= pNode->sRelativeZ;
								}
							}

							// COUNT # OF ITEMS AT THIS LOCATION
							if (uiLevelNodeFlags & LEVELNODE_ITEM)
							{
								// Set item pool for this location.
								item_pool = item_pool ? item_pool->pNext : pNode->pItemPool;
								WORLDITEM const& wi = GetWorldItem(item_pool->iItemIndex);

								/* Limit rendering of items to MAX_RENDERED_ITEMS. Do not render
								 * hidden items either. */
								if (wi.bVisible != VISIBLE             ||
										wi.usFlags & WORLD_ITEM_DONTRENDER ||
										n_visible_items == MAX_RENDERED_ITEMS)
								{
									if (!(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS)) goto next_prev_node;
								}
								++n_visible_items;

								if (wi.bRenderZHeightAboveLevel > 0)
								{
									sYPos -= wi.bRenderZHeightAboveLevel;
								}
							}

							// If render tile is false...
							if (!fRenderTile) goto next_prev_node;
						}

						// specific code for node types on a per-tile basis
						switch (uiRowFlags)
						{
							case TILES_STATIC_LAND:
								goto zlevel_land;

							case TILES_STATIC_OBJECTS:
								// ATE: Modified to use constant z level, as these are same level as land items
								goto zlevel_objects;

							case TILES_STATIC_SHADOWS:
								if (uiLevelNodeFlags & LEVELNODE_EXITGRID)
								{
									fIntensityBlitter = TRUE;
									fShadowBlitter    = FALSE;
								}
								goto zlevel_shadows;

							case TILES_STATIC_STRUCTURES:
								if (TileElem)
								{
									if (TileElem->uiFlags & MULTI_Z_TILE) fMultiZBlitter = TRUE;
									if (TileElem->uiFlags & WALL_TILE)    fWallTile      = TRUE;
								}
								goto zlevel_structures;

							case TILES_STATIC_ROOF:
								// ATE: Added for shadows on roofs
								if (TileElem && TileElem->uiFlags & ROOFSHADOW_TILE)
								{
									fShadowBlitter = TRUE;
								}
								goto zlevel_roof;

							case TILES_STATIC_ONROOF:
								goto zlevel_onroof;

							case TILES_STATIC_TOPMOST:
								goto zlevel_topmost;

							case TILES_DYNAMIC_LAND:
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_land:
								sZLevel = LAND_Z_LEVEL;
								break;

							case TILES_DYNAMIC_OBJECTS:
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_objects:
								if (uiTileElemFlags & CLIFFHANG_TILE)
								{
									sZLevel = LAND_Z_LEVEL;
								}
								else if (uiTileElemFlags & OBJECTLAYER_USEZHEIGHT)
								{
									INT16 const world_y = GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
									sZLevel = (world_y * Z_SUBLAYERS) + LAND_Z_LEVEL;
								}
								else
								{
									sZLevel = OBJECT_Z_LEVEL;
								}
								break;

							case TILES_DYNAMIC_SHADOWS:
							{
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_shadows:
								INT16 const world_y = GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
								sZLevel = __max(((world_y - 80) * Z_SUBLAYERS) + SHADOW_Z_LEVEL, 0);
								break;
							}

							case TILES_DYNAMIC_STRUCTURES:
							{
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_structures:
								INT16 world_y = GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
								if (uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)
								{
									if (pCorpse->def.usFlags & ROTTING_CORPSE_VEHICLE)
									{
										if (pNode->pStructureData)
										{
											DB_STRUCTURE const& dbs = *pNode->pStructureData->pDBStructureRef->pDBStructure;
											sZOffsetX = dbs.bZTileOffsetX;
											sZOffsetY = dbs.bZTileOffsetY;
										}
										world_y = GetMapXYWorldY(iTempPosX_M + sZOffsetX, iTempPosY_M + sZOffsetY);
										sZLevel = STRUCT_Z_LEVEL;
									}
									else
									{
										sZOffsetX = -1;
										sZOffsetY = -1;
										world_y   = GetMapXYWorldY(iTempPosX_M + sZOffsetX, iTempPosY_M + sZOffsetY);
										world_y  += 20;
										sZLevel   = LAND_Z_LEVEL;
									}
								}
								else if (uiLevelNodeFlags & LEVELNODE_PHYSICSOBJECT)
								{
									world_y += pNode->sRelativeZ;
									sZLevel  = ONROOF_Z_LEVEL;
								}
								else if (uiLevelNodeFlags & LEVELNODE_ITEM)
								{
									WORLDITEM const& wi = GetWorldItem(pNode->pItemPool->iItemIndex);
									if (wi.bRenderZHeightAboveLevel > 0)
									{
										sZLevel  = STRUCT_Z_LEVEL + wi.bRenderZHeightAboveLevel;
									}
									else
									{
										sZLevel = OBJECT_Z_LEVEL;
									}
								}
								else if (uiAniTileFlags & ANITILE_SMOKE_EFFECT)
								{
									sZLevel = OBJECT_Z_LEVEL;
								}
								else if (uiLevelNodeFlags & LEVELNODE_USEZ)
								{
									if (uiLevelNodeFlags & LEVELNODE_NOZBLITTER)
									{
										world_y += 40;
									}
									else
									{
										world_y += pNode->sRelativeZ;
									}
									sZLevel = ONROOF_Z_LEVEL;
								}
								else
								{
									sZLevel = STRUCT_Z_LEVEL;
								}
								sZLevel += world_y * Z_SUBLAYERS;
								break;
							}

							case TILES_DYNAMIC_ROOF:
							{
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_roof:
								// Automatically adjust height.
								sYPos -= WALL_HEIGHT;

								INT16 const world_y = WALL_HEIGHT + GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
								sZLevel = (world_y * Z_SUBLAYERS) + ROOF_Z_LEVEL;
								break;
							}

							case TILES_DYNAMIC_ONROOF:
							{
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_onroof:
								// Automatically adjust height.
								sYPos -= WALL_HEIGHT;

								INT16 world_y = GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
								if (uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)
								{
									world_y += WALL_HEIGHT + 40;
								}
								if (uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)
								{ // XXX duplicate?
									world_y += WALL_HEIGHT + 40;
								}
								else
								{
									world_y += WALL_HEIGHT;
								}
								sZLevel = (world_y * Z_SUBLAYERS) + ONROOF_Z_LEVEL;
								break;
							}

							case TILES_DYNAMIC_TOPMOST:
								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
zlevel_topmost:
								sZLevel = TOPMOST_Z_LEVEL;
								break;

							case TILES_DYNAMIC_MERCS:
							case TILES_DYNAMIC_HIGHMERCS:
							case TILES_DYNAMIC_STRUCT_MERCS:
							{
								// Set flag to set layer as used
								uiAdditiveLayerUsedFlags |= uiRowFlags;

								SOLDIERTYPE const& s = *pNode->pSoldier;
								switch (uiRowFlags)
								{
									case TILES_DYNAMIC_MERCS:
										// If we are multi-tiled, ignore here
										if (s.uiStatusFlags & SOLDIER_MULTITILE) goto next_node;
										// If we are at a higher level, no not do anything unless we are at the highmerc stage
										if (s.bLevel > 0) goto next_node;
										break;

									case TILES_DYNAMIC_HIGHMERCS:
										// If we are multi-tiled, ignore here
										if (s.uiStatusFlags & SOLDIER_MULTITILE) goto next_node;
										// If we are at a lower level, no not do anything unless we are at the highmerc stage
										if (s.bLevel == 0) goto next_node;
										break;

									case TILES_DYNAMIC_STRUCT_MERCS:
										// If we are not multi-tiled, ignore here
										if (!(s.uiStatusFlags & SOLDIER_MULTITILE))
										{
											// If we are at a low level, no not do anything unless we are at the merc stage
											if (s.bLevel == 0) goto next_node;
										}
										else
										{
											fSaveZ                    = TRUE;
											fMultiTransShadowZBlitter = TRUE;
											fZBlitter                 = TRUE;

											// ATE: Use one direction for queen!
											sMultiTransShadowZBlitterIndex =
												s.ubBodyType == QUEENMONSTER ? 0 :
												OneCDirection(s.bDirection);
										}
										break;
									default:
										break;
								}

								// IF we are not active, or are a placeholder for multi-tile animations do nothing
								if (!s.bActive || uiLevelNodeFlags & LEVELNODE_MERCPLACEHOLDER) goto next_node;

								// Skip if we cannot see the guy!
								if (s.bLastRenderVisibleValue == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) goto next_node;

								// Get animation surface....
								UINT16 const usAnimSurface = GetSoldierAnimationSurface(&s);
								if (usAnimSurface == INVALID_ANIMATION_SURFACE) goto next_node;

								// Shade guy always lighter than sceane default!
								UINT8 ubShadeLevel;
								if (s.fBeginFade)
								{
									ubShadeLevel = s.ubFadeLevel;
								}
								else
								{
									ubShadeLevel  = pNode->ubShadeLevel & 0x0f;
									ubShadeLevel  = __max(ubShadeLevel - 2, DEFAULT_SHADE_LEVEL);
									ubShadeLevel |= pNode->ubShadeLevel & 0x30;
								}
								pShadeTable = s.pShades[ubShadeLevel];

								// Position guy based on guy's position
								float const dOffsetX = s.dXPos - gsRenderCenterX;
								float const dOffsetY = s.dYPos - gsRenderCenterY;

								// Calculate guy's position
								float dTempX_S;
								float dTempY_S;
								FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

								sXPos = g_ui.m_tacticalMapCenterX + (INT16)dTempX_S;
								sYPos = g_ui.m_tacticalMapCenterY + (INT16)dTempY_S - sTileHeight;

								// Adjust for offset position on screen
								sXPos -= gsRenderWorldOffsetX;
								sYPos -= gsRenderWorldOffsetY;

								// Adjust for soldier height
								sYPos -= s.sHeightAdjustment;

								// Handle shade stuff....
								if (!s.fBeginFade)
								{
									// Special effect - draw ghost if is seen by a guy in player's team but not current guy
									// ATE: Todo: setup flag for 'bad-guy' - can releive some checks in renderer
									if (!s.bNeutral && s.bSide != OUR_TEAM)
									{
										INT8 bGlowShadeOffset = 0;

										if (gTacticalStatus.ubCurrentTeam == OUR_TEAM)
										{
											// Shade differently depending on visiblity
											if (s.bLastRenderVisibleValue == 0)
											{
												bGlowShadeOffset = 10;
											}

											const SOLDIERTYPE* const sel = GetSelectedMan();
											if (sel                                       &&
													sel->bOppList[s.ubID] != SEEN_CURRENTLY   &&
													s.usAnimState         != CHARIOTS_OF_FIRE &&
													s.usAnimState         != BODYEXPLODING)
											{
												bGlowShadeOffset = 10;
											}
										}

										UINT16* const* pShadeStart =
											s.bLevel == 0 ? &s.pGlowShades[0] : &s.pShades[20];

										// Set shade
										// If a bad guy is highlighted
										if (gSelectedGuy != NULL && gSelectedGuy->bSide != OUR_TEAM)
										{
											if (gSelectedGuy == &s)
											{
												pShadeTable = pShadeStart[gsGlowFrames[gsCurrentGlowFrame] + bGlowShadeOffset];
												gsForceSoldierZLevel = TOPMOST_Z_LEVEL;
											}
											else
											{
												// Are we dealing with a not-so visible merc?
												if (bGlowShadeOffset == 10)
												{
													pShadeTable = s.effect_shade;
												}
											}
										}
										else
										{
											// Not highlighted, but maybe we are in enemy's turn and they have the baton
											if (gTacticalStatus.ubCurrentTeam == OUR_TEAM ||
													s.uiStatusFlags & SOLDIER_UNDERAICONTROL) // Does he have baton?
											{
												pShadeTable = pShadeStart[gsGlowFrames[gsCurrentGlowFrame] + bGlowShadeOffset];
												if (gsGlowFrames[gsCurrentGlowFrame] >= 7)
												{
													gsForceSoldierZLevel = TOPMOST_Z_LEVEL;
												}
											}
										}
									}
								}

								{ // Calculate Z level
									INT16 world_y;
									if (s.uiStatusFlags & SOLDIER_MULTITILE)
									{
										if (pNode->pStructureData)
										{
											DB_STRUCTURE const& dbs = *pNode->pStructureData->pDBStructureRef->pDBStructure;
											sZOffsetX = dbs.bZTileOffsetX;
											sZOffsetY = dbs.bZTileOffsetY;
										}
										world_y = GetMapXYWorldY(iTempPosX_M + sZOffsetX, iTempPosY_M + sZOffsetY);
									}
									else
									{
										world_y = GetMapXYWorldY(iTempPosX_M, iTempPosY_M);
									}

									if (s.uiStatusFlags & SOLDIER_VEHICLE)
									{
										sZLevel = (world_y * Z_SUBLAYERS) + STRUCT_Z_LEVEL;
									}
									else if (gsForceSoldierZLevel != 0)
									{
										sZLevel = gsForceSoldierZLevel;
									}
									else if (s.sZLevelOverride != -1)
									{
										sZLevel = s.sZLevelOverride;
									}
									else if (s.dHeightAdjustment > 0)
									{
										world_y += WALL_HEIGHT + 20;
										sZLevel = (world_y * Z_SUBLAYERS) + ONROOF_Z_LEVEL;
									}
									else
									{
										sZLevel = (world_y * Z_SUBLAYERS) + MERC_Z_LEVEL;
									}
								}

								if (!(uiFlags & TILES_DIRTY) && s.fForceShade)
								{
									pShadeTable = s.pForcedShade;
								}

								hVObject = gAnimSurfaceDatabase[usAnimSurface].hVideoObject;
								if (!hVObject) goto next_node;

								// ATE: If we are in a gridno that we should not use obscure blitter, set!
								if (!(me.ubExtFlags[0] & MAPELEMENT_EXT_NOBURN_STRUCT))
								{
									fObscuredBlitter = TRUE;
								}
								else
								{
									// ATE: Artificially increase z-level...
									sZLevel += 2;
								}

								usImageIndex = s.usAniFrame;

								uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
								break;
							}
							default:
								break;
						}

						// Adjust for interface level
						sYPos += gsRenderHeight;

						if (!fRenderTile) goto next_prev_node;

						if (uiLevelNodeFlags & LEVELNODE_HIDDEN &&
								/* If it is a roof and SHOW_ALL_ROOFS is on, turn off hidden tile check */
								(!TileElem || !(TileElem->uiFlags & ROOF_TILE) || !(gTacticalStatus.uiFlags & SHOW_ALL_ROOFS)))
							goto next_prev_node;

						if (uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)
						{
							// Set fmerc flag!
							fMerc = TRUE;
							fZWrite = TRUE;

							sMultiTransShadowZBlitterIndex = GetCorpseStructIndex(&pCorpse->def, TRUE);
							fMultiTransShadowZBlitter      = TRUE;
						}

						if (uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC && !(uiFlags & TILES_DIRTY))
						{
							// Remove flags!
							pNode->uiFlags &= ~LEVELNODE_LASTDYNAMIC;
							fZWrite = TRUE;
						}

						// RENDER
						if (uiLevelNodeFlags & LEVELNODE_WIREFRAME &&
								!gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME])
						{
						}
						else if (uiFlags & TILES_DIRTY)
						{
							if (!(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC))
							{
								ETRLEObject const& pTrav = hVObject->SubregionProperties(usImageIndex);
								UINT32 const uiBrushHeight = pTrav.usHeight;
								UINT32 const uiBrushWidth  = pTrav.usWidth;
								sXPos += pTrav.sOffsetX;
								sYPos += pTrav.sOffsetY;

								INT16 const h = MIN((INT16)uiBrushHeight, gsVIEWPORT_WINDOW_END_Y - sYPos);
								RegisterBackgroundRect(uiDirtyFlags, sXPos, sYPos, uiBrushWidth, h);
								if (fSaveZ)
								{
									RegisterBackgroundRect(uiDirtyFlags | BGND_FLAG_SAVE_Z, sXPos, sYPos, uiBrushWidth, h);
								}
							}
						}
						else if (uiLevelNodeFlags & LEVELNODE_DISPLAY_AP)
						{
							ETRLEObject const& pTrav = hVObject->SubregionProperties(usImageIndex);
							sXPos += pTrav.sOffsetX;
							sYPos += pTrav.sOffsetY;

							UINT8 const foreground = gfUIDisplayActionPointsBlack ? FONT_MCOLOR_BLACK : FONT_MCOLOR_WHITE;
							SetFontAttributes(TINYFONT1, foreground);
							SetFontDestBuffer(guiSAVEBUFFER, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y);
							wchar_t buf[16];
							swprintf(buf, lengthof(buf), L"%d", pNode->uiAPCost);
							INT16 sX;
							INT16 sY;
							FindFontCenterCoordinates(sXPos, sYPos, 1, 1, buf, TINYFONT1, &sX, &sY);
							MPrintBuffer(pDestBuf, uiDestPitchBYTES, sX, sY, buf);
							SetFontDestBuffer(FRAME_BUFFER);
						}
						else if (uiLevelNodeFlags & LEVELNODE_ITEM)
						{
							UINT16     outline_colour;
							bool const on_roof = uiRowFlags == TILES_STATIC_ONROOF || uiRowFlags == TILES_DYNAMIC_ONROOF;
							if (gGameSettings.fOptions[TOPTION_GLOW_ITEMS])
							{
								UINT16 const *palette =
									on_roof                                    ? us16BPPItemCycleYellowColors :
									gTacticalStatus.uiFlags & RED_ITEM_GLOW_ON ? us16BPPItemCycleRedColors    :
									us16BPPItemCycleWhiteColors;
								outline_colour = palette[gsCurrentItemGlowFrame];
							}
							else
							{
								outline_colour =
									on_roof ? gusYellowItemOutlineColor :
									gusNormalItemOutlineColor;
							}

							const BOOLEAN bBlitClipVal = BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
							if (bBlitClipVal == FALSE)
							{
								if (fObscuredBlitter)
								{
									Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, outline_colour);
								}
								else
								{
									Blt8BPPDataTo16BPPBufferOutlineZ(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, outline_colour);
								}
							}
							else if (bBlitClipVal == TRUE)
							{
								if (fObscuredBlitter)
								{
									Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, outline_colour, &gClippingRect);
								}
								else
								{
									Blt8BPPDataTo16BPPBufferOutlineZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, outline_colour, &gClippingRect);
								}
							}
						}
						// ATE: Check here for a lot of conditions!
						else if (uiLevelNodeFlags & LEVELNODE_PHYSICSOBJECT)
						{
							const BOOLEAN bBlitClipVal = BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);

							if (fShadowBlitter)
							{
								if (bBlitClipVal == FALSE)
								{
									Blt8BPPDataTo16BPPBufferShadowZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
								}
								else
								{
									Blt8BPPDataTo16BPPBufferShadowZNBClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
								}
							}
							else
							{
								if (bBlitClipVal == FALSE)
								{
									Blt8BPPDataTo16BPPBufferOutlineZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
								}
								else if (bBlitClipVal == TRUE)
								{
									Blt8BPPDataTo16BPPBufferOutlineClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, SGP_TRANSPARENT, &gClippingRect);
								}
							}
						}
						else
						{
							if (fMultiTransShadowZBlitter)
							{
								if (fZBlitter)
								{
									if (fObscuredBlitter)
									{
										Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, sMultiTransShadowZBlitterIndex, pShadeTable);
									}
									else
									{
										Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, sMultiTransShadowZBlitterIndex, pShadeTable);
									}
								}
							}
							else if (fMultiZBlitter)
							{
								if (fZBlitter)
								{
									if (fObscuredBlitter)
									{
										Blt8BPPDataTo16BPPBufferTransZIncObscureClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
									}
									else
									{
										if (fWallTile)
										{
											Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransZIncClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
										}
									}
								}
								else
								{
									Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
								}
							}
							else
							{
								const BOOLEAN bBlitClipVal = BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
								if (bBlitClipVal == TRUE)
								{
									if (fPixelate)
									{
										Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
									}
									else if (fMerc)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferTransShadowZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);
											}
											else
											{
												if (fObscuredBlitter)
												{
													Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);
												}
												else
												{
													Blt8BPPDataTo16BPPBufferTransShadowZNBClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);
												}
											}

											if (uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)
											{
												SGPVSurface::Lock l(guiSAVEBUFFER);

												// BLIT HERE
												Blt8BPPDataTo16BPPBufferTransShadowClip(l.Buffer<UINT16>(), l.Pitch(), hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);

												// Turn it off!
												pNode->uiFlags &= ~LEVELNODE_UPDATESAVEBUFFERONCE;
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransShadowClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);
										}
									}
									else if (fShadowBlitter)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferShadowZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferShadowZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferShadowClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
										}
									}
									else if (fIntensityBlitter)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferIntensityZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferIntensityZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferIntensityClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
										}
									}
									else if (fZBlitter)
									{
										if (fZWrite)
										{
											if (fObscuredBlitter)
											{
												Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferTransZClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransZNBClip(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
										}

										if (uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)
										{
											SGPVSurface::Lock l(guiSAVEBUFFER);

											// BLIT HERE
											Blt8BPPDataTo16BPPBufferTransZClip(l.Buffer<UINT16>(), l.Pitch(), gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);

											// Turn it off!
											pNode->uiFlags &= ~LEVELNODE_UPDATESAVEBUFFERONCE;
										}
									}
									else
									{
										Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
									}
								}
								else if (bBlitClipVal == FALSE)
								{
									if (fPixelate)
									{
										if (fZWrite)
										{
											Blt8BPPDataTo16BPPBufferTransZTranslucent(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransZNBTranslucent(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
										}
									}
									else if (fMerc)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferTransShadowZ(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
											}
											else
											{
												if (fObscuredBlitter)
												{
													Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
												}
												else
												{
													Blt8BPPDataTo16BPPBufferTransShadowZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
												}
											}

											if (uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)
											{
												SGPVSurface::Lock l(guiSAVEBUFFER);

												// BLIT HERE
												Blt8BPPDataTo16BPPBufferTransShadow(l.Buffer<UINT16>(), l.Pitch(), hVObject, sXPos, sYPos, usImageIndex, pShadeTable);

												// Turn it off!
												pNode->uiFlags &= ~LEVELNODE_UPDATESAVEBUFFERONCE;
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransShadow(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
										}
									}
									else if (fShadowBlitter)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferShadowZ(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferShadowZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferShadow(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex);
										}
									}
									else if (fIntensityBlitter)
									{
										if (fZBlitter)
										{
											if (fZWrite)
											{
												Blt8BPPDataTo16BPPBufferIntensityZ(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferIntensityZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferIntensity(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex);
										}
									}
									else if (fZBlitter)
									{
										if (fZWrite)
										{
											if (fObscuredBlitter)
											{
												Blt8BPPDataTo16BPPBufferTransZPixelateObscured(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
											else
											{
												Blt8BPPDataTo16BPPBufferTransZ(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
											}
										}
										else
										{
											Blt8BPPDataTo16BPPBufferTransZNB(pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);
										}

										if (uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)
										{
											SGPVSurface::Lock l(guiSAVEBUFFER);

											// BLIT HERE
											Blt8BPPDataTo16BPPBufferTransZ(l.Buffer<UINT16>(), l.Pitch(), gpZBuffer, sZLevel, hVObject, sXPos, sYPos, usImageIndex);

											// Turn it off!
											pNode->uiFlags &= ~LEVELNODE_UPDATESAVEBUFFERONCE;
										}

									}
									else
									{
										Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex);
									}
								}
							}
						}

next_prev_node:
						if (fLinkedListDirection)
						{
next_node:
							pNode = pNode->pNext;
						}
						else
						{
							pNode = pNode->pPrevNode;
						}
					}
				}
				else
				{
					if (gfEditMode)
					{
						// ATE: Used here in the editor to denote when an area is not in the world
						/* Kris:  Fixed a couple things here...
						 * It seems that scrolling to the bottom right hand corner of the
						 * map, would cause the end of the world to be drawn.  Now, this
						 * would only crash on my computer and not Emmons, so this should
						 * work.  Also, I changed the color from fluorescent green to
						 * black, which is easier on the eyes, and prevent the drawing of
						 * the end of the world if it would be drawn on the editor's
						 * taskbar. */
						if (iTempPosY_S < 360)
						{
							ColorFillVideoSurfaceArea(FRAME_BUFFER, iTempPosX_S, iTempPosY_S, iTempPosX_S + 40, MIN(iTempPosY_S + 20, 360), Get16BPPColor(FROMRGB(0, 0, 0)));
						}
					}
				}

next_tile:
				iTempPosX_S += 40;
				iTempPosX_M++;
				iTempPosY_M--;
			} while (iTempPosX_S < iEndXS);
		}

		if (bXOddFlag)
		{
			iAnchorPosY_M++;
		}
		else
		{
			iAnchorPosX_M++;
		}

		bXOddFlag = !bXOddFlag;
		iAnchorPosY_S += 10;
	}
	while (iAnchorPosY_S < iEndYS);

	if (uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE) EndCurInteractiveTileCheck();
}


// memcpy's the background to the new scroll position, and renders the missing strip
// via the RenderStaticWorldRect. Dynamic stuff will be updated on the next frame
// by the normal render cycle
static void ScrollBackground(INT16 sScrollXIncrement, INT16 sScrollYIncrement)
{
	if (!gfDoVideoScroll)
	{
		// Clear z-buffer
		memset(gpZBuffer, LAND_Z_LEVEL, gsVIEWPORT_END_Y * SCREEN_WIDTH * 2);

		RenderStaticWorldRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y, FALSE);

		FreeBackgroundRectType(BGND_FLAG_ANIMATED);
	}
	else
	{
		gsScrollXIncrement += sScrollXIncrement;
		gsScrollYIncrement += sScrollYIncrement;
	}
}


enum ScrollType {
	ScrollType_Undefined,
	ScrollType_Horizontal,
	ScrollType_Vertical
};

static BOOLEAN ApplyScrolling(INT16 sTempRenderCenterX, INT16 sTempRenderCenterY, BOOLEAN fForceAdjust, BOOLEAN fCheckOnly,
				ScrollType scrollType=ScrollType_Undefined);
static void ClearMarkedTiles(void);
static void ExamineZBufferRect(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
static void RenderDynamicWorld(void);
static void RenderMarkedWorld(void);
static void RenderRoomInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);
static void RenderStaticWorld(void);


// Render routine takes center X, Y and Z coordinate and gets world
// Coordinates for the window from that using the following functions
// For coordinate transformations
void RenderWorld(void)
{
	gfRenderFullThisFrame = FALSE;

	// If we are testing renderer, set background to pink!
	if (gTacticalStatus.uiFlags & DEBUGCLIFFS)
	{
		ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y, Get16BPPColor(FROMRGB(0, 255, 0)));
		SetRenderFlags(RENDER_FLAG_FULL);
	}

	if (gTacticalStatus.uiFlags & SHOW_Z_BUFFER)
	{
		SetRenderFlags(RENDER_FLAG_FULL);
	}

	// For now here, update animated tiles
	if (COUNTERDONE(ANIMATETILES))
	{
		RESETCOUNTER(ANIMATETILES);
		for (UINT32 i = 0; i != gusNumAnimatedTiles; ++i)
		{
			TILE_ANIMATION_DATA& a = *gTileDatabase[gusAnimatedTiles[i]].pAnimData;
			if (++a.bCurrentFrame >= a.ubNumFrames) a.bCurrentFrame = 0;
		}
	}

	// HERE, UPDATE GLOW INDEX
	if (COUNTERDONE(GLOW_ENEMYS))
	{
		RESETCOUNTER(GLOW_ENEMYS);
		gsCurrentGlowFrame     = (gsCurrentGlowFrame     + 1) % lengthof(gsGlowFrames);
		gsCurrentItemGlowFrame = (gsCurrentItemGlowFrame + 1) % NUM_ITEM_CYCLE_COLORS;
	}

	if (gRenderFlags & RENDER_FLAG_FULL)
	{
		gfRenderFullThisFrame = TRUE;
		gfTopMessageDirty     = TRUE;

		// Dirty the interface...
		fInterfacePanelDirty = DIRTYLEVEL2;

		// Apply scrolling sets some world variables
		ApplyScrolling(gsRenderCenterX, gsRenderCenterY, TRUE, FALSE);
		ResetLayerOptimizing();

		if (gRenderFlags & RENDER_FLAG_NOZ)
		{
			RenderStaticWorldRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y, FALSE);
		}
		else
		{
			RenderStaticWorld();
		}

		if (!(gRenderFlags & RENDER_FLAG_SAVEOFF)) UpdateSaveBuffer();
	}
	else if (gRenderFlags & RENDER_FLAG_MARKED)
	{
		ResetLayerOptimizing();
		RenderMarkedWorld();
		if (!(gRenderFlags & RENDER_FLAG_SAVEOFF)) UpdateSaveBuffer();
	}

	if (!g_scroll_inertia               ||
			gRenderFlags & RENDER_FLAG_NOZ  ||
			gRenderFlags & RENDER_FLAG_FULL ||
			gRenderFlags & RENDER_FLAG_MARKED)
	{
		RenderDynamicWorld();
	}

	if (g_scroll_inertia) EmptyBackgroundRects();

	if (gRenderFlags & RENDER_FLAG_ROOMIDS)
	{
		RenderRoomInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
	}

#ifdef _DEBUG
	if (gRenderFlags & RENDER_FLAG_FOVDEBUG)
	{
		RenderFOVDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
	}
	else if (gfDisplayCoverValues)
	{
		RenderCoverDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
	}
	else if (gfDisplayGridNoVisibleValues)
	{
		RenderGridNoVisibleDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
	}
#endif


	if (gRenderFlags & RENDER_FLAG_MARKED) ClearMarkedTiles();

	if (gRenderFlags & RENDER_FLAG_CHECKZ && !(gTacticalStatus.uiFlags & NOHIDE_REDUNDENCY))
	{
		ExamineZBufferRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y);
	}

	gRenderFlags &= ~(RENDER_FLAG_FULL | RENDER_FLAG_MARKED | RENDER_FLAG_ROOMIDS | RENDER_FLAG_CHECKZ);

	if (gTacticalStatus.uiFlags & SHOW_Z_BUFFER)
	{
		// COPY Z BUFFER TO FRAME BUFFER
		SGPVSurface::Lock l(FRAME_BUFFER);
		UINT16* const pDestBuf = l.Buffer<UINT16>();

		for (UINT32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
		{
			pDestBuf[i] = gpZBuffer[i];
		}
	}
}


static void CalcRenderParameters(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
static void ResetRenderParameters(void);


// Start with a center X,Y,Z world coordinate and render direction
// Determine WorldIntersectionPoint and the starting block from these
// Then render away!
void RenderStaticWorldRect(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom, BOOLEAN fDynamicsToo)
{
	RenderLayerID sLevelIDs[10];

	// Calculate render starting parameters
	CalcRenderParameters(sLeft, sTop, sRight, sBottom);

	// Reset layer optimizations
	ResetLayerOptimizing();

	// STATICS
	sLevelIDs[0] = RENDER_STATIC_LAND;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);

	sLevelIDs[0] = RENDER_STATIC_OBJECTS;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);

	if (gRenderFlags & RENDER_FLAG_SHADOWS)
	{
		sLevelIDs[0] = RENDER_STATIC_SHADOWS;
		RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);
	}

	sLevelIDs[0] = RENDER_STATIC_STRUCTS;
	sLevelIDs[1] = RENDER_STATIC_ROOF;
	sLevelIDs[2] = RENDER_STATIC_ONROOF;
	sLevelIDs[3] = RENDER_STATIC_TOPMOST;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 4, sLevelIDs);

	//ATE: Do obsucred layer!
	sLevelIDs[0] = RENDER_STATIC_STRUCTS;
	sLevelIDs[1] = RENDER_STATIC_ONROOF;
	RenderTiles(TILES_OBSCURED, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 2, sLevelIDs);

	if (fDynamicsToo)
	{
		// DYNAMICS
		sLevelIDs[0] = RENDER_DYNAMIC_LAND;
		sLevelIDs[1] = RENDER_DYNAMIC_OBJECTS;
		sLevelIDs[2] = RENDER_DYNAMIC_SHADOWS;
		sLevelIDs[3] = RENDER_DYNAMIC_STRUCT_MERCS;
		sLevelIDs[4] = RENDER_DYNAMIC_MERCS;
		sLevelIDs[5] = RENDER_DYNAMIC_STRUCTS;
		sLevelIDs[6] = RENDER_DYNAMIC_ROOF;
		sLevelIDs[7] = RENDER_DYNAMIC_HIGHMERCS;
		sLevelIDs[8] = RENDER_DYNAMIC_ONROOF;
		RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 9, sLevelIDs);

		SumAdditiveLayerOptimization();
	}

	ResetRenderParameters();

	if (!gfDoVideoScroll) AddBaseDirtyRect(sLeft, sTop, sRight, sBottom);
}


static void RenderStaticWorld(void)
{
	RenderLayerID sLevelIDs[9];

	// Calculate render starting parameters
	CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

	// Clear z-buffer
	memset(gpZBuffer, LAND_Z_LEVEL, gsVIEWPORT_END_Y * SCREEN_WIDTH * 2);

	FreeBackgroundRectType(BGND_FLAG_ANIMATED);
	InvalidateBackgroundRects();

	sLevelIDs[0] = RENDER_STATIC_LAND;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);

	sLevelIDs[0] = RENDER_STATIC_OBJECTS;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);

	if (gRenderFlags & RENDER_FLAG_SHADOWS)
	{
		sLevelIDs[0] = RENDER_STATIC_SHADOWS;
		RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 1, sLevelIDs);
	}

	sLevelIDs[0] = RENDER_STATIC_STRUCTS;
	sLevelIDs[1] = RENDER_STATIC_ROOF;
	sLevelIDs[2] = RENDER_STATIC_ONROOF;
	sLevelIDs[3] = RENDER_STATIC_TOPMOST;
	RenderTiles(TILES_NONE, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 4, sLevelIDs);

	//ATE: Do obsucred layer!
	sLevelIDs[0] = RENDER_STATIC_STRUCTS;
	sLevelIDs[1] = RENDER_STATIC_ONROOF;
	RenderTiles(TILES_OBSCURED, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS, gsLEndYS, 2, sLevelIDs);

	AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y);
	ResetRenderParameters();
}


static void RenderMarkedWorld(void)
{
	RenderLayerID sLevelIDs[4];

	CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

	RestoreBackgroundRects();
	FreeBackgroundRectType(BGND_FLAG_ANIMATED);
	InvalidateBackgroundRects();

	ResetLayerOptimizing();

	sLevelIDs[0] = RENDER_STATIC_LAND;
	sLevelIDs[1] = RENDER_STATIC_OBJECTS;
	RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 2, sLevelIDs);

	if (gRenderFlags & RENDER_FLAG_SHADOWS)
	{
		sLevelIDs[0] = RENDER_STATIC_SHADOWS;
		RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);
	}

	sLevelIDs[0] = RENDER_STATIC_STRUCTS;
	RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);

	sLevelIDs[0] = RENDER_STATIC_ROOF;
	RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);

	sLevelIDs[0] = RENDER_STATIC_ONROOF;
	RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);

	sLevelIDs[0] = RENDER_STATIC_TOPMOST;
	RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);

	AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y);

	ResetRenderParameters();
}


static void RenderDynamicWorld(void)
{
	RenderLayerID sLevelIDs[10];

	CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

	RestoreBackgroundRects();

	sLevelIDs[0] = RENDER_DYNAMIC_OBJECTS;
	sLevelIDs[1] = RENDER_DYNAMIC_SHADOWS;
	sLevelIDs[2] = RENDER_DYNAMIC_STRUCT_MERCS;
	sLevelIDs[3] = RENDER_DYNAMIC_MERCS;
	sLevelIDs[4] = RENDER_DYNAMIC_STRUCTS;
	sLevelIDs[5] = RENDER_DYNAMIC_HIGHMERCS;
	sLevelIDs[6] = RENDER_DYNAMIC_ROOF;
	sLevelIDs[7] = RENDER_DYNAMIC_ONROOF;
	sLevelIDs[8] = RENDER_DYNAMIC_TOPMOST;
	RenderTiles(TILES_DIRTY, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 9, sLevelIDs);

	if (!GameState::getInstance()->isEditorMode() || (!gfEditMode && !gfAniEditMode))
	{
		RenderTacticalInterface();
	}

	SaveBackgroundRects();

	sLevelIDs[0] = RENDER_DYNAMIC_OBJECTS;
	sLevelIDs[1] = RENDER_DYNAMIC_SHADOWS;
	sLevelIDs[2] = RENDER_DYNAMIC_STRUCT_MERCS;
	sLevelIDs[3] = RENDER_DYNAMIC_MERCS;
	sLevelIDs[4] = RENDER_DYNAMIC_STRUCTS;
	RenderTiles(TILES_NONE, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 5, sLevelIDs);

	sLevelIDs[0] = RENDER_DYNAMIC_ROOF;
	sLevelIDs[1] = RENDER_DYNAMIC_HIGHMERCS;
	sLevelIDs[2] = RENDER_DYNAMIC_ONROOF;
	RenderTiles(TILES_NONE, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 3, sLevelIDs);

	sLevelIDs[0] = RENDER_DYNAMIC_TOPMOST;
	// ATE: check here for mouse over structs.....
	RenderTiles(TILES_DYNAMIC_CHECKFOR_INT_TILE, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS, 1, sLevelIDs);

	SumAdditiveLayerOptimization();
	ResetRenderParameters();
}


static BOOLEAN HandleScrollDirections(UINT32 ScrollFlags, INT16 sScrollXStep, INT16 sScrollYStep, BOOLEAN fCheckOnly)
{
	// printf("HandleScrollDirections: %d, (%4d, %4d), %d\n", ScrollFlags, sScrollXStep, sScrollYStep, fCheckOnly);
	INT16 scroll_x = 0;
	if (ScrollFlags & SCROLL_LEFT)  scroll_x -= sScrollXStep;
	if (ScrollFlags & SCROLL_RIGHT) scroll_x += sScrollXStep;

	INT16 scroll_y = 0;
	if (ScrollFlags & SCROLL_UP)    scroll_y -= sScrollYStep;
	if (ScrollFlags & SCROLL_DOWN)  scroll_y += sScrollYStep;

	if (scroll_x != 0)
	{
		// Check horizontal
		INT16 sTempX_W;
		INT16 sTempY_W;
		FromScreenToCellCoordinates(scroll_x, 0, &sTempX_W, &sTempY_W);
		const INT16 sTempRenderCenterX = gsRenderCenterX + sTempX_W;
		const INT16 sTempRenderCenterY = gsRenderCenterY + sTempY_W;
		if (!ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly, ScrollType_Horizontal))
		{
			scroll_x = 0;
		}
	}

	if (scroll_y != 0)
	{
		// Check vertical
		INT16 sTempX_W;
		INT16 sTempY_W;
		FromScreenToCellCoordinates(0, scroll_y, &sTempX_W, &sTempY_W);
		const INT16 sTempRenderCenterX = gsRenderCenterX + sTempX_W;
		const INT16 sTempRenderCenterY = gsRenderCenterY + sTempY_W;
		if (!ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly, ScrollType_Vertical))
		{
			scroll_y = 0;
		}
	}

	const BOOLEAN fAGoodMove = (scroll_x != 0 || scroll_y != 0);
	if (fAGoodMove && !fCheckOnly) ScrollBackground(scroll_x, scroll_y);

	return fAGoodMove;
}


static UINT ScrollSpeed(void)
{
	UINT speed = 20 << (_KeyDown(SHIFT) ? 2 : gubCurScrollSpeedID);
	if (!gfDoVideoScroll) speed *= 2;
	return speed;
}


void ScrollWorld(void)
{
	static UINT8   ubOldScrollSpeed        = 0;
	static BOOLEAN fFirstTimeInSlideToMode = TRUE;

	if (gfIgnoreScrollDueToCenterAdjust)
	{
		//	gfIgnoreScrollDueToCenterAdjust = FALSE;
		return;
	}

	BOOLEAN fIgnoreInput = FALSE;

	if (gfIgnoreScrolling) return;
	if (gCurrentUIMode == LOCKUI_MODE) fIgnoreInput = TRUE;

	// If in editor, ignore scrolling if any of the shift keys pressed with arrow keys
	if (gfEditMode && _KeyDown(CTRL)) return;

	if (_KeyDown(ALT)) return;

	UINT32 ScrollFlags = 0;

	do
	{
		// Check for sliding
		if (gTacticalStatus.sSlideTarget != NOWHERE)
		{
			// Ignore all input...
			// Check if we have reached out dest!
			if (fFirstTimeInSlideToMode)
			{
				ubOldScrollSpeed = gubCurScrollSpeedID;
				fFirstTimeInSlideToMode = FALSE;
			}

			ScrollFlags = 0;
			INT8 bDirection;
			if (SoldierLocationRelativeToScreen(gTacticalStatus.sSlideTarget, &bDirection, &ScrollFlags) &&
					GridNoOnVisibleWorldTile(gTacticalStatus.sSlideTarget))
			{
				static const UINT32 gScrollDirectionFlags[] =
				{
					SCROLL_UP | SCROLL_RIGHT,
					SCROLL_RIGHT,
					SCROLL_DOWN | SCROLL_RIGHT,
					SCROLL_DOWN,
					SCROLL_DOWN | SCROLL_LEFT,
					SCROLL_LEFT,
					SCROLL_UP | SCROLL_LEFT,
					SCROLL_UP,
				};

				ScrollFlags  = gScrollDirectionFlags[bDirection];
				fIgnoreInput = TRUE;
			}
			else
			{
				// We've stopped!
				gTacticalStatus.sSlideTarget = NOWHERE;
			}
		}
		else
		{
			// Restore old scroll speed
			if (!fFirstTimeInSlideToMode)
			{
				gubCurScrollSpeedID = ubOldScrollSpeed;
			}
			fFirstTimeInSlideToMode = TRUE;
		}

		if (!fIgnoreInput)
		{
			// Check keys
			if (_KeyDown(SDLK_UP))    ScrollFlags |= SCROLL_UP;
			if (_KeyDown(SDLK_DOWN))  ScrollFlags |= SCROLL_DOWN;
			if (_KeyDown(SDLK_RIGHT)) ScrollFlags |= SCROLL_RIGHT;
			if (_KeyDown(SDLK_LEFT))  ScrollFlags |= SCROLL_LEFT;

			// Do mouse - PUT INTO A TIMER!
			// Put a counter on starting from mouse, if we have not started already!
			if (!g_scroll_inertia && !gfScrollPending)
			{
				if (!COUNTERDONE(STARTSCROLL)) break;
				RESETCOUNTER(STARTSCROLL);
			}

			if (gusMouseYPos <  NO_PX_SHOW_EXIT_CURS)                 ScrollFlags |= SCROLL_UP;
			if (gusMouseYPos >= SCREEN_HEIGHT - NO_PX_SHOW_EXIT_CURS) ScrollFlags |= SCROLL_DOWN;
			if (gusMouseXPos >= SCREEN_WIDTH  - NO_PX_SHOW_EXIT_CURS) ScrollFlags |= SCROLL_RIGHT;
			if (gusMouseXPos <  NO_PX_SHOW_EXIT_CURS)                 ScrollFlags |= SCROLL_LEFT;
		}
	}
	while (FALSE);


	BOOLEAN fAGoodMove   = FALSE;
	INT16   sScrollXStep = -1;
	INT16   sScrollYStep = -1;
	if (ScrollFlags != 0)
	{
		// Adjust speed based on whether shift is down
		const UINT speed = ScrollSpeed();
		sScrollXStep = speed;
		sScrollYStep = speed / 2;

		fAGoodMove = HandleScrollDirections(ScrollFlags, sScrollXStep, sScrollYStep, TRUE);
	}

	// Has this been an OK scroll?
	if (fAGoodMove)
	{
		if (COUNTERDONE(NEXTSCROLL))
		{
			RESETCOUNTER(NEXTSCROLL);

			// Are we starting a new scroll?
			if (!g_scroll_inertia && !gfScrollPending)
			{
				// We are starting to scroll - setup scroll pending
				gfScrollPending = TRUE;

				// Remove any interface stuff
				ClearInterface();

				// Return so that next frame things will be erased!
				return;
			}

			// If here, set scroll pending to false
			gfScrollPending = FALSE;

			g_scroll_inertia = true;

			// Now we actually begin our scrolling
			HandleScrollDirections(ScrollFlags, sScrollXStep, sScrollYStep, FALSE);
		}
	}
	else
	{
		// ATE: Also if scroll pending never got to scroll....
		if (gfScrollPending)
		{
			// Do a complete rebuild!
			gfScrollPending = FALSE;

			// Restore Interface!
			RestoreInterface();

			DeleteVideoOverlaysArea();
		}

		// Check if we have just stopped scrolling!
		if (g_scroll_inertia)
		{
			SetRenderFlags(RENDER_FLAG_FULL | RENDER_FLAG_CHECKZ);

			// Restore Interface!
			RestoreInterface();

			DeleteVideoOverlaysArea();
		}

		g_scroll_inertia = false;
		gfScrollPending  = FALSE;
	}
}


void InitRenderParams(UINT8 ubRestrictionID)
{
	INT16 gTopLeftWorldLimitX;     // XXX HACK000E
	INT16 gTopLeftWorldLimitY;     // XXX HACK000E
	INT16 gTopRightWorldLimitX;    // XXX HACK000E
	INT16 gTopRightWorldLimitY;    // XXX HACK000E
	INT16 gBottomLeftWorldLimitX;  // XXX HACK000E
	INT16 gBottomLeftWorldLimitY;  // XXX HACK000E
	INT16 gBottomRightWorldLimitX; // XXX HACK000E
	INT16 gBottomRightWorldLimitY; // XXX HACK000E
	switch (ubRestrictionID)
	{
		case 0: // Default!
			gTopLeftWorldLimitX = CELL_X_SIZE;
			gTopLeftWorldLimitY = CELL_X_SIZE * WORLD_ROWS / 2;

			gTopRightWorldLimitX = CELL_Y_SIZE * WORLD_COLS / 2;
			gTopRightWorldLimitY = CELL_X_SIZE;

			gBottomLeftWorldLimitX = CELL_Y_SIZE * WORLD_COLS / 2;
			gBottomLeftWorldLimitY = CELL_Y_SIZE * WORLD_ROWS;

			gBottomRightWorldLimitX = CELL_Y_SIZE * WORLD_COLS;
			gBottomRightWorldLimitY = CELL_X_SIZE * WORLD_ROWS / 2;
			break;

		case 1: // BAEMENT LEVEL 1
			gTopLeftWorldLimitX = CELL_X_SIZE * WORLD_ROWS * 3 / 10;
			gTopLeftWorldLimitY = CELL_X_SIZE * WORLD_ROWS     /  2;

			gTopRightWorldLimitX = CELL_X_SIZE * WORLD_ROWS     /  2;
			gTopRightWorldLimitY = CELL_X_SIZE * WORLD_COLS * 3 / 10;

			gBottomLeftWorldLimitX = CELL_X_SIZE * WORLD_ROWS     /  2;
			gBottomLeftWorldLimitY = CELL_X_SIZE * WORLD_COLS * 7 / 10;

			gBottomRightWorldLimitX = CELL_X_SIZE * WORLD_ROWS * 7 / 10;
			gBottomRightWorldLimitY = CELL_X_SIZE * WORLD_ROWS     /  2;
			break;

		default: abort(); // HACK000E
	}

	gCenterWorldX = CELL_X_SIZE * WORLD_ROWS / 2;
	gCenterWorldY = CELL_X_SIZE * WORLD_COLS / 2;

	// Convert Bounding box into screen coords
	FromCellToScreenCoordinates(gTopLeftWorldLimitX,     gTopLeftWorldLimitY,     &gsTLX, &gsTLY);
	FromCellToScreenCoordinates(gTopRightWorldLimitX,    gTopRightWorldLimitY,    &gsTRX, &gsTRY);
	FromCellToScreenCoordinates(gBottomLeftWorldLimitX,  gBottomLeftWorldLimitY,  &gsBLX, &gsBLY);
	FromCellToScreenCoordinates(gBottomRightWorldLimitX, gBottomRightWorldLimitY, &gsBRX, &gsBRY);
	FromCellToScreenCoordinates(gCenterWorldX,           gCenterWorldY,           &gsCX,  &gsCY);

	// Adjust for interface height tabbing!
	gsTLY += ROOF_LEVEL_HEIGHT;
	gsTRY += ROOF_LEVEL_HEIGHT;
	gsCY  += ROOF_LEVEL_HEIGHT / 2;

	SLOGD(DEBUG_TAG_RENDERWORLD, "World Screen Width %d Height %d", gsTRX - gsTLX, gsBRY - gsTRY);

	// Determine scale factors
	// First scale world screen coords for VIEWPORT ratio
	const double dWorldX = gsTRX - gsTLX;
	const double dWorldY = gsBRY - gsTRY;

	gdScaleX = (double)RADAR_WINDOW_WIDTH  / dWorldX;
	gdScaleY = (double)RADAR_WINDOW_HEIGHT / dWorldY;

	const UINT32 n = NUM_ITEM_CYCLE_COLORS;
	for (UINT32 i = 0; i < n; ++i)
	{
		const UINT32 l = (i < n / 2 ? i + 1 : n - i) * (250 / (n / 2));
		us16BPPItemCycleWhiteColors[i]  = Get16BPPColor(FROMRGB(l, l, l));
		us16BPPItemCycleRedColors[i]    = Get16BPPColor(FROMRGB(l, 0, 0));
		us16BPPItemCycleYellowColors[i] = Get16BPPColor(FROMRGB(l, l, 0));
	}

	gusNormalItemOutlineColor = Get16BPPColor(FROMRGB(255, 255, 255));
	gusYellowItemOutlineColor = Get16BPPColor(FROMRGB(255, 255,   0));
}

/** This function checks whether the render screen can be moved to new position. */
static BOOLEAN ApplyScrolling(INT16 sTempRenderCenterX, INT16 sTempRenderCenterY, BOOLEAN fForceAdjust, BOOLEAN fCheckOnly,
				ScrollType scrollType)
{
	// printf("~ %4d, %4d\n", sTempRenderCenterX, sTempRenderCenterY);

	// Make sure it's a multiple of 5
	sTempRenderCenterX = sTempRenderCenterX / CELL_X_SIZE * CELL_X_SIZE + CELL_X_SIZE / 2;
	sTempRenderCenterY = sTempRenderCenterY / CELL_X_SIZE * CELL_Y_SIZE + CELL_Y_SIZE / 2;

	// From render center in world coords, convert to render center in "screen" coords
	INT16 sScreenCenterX;
	INT16 sScreenCenterY;
	FromCellToScreenCoordinates(sTempRenderCenterX, sTempRenderCenterY, &sScreenCenterX, &sScreenCenterY);

	// Adjust for offset position on screen
	sScreenCenterX -=  0;
	sScreenCenterY -= 10;

	const INT16 sX_S = g_ui.m_tacticalMapCenterX;
	const INT16 sY_S = g_ui.m_tacticalMapCenterY;

	// Get corners in screen coords
	// TOP LEFT
	const INT16 sTopLeftWorldX = sScreenCenterX - sX_S;
	const INT16 sTopLeftWorldY = sScreenCenterY - sY_S;

	const INT16 sTopRightWorldX = sScreenCenterX + sX_S;
	// const INT16 sTopRightWorldY = sScreenCenterY - sY_S;

	// const INT16 sBottomLeftWorldX = sScreenCenterX - sX_S;
	const INT16 sBottomLeftWorldY = sScreenCenterY + sY_S;

	const INT16 sBottomRightWorldX = sScreenCenterX + sX_S;
	const INT16 sBottomRightWorldY = sScreenCenterY + sY_S;

#define TILE_SIZE 10

	// Checking if screen shows areas outside of the map
	BOOLEAN fOutLeft   = (gsTLX > sTopLeftWorldX);
	BOOLEAN fOutRight  = (gsTRX < sTopRightWorldX);
	BOOLEAN fOutTop    = (gsTLY >= sTopLeftWorldY);            /* top of the screen is above top of the map */
	BOOLEAN fOutBottom = (gsBLY < sBottomLeftWorldY);          /* bottom of the screen is below bottom if the map */

	int mapHeight = gsBLY - gsTLY;
	int screenHeight = gsVIEWPORT_END_Y - gsVIEWPORT_START_Y;

	int mapWidth = gsTRX - gsTLX;
	int screenWidth = gsVIEWPORT_END_X - gsVIEWPORT_START_X;

	BOOLEAN fScrollGood = FALSE;

	if (!fOutRight && !fOutLeft && !fOutTop && !fOutBottom)
	{
		// Nothing goes outside the borders of the map.
		// Can change render center.
		fScrollGood = TRUE;
	}
	else
	{
		// Something is outside the border.
		// Let's check if we can move horizontally or vertically.

		if((scrollType == ScrollType_Horizontal)
			&& (((sTempRenderCenterX < gsRenderCenterX) && !fOutLeft)                /** moving left */
			|| ((sTempRenderCenterX > gsRenderCenterX) && !fOutRight)))            /** moving right */
		{
			// can move
			fScrollGood = TRUE;
		}

		if((scrollType == ScrollType_Vertical)
			&& (((sTempRenderCenterY < gsRenderCenterY) && !fOutTop)
			|| ((sTempRenderCenterY > gsRenderCenterY) && !fOutBottom)))
		{
			// can move
			fScrollGood = TRUE;
		}
	}

	// If in editor, anything goes
	if (gfEditMode && _KeyDown(SHIFT)) fScrollGood = TRUE;

	// Reset some UI flags
	gfUIShowExitEast  = FALSE;
	gfUIShowExitWest  = FALSE;
	gfUIShowExitNorth = FALSE;
	gfUIShowExitSouth = FALSE;

	if (!fScrollGood)
	{
		if (fForceAdjust)
		{
			INT16 newScreenCenterX = sScreenCenterX;
			INT16 newScreenCenterY = sScreenCenterY;

			if (screenHeight > mapHeight)
			{
				// printf("screen height is bigger than map height\n");
				newScreenCenterY = gsCY;
			}
			else if (fOutTop)
			{
				newScreenCenterY = gsTLY + sY_S;
			}
			else if (fOutBottom)
			{
				newScreenCenterY = gsBLY - sY_S - 50;
			}

			if (screenWidth > mapWidth)
			{
				// printf("screen width is bigger than map width\n");
				newScreenCenterX = gsCX;
			}
			else if (fOutLeft)
			{
				newScreenCenterX = gsTLX + sX_S;
			}
			else if (fOutRight)
			{
				newScreenCenterX = gsTRX - sX_S;
			}

			INT16 sTempPosX_W;
			INT16 sTempPosY_W;
			FromScreenToCellCoordinates(newScreenCenterX, newScreenCenterY, &sTempPosX_W, &sTempPosY_W);
			sTempRenderCenterX = sTempPosX_W;
			sTempRenderCenterY = sTempPosY_W;
			fScrollGood = TRUE;
		}
		else
		{
			if (fOutRight  && gusMouseXPos >= SCREEN_WIDTH - NO_PX_SHOW_EXIT_CURS)  gfUIShowExitEast  = TRUE;
			if (fOutLeft   && gusMouseXPos <  NO_PX_SHOW_EXIT_CURS)                 gfUIShowExitWest  = TRUE;
			if (fOutTop    && gusMouseYPos <  NO_PX_SHOW_EXIT_CURS)                 gfUIShowExitNorth = TRUE;
			if (fOutBottom && gusMouseYPos >= SCREEN_HEIGHT - NO_PX_SHOW_EXIT_CURS) gfUIShowExitSouth = TRUE;
		}
	}

	if (fScrollGood && !fCheckOnly)
	{
		// Make sure it's a multiple of 5
		gsRenderCenterX = sTempRenderCenterX / CELL_X_SIZE * CELL_X_SIZE + CELL_X_SIZE / 2;
		gsRenderCenterY = sTempRenderCenterY / CELL_X_SIZE * CELL_Y_SIZE + CELL_Y_SIZE / 2;

		gsTopLeftWorldX = sTopLeftWorldX - gsTLX;
		gsTopLeftWorldY = sTopLeftWorldY - gsTLY;

		gsBottomRightWorldX = sBottomRightWorldX - gsTLX;
		gsBottomRightWorldY = sBottomRightWorldY - gsTLY;

		SetPositionSndsVolumeAndPanning();
	}

	return fScrollGood;
}


static void ClearMarkedTiles(void)
{
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags &= ~MAPELEMENT_REDRAW;
	}
}


void InvalidateWorldRedundency(void)
{
	SetRenderFlags(RENDER_FLAG_CHECKZ);
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags |= MAPELEMENT_REEVALUATE_REDUNDENCY;
	}
}


#define Z_STRIP_DELTA_Y  (Z_SUBLAYERS * 10)

/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZIncClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
static void Blt8BPPDataTo16BPPBufferTransZIncClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion)
{
	UINT32 Unblitted;
	INT32  LSCount;
	UINT16 usZLevel, usZColsToGo, usZIndex;

	Assert(hSrcVObject != NULL);
	Assert(pBuffer     != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	INT32       const  usHeight = pTrav.usHeight;
	INT32       const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	INT32 ClipX1;
	INT32 ClipY1;
	INT32 ClipX2;
	INT32 ClipY2;
	if (clipregion == NULL)
	{
		ClipX1 = ClippingRect.iLeft;
		ClipY1 = ClippingRect.iTop;
		ClipX2 = ClippingRect.iRight;
		ClipY2 = ClippingRect.iBottom;
	}
	else
	{
		ClipX1 = clipregion->iLeft;
		ClipY1 = clipregion->iTop;
		ClipX2 = clipregion->iRight;
		ClipY2 = clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	const INT32 LeftSkip   = __min(ClipX1 -   MIN(ClipX1, iTempX), usWidth);
	INT32       TopSkip    = __min(ClipY1 - __min(ClipY1, iTempY), usHeight);
	const INT32 RightSkip  = __min(  MAX(ClipX2, iTempX + usWidth)  - ClipX2, usWidth);
	const INT32 BottomSkip = __min(__max(ClipY2, iTempY + usHeight) - ClipY2, usHeight);

	// calculate the remaining rows and columns to blit
	const INT32 BlitLength = usWidth  - LeftSkip - RightSkip;
	INT32       BlitHeight = usHeight - TopSkip  - BottomSkip;

	// check if whole thing is clipped
	if (LeftSkip >= usWidth  || RightSkip  >= usWidth)  return;
	if (TopSkip  >= usHeight || BottomSkip >= usHeight) return;

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	UINT8*       DestPtr  = (UINT8*)pBuffer  + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	UINT8*       ZPtr     = (UINT8*)pZBuffer + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	const UINT32 LineSkip = uiDestPitchBYTES - BlitLength * 2;
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();

	if (hSrcVObject->ppZStripInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}
	// setup for the z-column blitting stuff
	const ZStripInfo* const pZInfo = hSrcVObject->ppZStripInfo[usIndex];
	if (pZInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}

	UINT16 usZStartLevel = (INT16)usZValue + pZInfo->bInitialZChange * Z_STRIP_DELTA_Y;
	// set to odd number of pixels for first column

	UINT16 usZStartCols;
	if  (LeftSkip > pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = LeftSkip - pZInfo->ubFirstZStripWidth;
		usZStartCols = 20 - usZStartCols % 20;
	}
	else if (LeftSkip < pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = pZInfo->ubFirstZStripWidth - LeftSkip;
	}
	else
	{
		usZStartCols = 20;
	}

	usZColsToGo = usZStartCols;

	const INT8* const pZArray = pZInfo->pbZChange;

	UINT16 usZStartIndex;
	if (LeftSkip >= pZInfo->ubFirstZStripWidth)
	{
		// Index into array after doing left clipping
		usZStartIndex = 1 + (LeftSkip - pZInfo->ubFirstZStripWidth) / 20;

		//calculates the Z-value after left-side clipping
		if (usZStartIndex)
		{
			for (UINT16 i = 0; i < usZStartIndex; i++)
			{
				switch (pZArray[i])
				{
					case -1: usZStartLevel -= Z_STRIP_DELTA_Y; break;
					case  0: /* no change */                   break;
					case  1: usZStartLevel += Z_STRIP_DELTA_Y; break;
				}
			}
		}
	}
	else
	{
		usZStartIndex = 0;
	}

	usZLevel = usZStartLevel;
	usZIndex = usZStartIndex;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		usZLevel = usZStartLevel;
		usZIndex = usZStartIndex;
		usZColsToGo = usZStartCols;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > LSCount)
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > LSCount) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
				for (;;)
				{
					if (PxCount >= usZColsToGo)
					{
						PxCount -= usZColsToGo;
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
					else
					{
						usZColsToGo -= PxCount;
						break;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > LSCount)
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZLevel)
					{
						*(UINT16*)ZPtr = usZLevel;
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
					if (--usZColsToGo == 0)
					{
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZIncClipSaveZBurnsThrough

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
static void Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion)
{
	UINT32 Unblitted;
	INT32  LSCount;
	UINT16 usZLevel, usZColsToGo, usZStartIndex, usZIndex;

	Assert(hSrcVObject != NULL);
	Assert(pBuffer     != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	INT32       const  usHeight = pTrav.usHeight;
	INT32       const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	INT32 ClipX1;
	INT32 ClipY1;
	INT32 ClipX2;
	INT32 ClipY2;
	if (clipregion == NULL)
	{
		ClipX1 = ClippingRect.iLeft;
		ClipY1 = ClippingRect.iTop;
		ClipX2 = ClippingRect.iRight;
		ClipY2 = ClippingRect.iBottom;
	}
	else
	{
		ClipX1 = clipregion->iLeft;
		ClipY1 = clipregion->iTop;
		ClipX2 = clipregion->iRight;
		ClipY2 = clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	const INT32 LeftSkip   = __min(ClipX1 -   MIN(ClipX1, iTempX), usWidth);
	INT32       TopSkip    = __min(ClipY1 - __min(ClipY1, iTempY), usHeight);
	const INT32 RightSkip  = __min(  MAX(ClipX2, iTempX + usWidth)  - ClipX2, usWidth);
	const INT32 BottomSkip = __min(__max(ClipY2, iTempY + usHeight) - ClipY2, usHeight);

	// calculate the remaining rows and columns to blit
	const INT32 BlitLength = usWidth  - LeftSkip - RightSkip;
	INT32       BlitHeight = usHeight - TopSkip  - BottomSkip;

	// check if whole thing is clipped
	if (LeftSkip >= usWidth  || RightSkip  >= usWidth)  return;
	if (TopSkip  >= usHeight || BottomSkip >= usHeight) return;

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	UINT8*       DestPtr  = (UINT8*)pBuffer  + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	UINT8*       ZPtr     = (UINT8*)pZBuffer + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	const UINT32 LineSkip = uiDestPitchBYTES - BlitLength * 2;
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();

	if (hSrcVObject->ppZStripInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}
	// setup for the z-column blitting stuff
	const ZStripInfo* const pZInfo = hSrcVObject->ppZStripInfo[usIndex];
	if (pZInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}

	UINT16 usZStartLevel = (INT16)usZValue + pZInfo->bInitialZChange * Z_STRIP_DELTA_Y;
	// set to odd number of pixels for first column

	UINT16 usZStartCols;
	if (LeftSkip > pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = LeftSkip - pZInfo->ubFirstZStripWidth;
		usZStartCols = 20 - usZStartCols % 20;
	}
	else if (LeftSkip < pZInfo->ubFirstZStripWidth)
	{
		usZStartCols  = pZInfo->ubFirstZStripWidth - LeftSkip;
	}
	else
	{
		usZStartCols = 20;
	}

	usZColsToGo = usZStartCols;

	const INT8* const pZArray = pZInfo->pbZChange;

	if (LeftSkip >= pZInfo->ubFirstZStripWidth)
	{
		// Index into array after doing left clipping
		usZStartIndex = 1 + (LeftSkip - pZInfo->ubFirstZStripWidth) / 20;

		//calculates the Z-value after left-side clipping
		if (usZStartIndex)
		{
			for (UINT16 i = 0; i < usZStartIndex; i++)
			{
				switch (pZArray[i])
				{
					case -1: usZStartLevel -= Z_STRIP_DELTA_Y; break;
					case  0: /* no change */                   break;
					case  1: usZStartLevel += Z_STRIP_DELTA_Y; break;
				}
			}
		}
	}
	else
	{
		usZStartIndex = 0;
	}

	usZLevel = usZStartLevel;
	usZIndex = usZStartIndex;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		usZLevel = usZStartLevel;
		usZIndex = usZStartIndex;
		usZColsToGo = usZStartCols;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > LSCount)
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > LSCount) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
				for (;;)
				{
					if (PxCount >= usZColsToGo)
					{
						PxCount -= usZColsToGo;
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
					else
					{
						usZColsToGo -= PxCount;
						break;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > LSCount)
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZLevel)
					{
						*(UINT16*)ZPtr = usZLevel;
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
					if (--usZColsToGo == 0)
					{
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/**********************************************************************************************
Blt8BPPDataTo16BPPBufferTransZIncObscureClip

	Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
	buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
	pixel's Z level is below that of the current pixel, it is written on, and the Z value is
	updated to the current value, for any non-transparent pixels. The Z-buffer is 16 bit, and
	must be the same dimensions (including Pitch) as the destination.

	//ATE: This blitter makes the values that are =< z value pixellate rather than not
	// render at all

**********************************************************************************************/
static void Blt8BPPDataTo16BPPBufferTransZIncObscureClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion)
{
	UINT32 Unblitted;
	INT32  LSCount;
	UINT16 usZLevel, usZColsToGo, usZIndex;

	Assert(hSrcVObject != NULL);
	Assert(pBuffer     != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	INT32       const  usHeight = pTrav.usHeight;
	INT32       const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	INT32 ClipX1;
	INT32 ClipY1;
	INT32 ClipX2;
	INT32 ClipY2;
	if (clipregion == NULL)
	{
		ClipX1 = ClippingRect.iLeft;
		ClipY1 = ClippingRect.iTop;
		ClipX2 = ClippingRect.iRight;
		ClipY2 = ClippingRect.iBottom;
	}
	else
	{
		ClipX1 = clipregion->iLeft;
		ClipY1 = clipregion->iTop;
		ClipX2 = clipregion->iRight;
		ClipY2 = clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	const INT32 LeftSkip   = __min(ClipX1 -   MIN(ClipX1, iTempX), usWidth);
	INT32       TopSkip    = __min(ClipY1 - __min(ClipY1, iTempY), usHeight);
	const INT32 RightSkip  = __min(  MAX(ClipX2, iTempX + usWidth)  - ClipX2, usWidth);
	const INT32 BottomSkip = __min(__max(ClipY2, iTempY + usHeight) - ClipY2, usHeight);

	UINT32 uiLineFlag = iTempY & 1;

	// calculate the remaining rows and columns to blit
	const INT32 BlitLength = usWidth  - LeftSkip - RightSkip;
	INT32       BlitHeight = usHeight - TopSkip  - BottomSkip;

	// check if whole thing is clipped
	if (LeftSkip >= usWidth  || RightSkip  >= usWidth)  return;
	if (TopSkip  >= usHeight || BottomSkip >= usHeight) return;

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	UINT8*       DestPtr  = (UINT8*)pBuffer  + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	UINT8*       ZPtr     = (UINT8*)pZBuffer + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	const UINT32 LineSkip = uiDestPitchBYTES - BlitLength * 2;
	UINT16 const* const p16BPPPalette = hSrcVObject->CurrentShade();

	if (hSrcVObject->ppZStripInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}
	// setup for the z-column blitting stuff
	const ZStripInfo* const pZInfo = hSrcVObject->ppZStripInfo[usIndex];
	if (pZInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}

	UINT16 usZStartLevel = (INT16)usZValue + pZInfo->bInitialZChange * Z_STRIP_DELTA_Y;
	// set to odd number of pixels for first column

	UINT16 usZStartCols;
	if (LeftSkip > pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = LeftSkip - pZInfo->ubFirstZStripWidth;
		usZStartCols = 20 - usZStartCols % 20;
	}
	else if (LeftSkip < pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = pZInfo->ubFirstZStripWidth - LeftSkip;
	}
	else
	{
		usZStartCols = 20;
	}

	usZColsToGo = usZStartCols;

	const INT8* const pZArray  = pZInfo->pbZChange;

	UINT16 usZStartIndex;
	if (LeftSkip >= pZInfo->ubFirstZStripWidth)
	{
		// Index into array after doing left clipping
		usZStartIndex = 1 + (LeftSkip - pZInfo->ubFirstZStripWidth) / 20;

		//calculates the Z-value after left-side clipping
		if (usZStartIndex)
		{
			for (UINT16 i = 0; i < usZStartIndex; i++)
			{
				switch (pZArray[i])
				{
					case -1: usZStartLevel -= Z_STRIP_DELTA_Y; break;
					case  0: /* no change */                   break;
					case  1: usZStartLevel += Z_STRIP_DELTA_Y; break;
				}
			}
		}
	}
	else
	{
		usZStartIndex = 0;
	}

	usZLevel = usZStartLevel;
	usZIndex = usZStartIndex;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		uiLineFlag ^= 1; // XXX evaluate before loop
		TopSkip--;
	}

	do
	{
		usZLevel = usZStartLevel;
		usZIndex = usZStartIndex;
		usZColsToGo = usZStartCols;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > LSCount)
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > LSCount) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
				for (;;)
				{
					if (PxCount >= usZColsToGo)
					{
						PxCount -= usZColsToGo;
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
					else
					{
						usZColsToGo -= PxCount;
						break;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > LSCount)
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZLevel ||
							uiLineFlag == (((uintptr_t)DestPtr & 2) != 0)) // XXX update Z when pixelating?
					{
						*(UINT16*)ZPtr = usZLevel;
						*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
					if (--usZColsToGo == 0)
					{
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		uiLineFlag ^= 1;
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


/* Blitter Specs
	* 1) 8 to 16 bpp
	* 2) strip z-blitter
	* 3) clipped
	* 4) trans shadow - if value is 254, makes a shadow */
static void Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, INT16 sZIndex, const UINT16* p16BPPPalette)
{
	UINT32 Unblitted;
	INT32  LSCount;
	UINT16 usZLevel, usZColsToGo, usZIndex;

	Assert(hSrcVObject != NULL);
	Assert(pBuffer     != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	INT32       const  usHeight = pTrav.usHeight;
	INT32       const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	INT32 ClipX1;
	INT32 ClipY1;
	INT32 ClipX2;
	INT32 ClipY2;
	if (clipregion == NULL)
	{
		ClipX1 = ClippingRect.iLeft;
		ClipY1 = ClippingRect.iTop;
		ClipX2 = ClippingRect.iRight;
		ClipY2 = ClippingRect.iBottom;
	}
	else
	{
		ClipX1 = clipregion->iLeft;
		ClipY1 = clipregion->iTop;
		ClipX2 = clipregion->iRight;
		ClipY2 = clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	const INT32 LeftSkip   = __min(ClipX1 -   MIN(ClipX1, iTempX), usWidth);
	INT32       TopSkip    = __min(ClipY1 - __min(ClipY1, iTempY), usHeight);
	const INT32 RightSkip  = __min(  MAX(ClipX2, iTempX + usWidth)  - ClipX2, usWidth);
	const INT32 BottomSkip = __min(__max(ClipY2, iTempY + usHeight) - ClipY2, usHeight);

	UINT32 uiLineFlag = iTempY & 1;

	// calculate the remaining rows and columns to blit
	const INT32 BlitLength = usWidth - LeftSkip - RightSkip;
	INT32       BlitHeight = usHeight - TopSkip - BottomSkip;

	// check if whole thing is clipped
	if (LeftSkip >= usWidth  || RightSkip  >= usWidth)  return;
	if (TopSkip  >= usHeight || BottomSkip >= usHeight) return;

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	UINT8*       DestPtr = (UINT8*)pBuffer  + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	UINT8*       ZPtr    = (UINT8*)pZBuffer + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	const UINT32 LineSkip = uiDestPitchBYTES - BlitLength * 2;

	if (hSrcVObject->ppZStripInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}
	// setup for the z-column blitting stuff
	const ZStripInfo* const pZInfo = hSrcVObject->ppZStripInfo[sZIndex];
	if (pZInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}

	UINT16 usZStartLevel = (INT16)usZValue + pZInfo->bInitialZChange * Z_SUBLAYERS * 10;

	UINT16 usZStartCols;
	if (LeftSkip > pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = LeftSkip - pZInfo->ubFirstZStripWidth;
		usZStartCols = 20 - usZStartCols % 20;
	}
	else if (LeftSkip < pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = pZInfo->ubFirstZStripWidth - LeftSkip;
	}
	else
	{
		usZStartCols = 20;
	}

	// set to odd number of pixels for first column
	usZColsToGo = usZStartCols;

	const INT8* const pZArray = pZInfo->pbZChange;

	UINT16 usZStartIndex;
	if (LeftSkip >= usZColsToGo)
	{
		// Index into array after doing left clipping
		usZStartIndex = 1 + (LeftSkip - pZInfo->ubFirstZStripWidth) / 20;

		//calculates the Z-value after left-side clipping
		if (usZStartIndex)
		{
			for (UINT16 i = 0; i < usZStartIndex; i++)
			{
				switch (pZArray[i])
				{
					case -1: usZStartLevel -= Z_SUBLAYERS; break;
					case  0: /* no change */               break;
					case  1: usZStartLevel += Z_SUBLAYERS; break;
				}
			}
		}
	}
	else
	{
		usZStartIndex = 0;
	}

	usZLevel = usZStartLevel;
	usZIndex = usZStartIndex;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		usZLevel = usZStartLevel;
		usZIndex = usZStartIndex;
		usZColsToGo = usZStartCols;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > LSCount)
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > LSCount) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
				for (;;)
				{
					if (PxCount >= usZColsToGo)
					{
						PxCount -= usZColsToGo;
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
					else
					{
						usZColsToGo -= PxCount;
						break;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > LSCount)
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr < usZLevel ||
							uiLineFlag == (((uintptr_t)DestPtr & 2) != 0)) // XXX update Z when pixelating?
					{
						*(UINT16*)ZPtr = usZLevel;
						UINT8 Px = *SrcPtr;
						if (Px == 254)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
						else
						{
							*(UINT16*)DestPtr = p16BPPPalette[Px];
						}
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
					if (--usZColsToGo == 0)
					{
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		uiLineFlag ^= 1;
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}

/* Blitter Specs
	* 1) 8 to 16 bpp
	* 2) strip z-blitter
	* 3) clipped
	* 4) trans shadow - if value is 254, makes a shadow */
static void Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(UINT16* pBuffer, UINT32 uiDestPitchBYTES, UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, SGPRect* clipregion, INT16 sZIndex, const UINT16* p16BPPPalette)
{
	UINT32 Unblitted;
	INT32  LSCount;
	UINT16 usZLevel, usZColsToGo, usZIndex;

	Assert(hSrcVObject != NULL);
	Assert(pBuffer     != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	INT32       const  usHeight = pTrav.usHeight;
	INT32       const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	INT32 ClipX1;
	INT32 ClipY1;
	INT32 ClipX2;
	INT32 ClipY2;
	if (clipregion == NULL)
	{
		ClipX1 = ClippingRect.iLeft;
		ClipY1 = ClippingRect.iTop;
		ClipX2 = ClippingRect.iRight;
		ClipY2 = ClippingRect.iBottom;
	}
	else
	{
		ClipX1 = clipregion->iLeft;
		ClipY1 = clipregion->iTop;
		ClipX2 = clipregion->iRight;
		ClipY2 = clipregion->iBottom;
	}

	// Calculate rows hanging off each side of the screen
	const INT32 LeftSkip   = __min(ClipX1 -   MIN(ClipX1, iTempX), usWidth);
	INT32       TopSkip    = __min(ClipY1 - __min(ClipY1, iTempY), usHeight);
	const INT32 RightSkip  = __min(  MAX(ClipX2, iTempX + usWidth)  - ClipX2, usWidth);
	const INT32 BottomSkip = __min(__max(ClipY2, iTempY + usHeight) - ClipY2, usHeight);

	// calculate the remaining rows and columns to blit
	const INT32 BlitLength = usWidth  - LeftSkip - RightSkip;
	INT32       BlitHeight = usHeight - TopSkip  - BottomSkip;

	// check if whole thing is clipped
	if (LeftSkip >= usWidth  || RightSkip  >= usWidth)  return;
	if (TopSkip  >= usHeight || BottomSkip >= usHeight) return;

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	UINT8*       DestPtr  = (UINT8*)pBuffer  + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	UINT8*       ZPtr     = (UINT8*)pZBuffer + uiDestPitchBYTES * (iTempY + TopSkip) + (iTempX + LeftSkip) * 2;
	const UINT32 LineSkip = uiDestPitchBYTES - BlitLength * 2;

	if (hSrcVObject->ppZStripInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}
	// setup for the z-column blitting stuff
	const ZStripInfo* const pZInfo = hSrcVObject->ppZStripInfo[sZIndex];
	if (pZInfo == NULL)
	{
		SLOGW(DEBUG_TAG_RENDERWORLD, "Missing Z-Strip info on multi-Z object");
		return;
	}

	UINT16 usZStartLevel = (INT16)usZValue + pZInfo->bInitialZChange * Z_SUBLAYERS * 10;

	UINT16 usZStartCols;
	if (LeftSkip > pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = LeftSkip - pZInfo->ubFirstZStripWidth;
		usZStartCols = 20 - usZStartCols % 20;
	}
	else if (LeftSkip < pZInfo->ubFirstZStripWidth)
	{
		usZStartCols = pZInfo->ubFirstZStripWidth - LeftSkip;
	}
	else
	{
		usZStartCols = 20;
	}

	// set to odd number of pixels for first column
	usZColsToGo = usZStartCols;

	const INT8* const pZArray = pZInfo->pbZChange;

	UINT16 usZStartIndex;
	if (LeftSkip >= usZColsToGo)
	{
		// Index into array after doing left clipping
		usZStartIndex = 1 + (LeftSkip - pZInfo->ubFirstZStripWidth) / 20;

		// calculates the Z-value after left-side clipping
		if (usZStartIndex)
		{
			for (UINT16 i = 0; i < usZStartIndex; i++)
			{
				switch (pZArray[i])
				{
					case -1: usZStartLevel -= Z_SUBLAYERS; break;
					case  0: /* no change */               break;
					case  1: usZStartLevel += Z_SUBLAYERS; break;
				}
			}
		}
	}
	else
	{
		usZStartIndex = 0;
	}

	usZLevel = usZStartLevel;
	usZIndex = usZStartIndex;

	UINT32 PxCount;

	while (TopSkip > 0)
	{
		for (;;)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80) continue;
			if (PxCount == 0) break;
			SrcPtr += PxCount;
		}
		TopSkip--;
	}

	do
	{
		usZLevel = usZStartLevel;
		usZIndex = usZStartIndex;
		usZColsToGo = usZStartCols;
		for (LSCount = LeftSkip; LSCount > 0; LSCount -= PxCount)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
				if (PxCount > LSCount)
				{
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitTransparent;
				}
			}
			else
			{
				if (PxCount > LSCount)
				{
					SrcPtr += LSCount;
					PxCount -= LSCount;
					LSCount = BlitLength;
					goto BlitNonTransLoop;
				}
				SrcPtr += PxCount;
			}
		}

		LSCount = BlitLength;
		while (LSCount > 0)
		{
			PxCount = *SrcPtr++;
			if (PxCount & 0x80)
			{
BlitTransparent: // skip transparent pixels
				PxCount &= 0x7F;
				if (PxCount > LSCount) PxCount = LSCount;
				LSCount -= PxCount;
				DestPtr += 2 * PxCount;
				ZPtr    += 2 * PxCount;
				for (;;)
				{
					if (PxCount >= usZColsToGo)
					{
						PxCount -= usZColsToGo;
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_STRIP_DELTA_Y;
						}
						else if (delta > 0)
						{
							usZLevel += Z_STRIP_DELTA_Y;
						}
					}
					else
					{
						usZColsToGo -= PxCount;
						break;
					}
				}
			}
			else
			{
BlitNonTransLoop: // blit non-transparent pixels
				if (PxCount > LSCount)
				{
					Unblitted = PxCount - LSCount;
					PxCount = LSCount;
				}
				else
				{
					Unblitted = 0;
				}
				LSCount -= PxCount;

				do
				{
					if (*(UINT16*)ZPtr <= usZLevel)
					{
						*(UINT16*)ZPtr = usZLevel;

						UINT32 Px = *SrcPtr;
						if (Px == 254)
						{
							*(UINT16*)DestPtr = ShadeTable[*(UINT16*)DestPtr];
						}
						else
						{
							*(UINT16*)DestPtr = p16BPPPalette[*SrcPtr];
						}
					}
					SrcPtr++;
					DestPtr += 2;
					ZPtr += 2;
					if (--usZColsToGo == 0)
					{
						usZColsToGo = 20;

						INT8 delta = pZArray[usZIndex++];
						if (delta < 0)
						{
							usZLevel -= Z_SUBLAYERS;
						}
						else if (delta > 0)
						{
							usZLevel += Z_SUBLAYERS;
						}
					}
				}
				while (--PxCount > 0);
				SrcPtr += Unblitted;
			}
		}

		while (*SrcPtr++ != 0) {} // skip along until we hit and end-of-line marker
		DestPtr += LineSkip;
		ZPtr += LineSkip;
	}
	while (--BlitHeight > 0);
}


static void RenderRoomInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS)
{
	INT16 sAnchorPosX_M = sStartPointX_M;
	INT16 sAnchorPosY_M = sStartPointY_M;
	INT16 sAnchorPosX_S = sStartPointX_S;
	INT16 sAnchorPosY_S = sStartPointY_S;

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	BOOLEAN bXOddFlag = FALSE;
	do
	{
		INT16 sTempPosX_M = sAnchorPosX_M;
		INT16 sTempPosY_M = sAnchorPosY_M;
		INT16 sTempPosX_S = sAnchorPosX_S;
		INT16 sTempPosY_S = sAnchorPosY_S;

		if (bXOddFlag) sTempPosX_S += 20;

		do
		{
			const UINT16 usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);
			if (usTileIndex < GRIDSIZE)
			{
				const INT16 sX = sTempPosX_S + WORLD_TILE_X / 2 - 5;
				INT16       sY = sTempPosY_S + WORLD_TILE_Y / 2 - 5;

				// THIS ROOM STUFF IS ONLY DONE IN THE EDITOR...
				// ADJUST BY SHEIGHT
				sY -= gpWorldLevelData[usTileIndex].sHeight;

				if (gubWorldRoomInfo[usTileIndex] != NO_ROOM)
				{
					SetFont(SMALLCOMPFONT);
					SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, gsVIEWPORT_END_Y);
					switch (gubWorldRoomInfo[usTileIndex] % 5)
					{
						case 0: SetFontForeground(FONT_GRAY3);   break;
						case 1: SetFontForeground(FONT_YELLOW);  break;
						case 2: SetFontForeground(FONT_LTRED);   break;
						case 3: SetFontForeground(FONT_LTBLUE);  break;
						case 4: SetFontForeground(FONT_LTGREEN); break;
					}
					mprintf_buffer(pDestBuf, uiDestPitchBYTES, sX, sY, L"%d", gubWorldRoomInfo[usTileIndex]);
					SetFontDestBuffer(FRAME_BUFFER);
				}
			}

			sTempPosX_S += 40;
			sTempPosX_M++;
			sTempPosY_M--;
		}
		while (sTempPosX_S < sEndXS);

		if (bXOddFlag)
		{
			sAnchorPosY_M++;
		}
		else
		{
			sAnchorPosX_M++;
		}

		bXOddFlag = !bXOddFlag;
		sAnchorPosY_S += 10;
	}
	while (sAnchorPosY_S < sEndYS);
}


#ifdef _DEBUG

static void RenderFOVDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS)
{
	INT16 sAnchorPosX_M = sStartPointX_M;
	INT16 sAnchorPosY_M = sStartPointY_M;
	INT16 sAnchorPosX_S = sStartPointX_S;
	INT16 sAnchorPosY_S = sStartPointY_S;

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	BOOLEAN bXOddFlag = FALSE;
	do
	{
		INT16 sTempPosX_M = sAnchorPosX_M;
		INT16 sTempPosY_M = sAnchorPosY_M;
		INT16 sTempPosX_S = sAnchorPosX_S;
		INT16 sTempPosY_S = sAnchorPosY_S;

		if (bXOddFlag) sTempPosX_S += 20;

		do
		{
			const UINT16 usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);
			if (usTileIndex < GRIDSIZE)
			{
				const INT16 sX = sTempPosX_S + WORLD_TILE_X / 2 - 5;
				INT16       sY = sTempPosY_S + WORLD_TILE_Y / 2 - 5;

				// Adjust for interface level
				sY -= gpWorldLevelData[usTileIndex].sHeight;
				sY += gsRenderHeight;

				if (gubFOVDebugInfoInfo[usTileIndex] != 0)
				{
					SetFont(SMALLCOMPFONT);
					SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, gsVIEWPORT_END_Y);
					SetFontForeground(FONT_GRAY3);
					mprintf_buffer(pDestBuf, uiDestPitchBYTES, sX, sY, L"%d", gubFOVDebugInfoInfo[usTileIndex]);
					SetFontDestBuffer(FRAME_BUFFER);

					Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, gTileDatabase[0].hTileSurface, sTempPosX_S, sTempPosY_S, 0, &gClippingRect);
				}

				if (gubGridNoMarkers[usTileIndex] == gubGridNoValue)
				{
					SetFont(SMALLCOMPFONT);
					SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, gsVIEWPORT_END_Y);
					SetFontForeground(FONT_FCOLOR_YELLOW);
					MPrintBuffer(pDestBuf, uiDestPitchBYTES, sX, sY + 4, L"x");
					SetFontDestBuffer(FRAME_BUFFER);
				}
			}

			sTempPosX_S += 40;
			sTempPosX_M++;
			sTempPosY_M--;
		}
		while (sTempPosX_S < sEndXS);

		if (bXOddFlag)
		{
			sAnchorPosY_M++;
		}
		else
		{
			sAnchorPosX_M++;
		}

		bXOddFlag = !bXOddFlag;
		sAnchorPosY_S += 10;
	}
	while (sAnchorPosY_S < sEndYS);
}


static void RenderCoverDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS)
{
	INT16 sAnchorPosX_M = sStartPointX_M;
	INT16 sAnchorPosY_M = sStartPointY_M;
	INT16 sAnchorPosX_S = sStartPointX_S;
	INT16 sAnchorPosY_S = sStartPointY_S;

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	BOOLEAN bXOddFlag = FALSE;
	do
	{
		INT16 sTempPosX_M = sAnchorPosX_M;
		INT16 sTempPosY_M = sAnchorPosY_M;
		INT16 sTempPosX_S = sAnchorPosX_S;
		INT16 sTempPosY_S = sAnchorPosY_S;

		if (bXOddFlag) sTempPosX_S += 20;

		do
		{
			const UINT16 usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);
			if (usTileIndex < GRIDSIZE)
			{
				const INT16 sX = sTempPosX_S + WORLD_TILE_X / 2 - 5;
				INT16       sY = sTempPosY_S + WORLD_TILE_Y / 2 - 5;

				// Adjust for interface level
				sY -= gpWorldLevelData[usTileIndex].sHeight;
				sY += gsRenderHeight;

				if (gsCoverValue[usTileIndex] != 0x7F7F)
				{
					SetFont(SMALLCOMPFONT);
					SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, gsVIEWPORT_END_Y);
					if (usTileIndex == gsBestCover)
					{
						SetFontForeground(FONT_MCOLOR_RED);
					}
					else if (gsCoverValue[usTileIndex] < 0)
					{
						SetFontForeground(FONT_MCOLOR_WHITE);
					}
					else
					{
						SetFontForeground(FONT_GRAY3);
					}
					mprintf_buffer(pDestBuf, uiDestPitchBYTES, sX, sY, L"%d", gsCoverValue[usTileIndex]);
					SetFontDestBuffer(FRAME_BUFFER);
				}
			}

			sTempPosX_S += 40;
			sTempPosX_M++;
			sTempPosY_M--;
		}
		while (sTempPosX_S < sEndXS);

		if (bXOddFlag)
		{
			sAnchorPosY_M++;
		}
		else
		{
			sAnchorPosX_M++;
		}

		bXOddFlag = !bXOddFlag;
		sAnchorPosY_S += 10;
	}
	while (sAnchorPosY_S < sEndYS);
}


static void RenderGridNoVisibleDebugInfo(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS)
{
	INT16 sAnchorPosX_M = sStartPointX_M;
	INT16 sAnchorPosY_M = sStartPointY_M;
	INT16 sAnchorPosX_S = sStartPointX_S;
	INT16 sAnchorPosY_S = sStartPointY_S;

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	BOOLEAN bXOddFlag = FALSE;
	do
	{
		INT16 sTempPosX_M = sAnchorPosX_M;
		INT16 sTempPosY_M = sAnchorPosY_M;
		INT16 sTempPosX_S = sAnchorPosX_S;
		INT16 sTempPosY_S = sAnchorPosY_S;

		if (bXOddFlag) sTempPosX_S += 20;

		do
		{
			const UINT16 usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);
			if (usTileIndex < GRIDSIZE)
			{
				const INT16 sX = sTempPosX_S + WORLD_TILE_X / 2 - 5;
				INT16       sY = sTempPosY_S + WORLD_TILE_Y / 2 - 5;

				// Adjust for interface level
				sY -= gpWorldLevelData[usTileIndex].sHeight;
				sY += gsRenderHeight;

				SetFont(SMALLCOMPFONT);
				SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, gsVIEWPORT_END_Y);

				if (!GridNoOnVisibleWorldTile(usTileIndex))
				{
					SetFontForeground(FONT_MCOLOR_RED);
				}
				else
				{
					SetFontForeground(FONT_GRAY3);
				}
				mprintf_buffer(pDestBuf, uiDestPitchBYTES, sX, sY, L"%d", usTileIndex);
				SetFontDestBuffer(FRAME_BUFFER);
			}

			sTempPosX_S += 40;
			sTempPosX_M++;
			sTempPosY_M--;
		}
		while (sTempPosX_S < sEndXS);

		if (bXOddFlag)
		{
			sAnchorPosY_M++;
		}
		else
		{
			sAnchorPosX_M++;
		}

		bXOddFlag = !bXOddFlag;
		sAnchorPosY_S += 10;
	}
	while (sAnchorPosY_S < sEndYS);
}

#endif


static void ExamineZBufferForHiddenTiles(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS);


static void ExamineZBufferRect(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
	CalcRenderParameters(sLeft, sTop, sRight, sBottom);
	ExamineZBufferForHiddenTiles(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
}


static BOOLEAN IsTileRedundant(UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex);


static void ExamineZBufferForHiddenTiles(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS)
{
	// Begin Render Loop
	INT16 sAnchorPosX_M = sStartPointX_M;
	INT16 sAnchorPosY_M = sStartPointY_M;
	INT16 sAnchorPosX_S = sStartPointX_S;
	INT16 sAnchorPosY_S = sStartPointY_S;

	// Get VObject for firt land peice!
	const TILE_ELEMENT* const TileElem = &gTileDatabase[FIRSTTEXTURE1];

	BOOLEAN bXOddFlag = FALSE;
	do
	{
		INT16       sTempPosX_M = sAnchorPosX_M;
		INT16       sTempPosY_M = sAnchorPosY_M;
		INT16       sTempPosX_S = sAnchorPosX_S;
		const INT16 sTempPosY_S = sAnchorPosY_S;

		if (bXOddFlag) sTempPosX_S += 20;

		do
		{
			const UINT16 usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);
			if (usTileIndex < GRIDSIZE)
			{
				// ATE: Don;t let any vehicle sit here....
				if (FindStructure(usTileIndex, STRUCTURE_MOBILE))
				{
					// Continue...
					goto ENDOFLOOP;
				}

				const INT16 sX = sTempPosX_S;
				INT16       sY = sTempPosY_S - gpWorldLevelData[usTileIndex].sHeight;

				// Adjust for interface level
				sY += gsRenderHeight;

				// Caluluate zvalue
				// Look for anything less than struct layer!
				INT16 sWorldX;
				INT16 sZLevel;
				GetAbsoluteScreenXYFromMapPos(usTileIndex, &sWorldX, &sZLevel);

				sZLevel += gsRenderHeight;
				sZLevel  = sZLevel * Z_SUBLAYERS + STRUCT_Z_LEVEL;

				if (gpWorldLevelData[usTileIndex].uiFlags & MAPELEMENT_REEVALUATE_REDUNDENCY)
				{
					const INT8 bBlitClipVal = BltIsClippedOrOffScreen(TileElem->hTileSurface, sX, sY, TileElem->usRegionIndex, &gClippingRect);
					if (bBlitClipVal == FALSE)
					{
						// Set flag to not evaluate again!
						gpWorldLevelData[usTileIndex].uiFlags &= ~MAPELEMENT_REEVALUATE_REDUNDENCY;

						if (IsTileRedundant(gpZBuffer, sZLevel, TileElem->hTileSurface, sX, sY, TileElem->usRegionIndex))
						{
							// Mark in the world!
							gpWorldLevelData[usTileIndex].uiFlags |= MAPELEMENT_REDUNDENT;
						}
						else
						{
							// Un Mark in the world!
							gpWorldLevelData[usTileIndex].uiFlags &= ~MAPELEMENT_REDUNDENT;
						}
					}
				}
			}

ENDOFLOOP:
			sTempPosX_S += 40;
			sTempPosX_M++;
			sTempPosY_M--;
		} while (sTempPosX_S < sEndXS);

		if (bXOddFlag)
		{
			++sAnchorPosY_M;
		}
		else
		{
			++sAnchorPosX_M;
		}

		bXOddFlag = !bXOddFlag;
		sAnchorPosY_S += 10;
	}
	while (sAnchorPosY_S < sEndYS);
}


static void CalcRenderParameters(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
	INT16 sTempPosX_W, sTempPosY_W;

	gOldClipRect = gClippingRect;

	// Set new clipped rect
	gClippingRect.iLeft   = __max(gsVIEWPORT_START_X,        sLeft);
	gClippingRect.iRight  = __min(gsVIEWPORT_END_X,          sRight);
	gClippingRect.iTop    = __max(gsVIEWPORT_WINDOW_START_Y, sTop);
	gClippingRect.iBottom = __min(gsVIEWPORT_WINDOW_END_Y,   sBottom);

	gsEndXS = sRight  + VIEWPORT_XOFFSET_S;
	gsEndYS = sBottom + VIEWPORT_YOFFSET_S;

	const INT16 sRenderCenterX_W = gsRenderCenterX;
	const INT16 sRenderCenterY_W = gsRenderCenterY;

	// STEP THREE - determine starting point in world coords
	// a) Determine where in screen coords to start rendering
	gsStartPointX_S = g_ui.m_tacticalMapCenterX - (sLeft - VIEWPORT_XOFFSET_S);
	gsStartPointY_S = g_ui.m_tacticalMapCenterY - (sTop  - VIEWPORT_YOFFSET_S);


	// b) Convert these distances into world distances
	FromScreenToCellCoordinates(gsStartPointX_S, gsStartPointY_S, &sTempPosX_W, &sTempPosY_W);

	// c) World start point is Render center minus this distance
	gsStartPointX_W = sRenderCenterX_W - sTempPosX_W;
	gsStartPointY_W = sRenderCenterY_W - sTempPosY_W;

	// NOTE: Increase X map value by 1 tile to offset where on screen we are...
	if (gsStartPointX_W > 0) gsStartPointX_W += CELL_X_SIZE;

	// d) screen start point is screen distances minus screen center
	gsStartPointX_S = sLeft - VIEWPORT_XOFFSET_S;
	gsStartPointY_S = sTop  - VIEWPORT_YOFFSET_S;

	// STEP FOUR - Determine Start block
	// a) Find start block
	gsStartPointX_M = gsStartPointX_W / CELL_X_SIZE;
	gsStartPointY_M = gsStartPointY_W / CELL_Y_SIZE;

	// STEP 5 - Determine Deltas for center and find screen values
	//Make sure these coordinates are multiples of scroll steps
	const INT16 sOffsetX_W = ABS(gsStartPointX_W) - ABS(gsStartPointX_M * CELL_X_SIZE);
	const INT16 sOffsetY_W = ABS(gsStartPointY_W) - ABS(gsStartPointY_M * CELL_Y_SIZE);

	INT16 sOffsetX_S;
	INT16 sOffsetY_S;
	FromCellToScreenCoordinates(sOffsetX_W, sOffsetY_W, &sOffsetX_S, &sOffsetY_S);

	if (gsStartPointY_W < 0)
	{
		gsStartPointY_S += 0;
	}
	else
	{
		gsStartPointY_S -= sOffsetY_S;
	}
	gsStartPointX_S -= sOffsetX_S;

	/////////////////////////////////////////
	//ATE: CALCULATE LARGER OFFSET VALUES
	gsLEndXS = sRight  + LARGER_VIEWPORT_XOFFSET_S;
	gsLEndYS = sBottom + LARGER_VIEWPORT_YOFFSET_S;

		// STEP THREE - determine starting point in world coords
	// a) Determine where in screen coords to start rendering
	gsLStartPointX_S = g_ui.m_tacticalMapCenterX - (sLeft - LARGER_VIEWPORT_XOFFSET_S);
	gsLStartPointY_S = g_ui.m_tacticalMapCenterY - (sTop  - LARGER_VIEWPORT_YOFFSET_S);


	// b) Convert these distances into world distances
	FromScreenToCellCoordinates(gsLStartPointX_S, gsLStartPointY_S, &sTempPosX_W, &sTempPosY_W);

	// c) World start point is Render center minus this distance
	gsLStartPointX_W = sRenderCenterX_W - sTempPosX_W;
	gsLStartPointY_W = sRenderCenterY_W - sTempPosY_W;

	// NOTE: Increase X map value by 1 tile to offset where on screen we are...
	if (gsLStartPointX_W > 0) gsLStartPointX_W += CELL_X_SIZE;

	// d) screen start point is screen distances minus screen center
	gsLStartPointX_S = sLeft - LARGER_VIEWPORT_XOFFSET_S;
	gsLStartPointY_S = sTop  - LARGER_VIEWPORT_YOFFSET_S;

	// STEP FOUR - Determine Start block
	// a) Find start block
	gsLStartPointX_M = gsLStartPointX_W / CELL_X_SIZE;
	gsLStartPointY_M = gsLStartPointY_W / CELL_Y_SIZE;

	// Adjust starting screen coordinates
	gsLStartPointX_S -= sOffsetX_S;

	if (gsLStartPointY_W < 0)
	{
		gsLStartPointY_S +=  0;
		gsLStartPointX_S -= 20;
	}
	else
	{
		gsLStartPointY_S -= sOffsetY_S;
	}
}


static void ResetRenderParameters(void)
{
	// Restore clipping rect
	gClippingRect = gOldClipRect;
}


static BOOLEAN IsTileRedundant(UINT16* pZBuffer, UINT16 usZValue, HVOBJECT hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex)
{
	BOOLEAN fHidden = TRUE;

	Assert(hSrcVObject != NULL);

	// Get Offsets from Index into structure
	ETRLEObject const& pTrav    = hSrcVObject->SubregionProperties(usIndex);
	UINT32             usHeight = pTrav.usHeight;
	UINT32      const  usWidth  = pTrav.usWidth;

	// Add to start position of dest buffer
	INT32 const iTempX = iX + pTrav.sOffsetX;
	INT32 const iTempY = iY + pTrav.sOffsetY;

	CHECKF(iTempX >= 0);
	CHECKF(iTempY >= 0);

	UINT8 const* SrcPtr   = hSrcVObject->PixData(pTrav);
	const UINT8* ZPtr     = (const UINT8*)(pZBuffer + iTempY * SCREEN_WIDTH + iTempX);
	const UINT32 LineSkip = (SCREEN_WIDTH - usWidth) * 2;

	do
	{
		for (;;)
		{
			UINT8 data = *SrcPtr++;

			if (data == 0) break;
			if (data & 0x80)
			{
				data &= 0x7F;
				ZPtr += 2 * data;
			}
			else
			{
				SrcPtr += data;
				do
				{
					if (*(const UINT16*)ZPtr < usZValue) return FALSE;
					ZPtr += 2;
				}
				while (--data > 0);
			}
		}
		ZPtr += LineSkip;
	}
	while (--usHeight > 0);
	return fHidden;
}


void SetRenderCenter(INT16 sNewX, INT16 sNewY)
{
	if (gfIgnoreScrolling) return;

	// Apply these new coordinates to the renderer!
	ApplyScrolling(sNewX, sNewY, TRUE, FALSE);

	// Set flag to ignore scrolling this frame
	gfIgnoreScrollDueToCenterAdjust = TRUE;

	// Set full render flag!
	// DIRTY THE WORLD!
	SetRenderFlags(RENDER_FLAG_FULL);

	gfPlotNewMovement = TRUE;

	if (gfScrollPending)
	{
		// Do a complete rebuild!
		gfScrollPending = FALSE;

		// Restore Interface!
		RestoreInterface();

		DeleteVideoOverlaysArea();
	}

	g_scroll_inertia = false;
}


#ifdef _DEBUG

void RenderFOVDebug(void)
{
	RenderFOVDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
}

void RenderCoverDebug(void)
{
	RenderCoverDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS, gsEndYS);
}

#endif


#ifdef WITH_UNITTESTS
#undef FAIL
#include "gtest/gtest.h"

TEST(RenderWorld, asserts)
{
	EXPECT_EQ(lengthof(RenderFX), NUM_RENDER_FX_TYPES);
	EXPECT_EQ(lengthof(RenderFXStartIndex), NUM_RENDER_FX_TYPES);
	EXPECT_EQ(lengthof(g_render_fx_layer_flags), NUM_RENDER_FX_TYPES);
}

#endif
