#ifndef __VIDEO_
#define __VIDEO_

#include <SDL.h>
#include "Types.h"

#define BUFFER_READY          0x00
#define BUFFER_BUSY           0x01
#define BUFFER_DIRTY          0x02
#define BUFFER_DISABLED       0x03

#define MAX_CURSOR_WIDTH      64
#define MAX_CURSOR_HEIGHT     64
#define VIDEO_NO_CURSOR				0xFFFF


#ifdef __cplusplus
extern "C" {
#endif

void    VideoSetFullScreen(BOOLEAN Enable);
BOOLEAN InitializeVideoManager(void);
extern void                 ShutdownVideoManager(void);
extern void                 SuspendVideoManager(void);
extern BOOLEAN              RestoreVideoManager(void);
extern void                 GetCurrentVideoSettings(UINT16* usWidth, UINT16* usHeight);
extern void                 InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);
extern void                 InvalidateScreen(void);
extern SDL_Surface* GetBackBufferObject(void);
extern SDL_Surface* GetFrameBufferObject(void);
extern SDL_Surface* GetMouseBufferObject(void);
extern PTR                  LockBackBuffer(UINT32 *uiPitch);
extern void                 UnlockBackBuffer(void);
extern PTR                  LockFrameBuffer(UINT32 *uiPitch);
extern void                 UnlockFrameBuffer(void);
extern PTR                  LockMouseBuffer(UINT32 *uiPitch);
extern void                 UnlockMouseBuffer(void);
extern BOOLEAN              GetPrimaryRGBDistributionMasks(UINT32 *RedBitMask, UINT32 *GreenBitMask, UINT32 *BblueBitMask);
extern void                 StartFrameBufferRender(void);
extern void                 EndFrameBufferRender(void);
extern void                 PrintScreen(void);

extern BOOLEAN							EraseMouseCursor( );
extern BOOLEAN							SetMouseCursorProperties( INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth );
void												DirtyCursor( );

void												VideoCaptureToggle( void );


void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);

void RefreshScreen(void);

void FatalError(const char *pError, ...);

#ifdef __cplusplus
}
#endif

#endif
