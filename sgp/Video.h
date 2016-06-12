#ifndef VIDEO_H
#define VIDEO_H

#include <SDL_video.h>
#include "Types.h"


#define VIDEO_NO_CURSOR 0xFFFF


void         VideoSetFullScreen(BOOLEAN enable);
void         InitializeVideoManager(void);
void         ShutdownVideoManager(void);
void         SuspendVideoManager(void);
void         InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
void         InvalidateScreen(void);
void         GetPrimaryRGBDistributionMasks(UINT32* RedBitMask, UINT32* GreenBitMask, UINT32* BlueBitMask);
void         EndFrameBufferRender(void);
void         PrintScreen(void);

/* Toggle between fullscreen and window mode after initialising the video
 * manager */
void VideoToggleFullScreen(void);

void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth);

void VideoCaptureToggle(void);

void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);

void RefreshScreen(void);

// Creates a list to contain video Surfaces
void InitializeVideoSurfaceManager(void);

// Deletes any video Surface placed into list
void ShutdownVideoSurfaceManager(void);

#endif
