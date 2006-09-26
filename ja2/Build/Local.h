#ifndef __LOCAL_DEFINES_
#define __LOCAL_DEFINES_

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

#endif
