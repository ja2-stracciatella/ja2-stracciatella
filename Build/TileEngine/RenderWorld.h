#ifndef RENDERWORLD_H
#define RENDERWORLD_H

extern BOOLEAN gfDoVideoScroll;
extern UINT8   gubCurScrollSpeedID;

// RENDERING FLAGS
#define RENDER_FLAG_FULL     0x00000001
#define RENDER_FLAG_SHADOWS  0x00000002
#define RENDER_FLAG_MARKED   0x00000004
#define RENDER_FLAG_SAVEOFF  0x00000008
#define RENDER_FLAG_NOZ      0x00000010
#define RENDER_FLAG_ROOMIDS  0x00000020
#define RENDER_FLAG_CHECKZ   0x00000040
#define RENDER_FLAG_FOVDEBUG 0x00000200

#define SCROLL_UP    0x00000001
#define SCROLL_DOWN  0x00000002
#define SCROLL_RIGHT 0x00000004
#define SCROLL_LEFT  0x00000008


#define Z_SUBLAYERS         8
#define LAND_Z_LEVEL        0
#define OBJECT_Z_LEVEL      1
#define SHADOW_Z_LEVEL      2
#define MERC_Z_LEVEL        3
#define STRUCT_Z_LEVEL      4
#define ROOF_Z_LEVEL        5
#define ONROOF_Z_LEVEL      6
#define TOPMOST_Z_LEVEL 32767


enum RenderLayerFlags
{
	// Highest bit value is rendered first
	TILES_LAYER_ALL            = 0xFFFFFFFF,
	TILES_STATIC_LAND          = 0x00040000,
	TILES_STATIC_OBJECTS       = 0x00020000,
	TILES_STATIC_SHADOWS       = 0x00008000,
	TILES_STATIC_STRUCTURES    = 0x00004000,
	TILES_STATIC_ROOF          = 0x00002000,
	TILES_STATIC_ONROOF        = 0x00001000,
	TILES_STATIC_TOPMOST       = 0x00000800,
	TILES_ALL_DYNAMICS         = 0x00000FFF,
	TILES_DYNAMIC_LAND         = 0x00000200,
	TILES_DYNAMIC_OBJECTS      = 0x00000100,
	TILES_DYNAMIC_SHADOWS      = 0x00000080,
	TILES_DYNAMIC_STRUCT_MERCS = 0x00000040,
	TILES_DYNAMIC_MERCS        = 0x00000020,
	TILES_DYNAMIC_STRUCTURES   = 0x00000010,
	TILES_DYNAMIC_ROOF         = 0x00000008,
	TILES_DYNAMIC_HIGHMERCS    = 0x00000004,
	TILES_DYNAMIC_ONROOF       = 0x00000002,
	TILES_DYNAMIC_TOPMOST      = 0x00000001,
	TILES_LAYER_NONE           = 0
};
ENUM_BITSET(RenderLayerFlags)


extern INT16 gsScrollXIncrement;
extern INT16 gsScrollYIncrement;
extern INT16 gsRenderHeight;


extern const INT16 gsVIEWPORT_START_X;
extern const INT16 gsVIEWPORT_START_Y;
extern const INT16 gsVIEWPORT_END_X;
extern const INT16 gsVIEWPORT_END_Y;
extern INT16       gsVIEWPORT_WINDOW_START_Y;
extern INT16       gsVIEWPORT_WINDOW_END_Y;

extern INT16 gsRenderCenterX;
extern INT16 gsRenderCenterY;
extern INT16 gsRenderWorldOffsetX;
extern INT16 gsRenderWorldOffsetY;

// CURRENT VIEWPORT IN WORLD COORDS
extern INT16 gsTopLeftWorldX;
extern INT16 gsTopLeftWorldY;
extern INT16 gsBottomRightWorldX;
extern INT16 gsBottomRightWorldY;


// GLOBAL COORDINATES
extern INT16 gCenterWorldX;
extern INT16 gCenterWorldY;
extern INT16 gsTLX;
extern INT16 gsTLY;
extern INT16 gsTRX;
extern INT16 gsTRY;
extern INT16 gsBLX;
extern INT16 gsBLY;
extern INT16 gsBRX;
extern INT16 gsBRY;
extern INT16 gsCX;
extern INT16 gsCY;
extern double gdScaleX;
extern double gdScaleY;

extern BOOLEAN gfIgnoreScrollDueToCenterAdjust;


void ScrollWorld(void);
void InitRenderParams(UINT8 ubRestrictionID);
void RenderWorld(void);

void ResetSpecificLayerOptimizing(RenderLayerFlags);

void SetRenderFlags(UINT32 uiFlags);
void ClearRenderFlags(UINT32 uiFlags);

void RenderSetShadows(BOOLEAN fShadows);

extern UINT16* gpZBuffer;
extern BOOLEAN gfIgnoreScrolling;

extern BOOLEAN gfScrollInertia;
extern BOOLEAN gfScrollPending;

void RenderStaticWorldRect(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom, BOOLEAN fDynamicsToo);

void InvalidateWorldRedundency(void);

void SetRenderCenter(INT16 sNewX, INT16 sNewY);

#if defined _DEBUG
void RenderFOVDebug(void);
void RenderCoverDebug(void);
#endif

#endif
