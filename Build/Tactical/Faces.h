#ifndef __TALKING_H_
#define __TALKING_H_
#define   NO_EXPRESSION			0
#define		BLINKING					1
#define		ANGRY							2
#define   SURPRISED					3

#include "Soldier_Control.h"

// Defines
#define		NUM_FACE_SLOTS					50

struct audio_gap
{
	UINT32 uiStart;
	UINT32 uiEnd;
	struct audio_gap *pNext;
};

#define  FACE_AUTO_DISPLAY_BUFFER		0xFFFFF000
#define  FACE_AUTO_RESTORE_BUFFER		0xFFFFFF00
#define  FACE_NO_RESTORE_BUFFER			0xFFFFFFF0

// FLAGS....
#define	 FACE_BIGFACE												0x00000001						// A BIGFACE instead of small face
#define	 FACE_POTENTIAL_KEYWAIT							0x00000002						// If the option is set, will not stop face until key pressed
#define	 FACE_PCTRIGGER_NPC									0x00000004						// This face has to trigger an NPC after being done
#define  FACE_INACTIVE_HANDLED_ELSEWHERE		0x00000008						// This face has been setup and any disable should be done
																																	// Externally
#define	 FACE_TRIGGER_PREBATTLE_INT					0x00000010
#define	 FACE_SHOW_WHITE_HILIGHT						0x00000020						// Show highlight around face
#define	 FACE_FORCE_SMALL										0x00000040						// force to small face
#define	 FACE_MODAL													0x00000080						// make game modal
#define	 FACE_MAKEACTIVE_ONCE_DONE					0x00000100
#define  FACE_SHOW_MOVING_HILIGHT						0x00000200
#define	 FACE_REDRAW_WHOLE_FACE_NEXT_FRAME	0x00000400						// Redraw the complete face next frame

#define	 FACE_DRAW_TEXT_OVER				2
#define	 FACE_ERASE_TEXT_OVER				1
#define	 FACE_NO_TEXT_OVER					0


// duration for talking
#define		FINAL_TALKING_DURATION	2000

typedef struct audio_gap AUDIO_GAP;

typedef struct
{
  // This is a structure that will contain data about the gaps in a particular
	// wave file

  UINT32					size; /* the number of entries in the list of AUDIO_GAPs itself*/
  UINT32					current_time;
  // Pointer to head and current entry of gap list
	AUDIO_GAP *pHead;
	AUDIO_GAP *pCurrent;

	BOOLEAN					audio_gap_active;

} AudioGapList;


typedef struct
{
	UINT32		uiFlags;												// Basic flags
	INT32			iID;
	BOOLEAN		fAllocated;											//Allocated or not
	BOOLEAN		fTalking;												//Set to true if face is talking ( can be sitting for user input to esc )
	BOOLEAN		fAnimatingTalking;							// Set if the face is animating right now
	BOOLEAN		fDisabled;											// Not active
	BOOLEAN		fValidSpeech;
	BOOLEAN		fInvalidAnim;

	UINT32		uiTalkingDuration;							// A delay based on text length for how long to talk if no speech
	UINT32		uiTalkingTimer;									// A timer to handle delay when no speech file
	UINT32		uiTalkingFromVeryBeginningTimer;// Timer from very beginning of talking...

	BOOLEAN		fFinishTalking;									// A flag to indicate we want to delay after speech done

	INT32			iVideoOverlay;									// Value for video overlay ( not used too much )

	UINT32		uiSoundID;											// Sound ID if one being played
	UINT8			ubSoldierID;										// SoldierID if one specified
	UINT8			ubCharacterNum;									// Profile ID num

  UINT16		usFaceX;												// X location to render face
  UINT16		usFaceY;												// Y location to render face
	UINT16		usFaceWidth;
	UINT16		usFaceHeight;
	UINT32		uiAutoDisplayBuffer;						// Display buffer for face
	UINT32		uiAutoRestoreBuffer;						// Restore buffer
	BOOLEAN		fAutoRestoreBuffer;							// Flag to indicate our own restorebuffer or not
	BOOLEAN		fAutoDisplayBuffer;							// Flag to indicate our own display buffer or not
	BOOLEAN		fDisplayTextOver;								// Boolean indicating to display text on face
	BOOLEAN		fCanHandleInactiveNow;
	wchar_t			zDisplayText[ 30 ];							// String of text that can be displayed

	UINT16		usEyesX;
	UINT16		usEyesY;
	UINT16		usEyesOffsetX;
	UINT16		usEyesOffsetY;

	UINT16		usEyesWidth;
	UINT16		usEyesHeight;

	UINT16		usMouthX;
	UINT16		usMouthY;
	UINT16		usMouthOffsetX;
	UINT16		usMouthOffsetY;
	UINT16		usMouthWidth;
	UINT16		usMouthHeight;

  UINT16		sEyeFrame;
	INT8			ubEyeWait;
  UINT32		uiEyelast;
  UINT32		uiEyeDelay;
	UINT32		uiBlinkFrequency;
	UINT32		uiExpressionFrequency;

	UINT8			ubExpression;

	INT8			bOldSoldierLife;
	INT8			bOldActionPoints;
	BOOLEAN		fOldShowHighlight;
	INT8			bOldAssignment;
	INT8			ubOldServiceCount;
	UINT8			ubOldServicePartner;
	BOOLEAN		fOldShowMoveHilight;

	UINT16		sMouthFrame;
  UINT32		uiMouthlast;
  UINT32		uiMouthDelay;

	UINT32		uiLastBlink;
	UINT32		uiLastExpression;

  UINT32		uiVideoObject;

	UINT32		uiUserData1;
	UINT32		uiUserData2;

	BOOLEAN		fCompatibleItems;
	BOOLEAN		fOldCompatibleItems;
	BOOLEAN		bOldStealthMode;
	INT8			bOldOppCnt;

	AudioGapList		GapList;

} FACETYPE;

// GLOBAL FOR FACES LISTING
FACETYPE	gFacesData[ NUM_FACE_SLOTS ];

// FACE HANDLING
//
// Faces are done like this: Call
INT32	InitFace( UINT8 usMercProfileID, UINT8 ubSoldierID, UINT32 uiInitFlags );
// The first parameter is the profile ID and the second is the soldier ID ( which for most cases
// will be NOBODY if the face is not created from a SOLDIERTYPE )
// This function allocates a slot in the table for the face, loads it's STI file, sets some
// values for X,Y locations of eyes from the profile. Does not mkae the face visible or anything like that


// Removes the face from the internal table, deletes any memory allocated if any.
void DeleteFace( INT32 iFaceIndex );

// IF you want to setup the face for automatic eye blinking, mouth movement, you need to call
void SetAutoFaceActive( UINT32 uiDisplayBuffer, UINT32 uiRestoreBuffer, INT32 iFaceIndex , UINT16 usFaceX, UINT16 usFaceY );
// The first paramter is the display buffer you wish the face to be rendered on. The second is the
// Internal savebuffer which is used to facilitate the rendering of only things which have changed when
// blinking. IF the value of FACE_AUTO_RESTORE_BUFFER is given, the system will allocate it's own memory for
// a saved buffer and will delete it when finished with it. This function also takes an XY location


// To begin rendering of the face sprite, call this function once:
BOOLEAN RenderAutoFace( INT32 iFaceIndex );
// This will draw the face into it's saved buffer and then display it on the display buffer. If the display
// buffer given is FRAME_BUFFER, the regions will automatically be dirtied, so no calls to InvalidateRegion()
// should be nessesary.

// If you want to setup the face to talking, ( most times this call is done in JA2 by other functions, not
//directly), you call
BOOLEAN SetFaceTalking(INT32 iFaceIndex, CHAR8* zSoundFile, STR16 zTextString);
// This function will setup appropriate face data and begin the speech process. It can fail if the sound
//cannot be played for any reason.

// Set some face talking flags without need to play sound
BOOLEAN ExternSetFaceTalking( INT32 iFaceIndex, UINT32 uiSoundID );

// Once this is done, this function must be called overy gameloop that you want to handle the sprite:
void HandleAutoFaces( );
// This will handle all faces set to be auto mamaged by SetAutoFaceActive(). What is does is determines
// the best mouth and eye graphic to use. It then renders only the rects nessessary into the display buffer.

// If you need to shutoff the face from talking, use the function
void ShutupaYoFace( INT32 iFaceIndex );
void InternalShutupaYoFace( INT32 iFaceIndex, BOOLEAN fForce );

// This can be used to times when you need process the user hitting <ESC> to cancel the speech, etc. It will
// shutoff any playing sound sample

// If you still want the face in moemory but want to stop if from being displayed, or handled call
void SetAutoFaceInActive( INT32 iFaceIndex );

// To set all currently allocated faces to either active or incactive, call these
void SetAllAutoFacesInactive( );

// FUnctions usually not needed for most uses, but give a finer control over rendering if needed
void HandleTalkingAutoFaces( );




// Same Functions but taking soldier ID first to get profile ID
INT32		InitSoldierFace( SOLDIERTYPE *pSoldier );
void		DeleteSoldierFace( SOLDIERTYPE *pSoldier );
void		SetAutoFaceActiveFromSoldier( UINT32 uiDisplayBuffer, UINT32 uiRestoreBuffer, UINT8 ubSoldierID , UINT16 usFaceX, UINT16 usFaceY );
void		SetAutoFaceInActiveFromSoldier( UINT8 ubSoldierID );
BOOLEAN RenderAutoFaceFromSoldier( UINT8 ubSoldierID );
BOOLEAN ExternRenderFaceFromSoldier( UINT32 uiBuffer, UINT8 ubSoldierID, INT16 sX, INT16 sY );









#endif
