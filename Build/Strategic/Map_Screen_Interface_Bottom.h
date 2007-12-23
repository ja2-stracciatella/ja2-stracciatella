#ifndef __MAP_INTERFACE_BOTTOM
#define __MAP_INTERFACE_BOTTOM

#include "Types.h"


#define MAX_MESSAGES_ON_MAP_BOTTOM  9


// exit to where defines
enum{
	MAP_EXIT_TO_LAPTOP = 0,
	MAP_EXIT_TO_TACTICAL,
	MAP_EXIT_TO_OPTIONS,
	MAP_EXIT_TO_LOAD,
	MAP_EXIT_TO_SAVE,
};

// there's no button for entering SAVE/LOAD screen directly...
extern UINT32 guiMapBottomExitButtons[ 3 ];

extern BOOLEAN fLapTop;
extern BOOLEAN fLeavingMapScreen;
extern BOOLEAN gfDontStartTransitionFromLaptop;
extern BOOLEAN gfStartMapScreenToLaptopTransition;


// function prototypes

BOOLEAN LoadMapScreenInterfaceBottom( void );
void DeleteMapScreenInterfaceBottom( void );
void RenderMapScreenInterfaceBottom( void );

// delete map bottom graphics
void DeleteMapBottomGraphics( void );

// load bottom graphics
void HandleLoadOfMapBottomGraphics( void );

// allowed to time compress?
BOOLEAN AllowedToTimeCompress( void );

void EnableDisAbleMapScreenOptionsButton( BOOLEAN fEnable );

// create and destroy masks to cover the time compression buttons as needed
void CreateDestroyMouseRegionMasksForTimeCompressionButtons( void );

BOOLEAN CommonTimeCompressionChecks( void );

BOOLEAN AnyUsableRealMercenariesOnTeam( void );

void RequestTriggerExitFromMapscreen( INT8 bExitToWhere );
BOOLEAN AllowedToExitFromMapscreenTo( INT8 bExitToWhere );
void HandleExitsFromMapScreen( void );

void MapScreenMsgScrollDown( UINT8 ubLinesDown );
void MapScreenMsgScrollUp( UINT8 ubLinesUp );

void ChangeCurrentMapscreenMessageIndex( UINT8 ubNewMessageIndex );
void MoveToEndOfMapScreenMessageList( void );


#ifdef JA2DEMO
// handle leaving during the demo
void HandleLeavingOfMapScreenDuringDemo( void );
#endif

// the dirty state of the mapscreen interface bottom
extern BOOLEAN fMapScreenBottomDirty;

#endif
