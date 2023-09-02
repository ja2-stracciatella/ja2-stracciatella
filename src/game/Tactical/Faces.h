#ifndef __TALKING_H_
#define __TALKING_H_

#include "JA2Types.h"
#include "Types.h"
#include <string_theory/string>


#define NO_EXPRESSION				0
#define BLINKING				1
#define ANGRY					2
#define SURPRISED				3

// Defines
#define NUM_FACE_SLOTS				50

#define FACE_AUTO_DISPLAY_BUFFER		0
#define FACE_AUTO_RESTORE_BUFFER		0

// FLAGS....
#define FACE_BIGFACE				0x00000001 // A BIGFACE instead of small face
#define FACE_POTENTIAL_KEYWAIT			0x00000002 // If the option is set, will not stop face until key pressed
#define FACE_PCTRIGGER_NPC			0x00000004 // This face has to trigger an NPC after being done
#define FACE_INACTIVE_HANDLED_ELSEWHERE	0x00000008	// This face has been setup and any disable should be done
								// Externally
#define FACE_TRIGGER_PREBATTLE_INT		0x00000010
#define FACE_SHOW_WHITE_HILIGHT		0x00000020 // Show highlight around face
#define FACE_FORCE_SMALL			0x00000040 // force to small face
#define FACE_MODAL				0x00000080 // make game modal
#define FACE_MAKEACTIVE_ONCE_DONE		0x00000100
#define FACE_SHOW_MOVING_HILIGHT		0x00000200
#define FACE_REDRAW_WHOLE_FACE_NEXT_FRAME	0x00000400	// Redraw the complete face next frame

#define FACE_DRAW_TEXT_OVER			2
#define FACE_ERASE_TEXT_OVER			1
#define FACE_NO_TEXT_OVER			0


struct AUDIO_GAP
{
	UINT32 start;
	UINT32 end;
};

// This is a structure that will contain data about the gaps in a particular
// wave file
struct AudioGapList
{
	AUDIO_GAP*       gaps; // Pointer to gap array
	const AUDIO_GAP* end;  // Pointer one past the end of the gap array
};


struct FACETYPE
{
	UINT32  uiFlags; // Basic flags
	BOOLEAN fAllocated; //Allocated or not
	BOOLEAN fTalking; //Set to true if face is talking ( can be sitting for user input to esc )
	BOOLEAN fAnimatingTalking; // Set if the face is animating right now
	BOOLEAN fDisabled; // Not active
	BOOLEAN fValidSpeech;
	BOOLEAN fInvalidAnim;

	UINT32  uiTalkingDuration; // A delay based on text length for how long to talk if no speech
	UINT32  uiTalkingTimer; // A timer to handle delay when no speech file
	UINT32  uiTalkingFromVeryBeginningTimer; // Timer from very beginning of talking...

	BOOLEAN fFinishTalking; // A flag to indicate we want to delay after speech done

	VIDEO_OVERLAY* video_overlay; // Value for video overlay ( not used too much )

	UINT32  uiSoundID; // Sound ID if one being played
	SOLDIERTYPE* soldier;                     // Soldier if one specified
	UINT8   ubCharacterNum; // Profile ID num

	UINT16  usFaceX; // X location to render face
	UINT16  usFaceY; // Y location to render face
	UINT16  usFaceWidth;
	UINT16  usFaceHeight;
	SGPVSurface* uiAutoDisplayBuffer; // Display buffer for face
	SGPVSurface* uiAutoRestoreBuffer; // Restore buffer
	BOOLEAN fAutoRestoreBuffer; // Flag to indicate our own restorebuffer or not
	BOOLEAN fAutoDisplayBuffer; // Flag to indicate our own display buffer or not
	BOOLEAN fDisplayTextOver; // Boolean indicating to display text on face
	BOOLEAN fCanHandleInactiveNow;
	ST::string zDisplayText; // String of text that can be displayed

	UINT16  usEyesX;
	UINT16  usEyesY;
	UINT16  usEyesOffsetX;
	UINT16  usEyesOffsetY;

	UINT16  usEyesWidth;
	UINT16  usEyesHeight;

	UINT16  usMouthX;
	UINT16  usMouthY;
	UINT16  usMouthOffsetX;
	UINT16  usMouthOffsetY;
	UINT16  usMouthWidth;
	UINT16  usMouthHeight;

	UINT16  sEyeFrame;
	INT8    ubEyeWait;
	UINT32  uiEyelast;
	UINT32  uiEyeDelay;
	UINT32  uiBlinkFrequency;
	UINT32  uiExpressionFrequency;

	UINT8   ubExpression;

	INT8    bOldSoldierLife;
	INT8    bOldActionPoints;
	INT8    bOldAssignment;
	INT8    ubOldServiceCount;
	const SOLDIERTYPE* old_service_partner;

	UINT16   sMouthFrame;
	UINT32   uiMouthlast;

	UINT32   uiLastBlink;
	UINT32   uiLastExpression;

	SGPVObject* uiVideoObject;

	BOOLEAN fCompatibleItems;
	BOOLEAN fOldCompatibleItems;
	BOOLEAN bOldStealthMode;
	INT8    bOldOppCnt;

	AudioGapList GapList;

	union // XXX TODO001E ugly
	{
		struct // Used for FACE_PCTRIGGER_NPC
		{
			ProfileID npc;
			UINT8     record;
		} trigger;
		struct // Used for FACE_TRIGGER_PREBATTLE_INT
		{
			GROUP* group;
		} initiating_battle;
	} u;
};

// FACE HANDLING
//
// Faces are done like this: Call
FACETYPE& InitFace(ProfileID id, SOLDIERTYPE* s, UINT32 uiInitFlags);
// The first parameter is the profile ID and the second is the soldier (which
// for most cases will be NULL if the face is not created from a SOLDIERTYPE).
// This function allocates a slot in the table for the face, loads its STI file,
// sets some values for X,Y locations of eyes from the profile. Does not make
// the face visible or anything like that


// Removes the face from the internal table, deletes any memory allocated if any.
void DeleteFace(FACETYPE*);

// IF you want to setup the face for automatic eye blinking, mouth movement, you need to call
void SetAutoFaceActive(SGPVSurface* display, SGPVSurface* restore, FACETYPE&, UINT16 usFaceX, UINT16 usFaceY);
// The first paramter is the display buffer you wish the face to be rendered on. The second is the
// Internal savebuffer which is used to facilitate the rendering of only things which have changed when
// blinking. IF the value of FACE_AUTO_RESTORE_BUFFER is given, the system will allocate it's own memory for
// a saved buffer and will delete it when finished with it. This function also takes an XY location


// To begin rendering of the face sprite, call this function once:
void RenderAutoFace(FACETYPE&);
// This will draw the face into it's saved buffer and then display it on the display buffer. If the display
// buffer given is FRAME_BUFFER, the regions will automatically be dirtied, so no calls to InvalidateRegion()
// should be nessesary.

// If you want to setup the face to talking, ( most times this call is done in JA2 by other functions, not
//directly), you call
void SetFaceTalking(FACETYPE& f, const ST::string& zSoundFile, const ST::string& zTextString);
// This function will setup appropriate face data and begin the speech process. It can fail if the sound
//cannot be played for any reason.

// Set some face talking flags without need to play sound
void ExternSetFaceTalking(FACETYPE&, UINT32 sound_id);

// Once this is done, this function must be called overy gameloop that you want to handle the sprite:
void HandleAutoFaces(void);
// This will handle all faces set to be auto mamaged by SetAutoFaceActive(). What is does is determines
// the best mouth and eye graphic to use. It then renders only the rects nessessary into the display buffer.

// If you need to shutoff the face from talking, use the function
void ShutupaYoFace(FACETYPE*);
void InternalShutupaYoFace(FACETYPE*, BOOLEAN fForce);

// This can be used to times when you need process the user hitting <ESC> to cancel the speech, etc. It will
// shutoff any playing sound sample

// If you still want the face in memory but want to stop if from being
// displayed, or handled call
void SetAutoFaceInActive(FACETYPE&);

// To set all currently allocated faces to either active or incactive, call these
void SetAllAutoFacesInactive(void);

// FUnctions usually not needed for most uses, but give a finer control over rendering if needed
void HandleTalkingAutoFaces(void);




// Same Functions but taking soldier first to get profile
void InitSoldierFace(SOLDIERTYPE&);
void DeleteSoldierFace( SOLDIERTYPE *pSoldier );

// To render an allocated face, but one that is independent of its active
// status and does not require eye blinking or mouth movements, call
void ExternRenderFace(SGPVSurface* buffer, FACETYPE&, INT16 x, INT16 y);

void LoadFacesGraphics();
void DeleteFacesGraphics();

#endif
