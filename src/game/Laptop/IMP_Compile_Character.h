#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#include "JA2Types.h"
#include "Types.h"

#define PLAYER_GENERATED_CHARACTER_ID 51
#define NUMBER_OF_PLAYER_PORTRAITS 16

/* The voices and portraits the I.M.P. screens offer. Voices are the shipped
 * sets of speech, dialogue text and battle sounds, named after the profiles
 * they were recorded for, one block of male ones followed by a female block.
 * Which profile a character ends up in is unrelated: it takes the first free
 * slot and its voice is written to its profile, so any number of characters
 * may share a voice or a portrait. */
#define NUMBER_OF_PLAYER_VOICES_PER_GENDER 3
#define NUMBER_OF_PLAYER_PORTRAITS_PER_GENDER (NUMBER_OF_PLAYER_PORTRAITS / 2)

void AddAnAttitudeToAttitudeList( INT8 bAttitude );
void CreateACharacterFromPlayerEnteredStats( void );
void CreatePlayersPersonalitySkillsAndAttitude( void );
void AddAPersonalityToPersonalityList( INT8 bPersonlity );
void AddSkillToSkillList( INT8 bSkill );
void ResetSkillsAttributesAndPersonality( void );
void HandleMercStatsForChangesInFace( void );

/** Number of profiles the data marks as I.M.P. slots. */
UINT8 GetNumberOfIMPSlots( void );

/** Number of I.M.P. characters created in this campaign so far. */
UINT8 GetNumberOfIMPCharactersCreated( void );

/** The profile the character being built lives in, which is always the first
 * free I.M.P. slot, or NO_PROFILE when every slot is taken. */
ProfileID GetIMPSlotInProgress( void );

/** Whether there is a free slot and the game policy allows another character. */
bool CanCreateAnotherIMPCharacter( void );

/** Hold the profile the finished character was built in for the rest of the
 * campaign. */
void MarkIMPCharacterCreated( ProfileID profile );

#endif
