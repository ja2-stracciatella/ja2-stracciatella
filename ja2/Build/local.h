#ifndef __LOCAL_DEFINES_
#define __LOCAL_DEFINES_

#include "Screens.h"

//
// Make sure the application has a name
//

#define APPLICATION_NAME      "Jagged Alliance 2"

//
// Basic defines for the video manager. These represent the starting values
//

#ifdef BOUNDS_CHECKER
	#define		WINDOWED_MODE
#endif

#ifdef _DEBUG
 #define WINDOWED_MODE
#endif


#define SCREEN_WIDTH          640
#define SCREEN_HEIGHT         480
#define PIXEL_DEPTH           16

//
// These defines are used as MUTEX handles.
//

#define MAX_MUTEX_HANDLES     32

#define REFRESH_THREAD_MUTEX  0
#define FRAME_BUFFER_MUTEX    1
#define MOUSE_BUFFER_MUTEX    2
#define DIRTY_BUFFER_MUTEX    3
#define SCROLL_MESSAGE_MUTEX  4


#endif
