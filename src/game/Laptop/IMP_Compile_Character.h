#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#include "Types.h"

#define PLAYER_GENERATED_CHARACTER_ID 51
#define NUMBER_OF_PLAYER_PORTRAITS 16

/* The player generated character profiles are laid out as one block of male
 * voices followed by one block of female ones, the slot being picked by
 * PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId. There is exactly one
 * slot per voice and a finished character keeps its slot for the rest of the
 * campaign, so the hard ceiling is three I.M.P.s of each gender. */
#define NUMBER_OF_PLAYER_VOICES_PER_GENDER 3
#define NUMBER_OF_PLAYER_GENERATED_CHARACTER_SLOTS (2 * NUMBER_OF_PLAYER_VOICES_PER_GENDER)
#define NUMBER_OF_PLAYER_PORTRAITS_PER_GENDER (NUMBER_OF_PLAYER_PORTRAITS / 2)

void AddAnAttitudeToAttitudeList( INT8 bAttitude );
void CreateACharacterFromPlayerEnteredStats( void );
void CreatePlayersPersonalitySkillsAndAttitude( void );
void AddAPersonalityToPersonalityList( INT8 bPersonlity );
void AddSkillToSkillList( INT8 bSkill );
void ResetSkillsAttributesAndPersonality( void );
void HandleMercStatsForChangesInFace( void );

/** Bit that a voice id stands for in LaptopSaveInfo.ubIMPCreatedSlots. */
UINT8 GetIMPVoiceSlotFlag( INT32 iVoiceId );

/** Whether the profile slot behind a voice id is spoken for. Out of range counts
 * as taken, so a caller cycling through voices cannot settle on one. */
bool IsIMPVoiceTaken( INT32 iVoiceId );

/** Whether one of the I.M.P.s created so far already wears this portrait. */
bool IsIMPPortraitTaken( INT32 iPortraitNumber );

/** Number of I.M.P. characters created in this campaign so far. */
UINT8 GetNumberOfIMPCharactersCreated( void );

/** Whether the game policy still allows another I.M.P. character. */
bool CanCreateAnotherIMPCharacter( void );

/** Whether another I.M.P. character may be created with the given gender. */
bool CanCreateIMPCharacterOfGender( bool fMale );

/** First voice of a gender whose slot is still free, counted within the gender
 * as iCurrentVoices is, or -1 when that gender has no slot left. */
INT32 GetFirstFreeIMPVoice( bool fMale );

/** First portrait of a gender that no I.M.P. wears yet, counted within the
 * gender as iCurrentPortrait is, or -1 when there is none. */
INT32 GetFirstFreeIMPPortrait( bool fMale );

/** Record a finished I.M.P. character so its slot cannot be handed out again. */
void MarkIMPCharacterCreated( INT32 iVoiceId );

#endif
