#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#include "IMPPolicy.h"
#include "JA2Types.h"
#include "Types.h"

#include <vector>

/* The first of the six profiles the shipped voices were recorded for. Nothing
 * is derived from it any more: it is only what an I.M.P. saved before the voice
 * id was stored has to be read against. */
#define PLAYER_GENERATED_CHARACTER_ID 51

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


/* The voices and portraits the data offers, in the order imp.json lists them.
 * The I.M.P. screens page through the entries of the gender being profiled;
 * what the character keeps is an index into the whole list. */

const std::vector<IMPVoice>& GetIMPVoices( void );
const std::vector<IMPPortrait>& GetIMPPortraits( void );

/** How many entries one gender is offered. Never zero for a gender the site
 * lets the player pick. */
INT32 GetNumberOfIMPVoices( bool fMale );
INT32 GetNumberOfIMPPortraits( bool fMale );

/** Where the n-th entry offered for one gender sits in the whole list. */
INT32 GetIMPVoiceIndex( bool fMale, INT32 iNth );
INT32 GetIMPPortraitIndex( bool fMale, INT32 iNth );

/** Where the portrait wearing this face image sits in the whole list, or -1
 * when the data no longer offers it. */
INT32 FindIMPPortraitByFace( UINT8 ubFaceIndex );

/** The portrait the character being built wears. Every screen takes its index
 * from GetIMPPortraitIndex, so it is only ever out of range if the data changed
 * under a half built character. */
const IMPPortrait& GetCurrentIMPPortrait( void );

#endif
