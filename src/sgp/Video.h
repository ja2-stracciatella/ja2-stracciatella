#ifndef VIDEO_H
#define VIDEO_H

#include <SDL_video.h>
#include "Types.h"
#include "stracciatella.h"


#define VIDEO_NO_CURSOR 0xFFFF
#define GAME_WINDOW g_game_window

extern SDL_Window* g_game_window;

using VideoScaleQuality = ScalingQuality;

void         VideoSetFullScreen(BOOLEAN enable);
void         InitializeVideoManager(VideoScaleQuality quality);
void         ShutdownVideoManager(void);
void         SuspendVideoManager(void);
void         InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
void         InvalidateScreen(void);
void         GetPrimaryRGBDistributionMasks(UINT32* RedBitMask, UINT32* GreenBitMask, UINT32* BlueBitMask);
void         EndFrameBufferRender(void);

void VideoSetBrightness(float brightness);

/* Toggle between fullscreen and window mode after initialising the video
 * manager */
void VideoToggleFullScreen(void);

void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth);

void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);

void RefreshScreen(void);

// Creates a list to contain video Surfaces
void InitializeVideoSurfaceManager(void);

// Deletes any video Surface placed into list
void ShutdownVideoSurfaceManager(void);

#endif
