#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#include "Types.h"

#define PLAYER_GENERATED_CHARACTER_ID 51
#define NUMBER_OF_PLAYER_PORTRAITS 16

/* The player generated character profiles are laid out as one block of male
 * voices followed by one block of female ones, the slot being picked by
 * PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId. Two I.M.P.s of the
 * same gender would therefore land in the same block and could share a slot,
 * which is why only one character per gender may be created. */
#define NUMBER_OF_PLAYER_VOICES_PER_GENDER 3
#define NUMBER_OF_PLAYER_GENERATED_CHARACTER_SLOTS (2 * NUMBER_OF_PLAYER_VOICES_PER_GENDER)

enum
{
	IMP_GENDER_MALE   = 0x01,
	IMP_GENDER_FEMALE = 0x02,
};

void AddAnAttitudeToAttitudeList( INT8 bAttitude );
void CreateACharacterFromPlayerEnteredStats( void );
void CreatePlayersPersonalitySkillsAndAttitude( void );
void AddAPersonalityToPersonalityList( INT8 bPersonlity );
void AddSkillToSkillList( INT8 bSkill );
void ResetSkillsAttributesAndPersonality( void );
void HandleMercStatsForChangesInFace( void );

/** Gender flag of the profile slot a voice id resolves to. */
UINT8 GetIMPGenderFlag( INT32 iVoiceId );

/** Number of I.M.P. characters created in this campaign so far. */
UINT8 GetNumberOfIMPCharactersCreated( void );

/** Whether the game policy still allows another I.M.P. character. */
bool CanCreateAnotherIMPCharacter( void );

/** Whether another I.M.P. character may be created with the given gender. */
bool CanCreateIMPCharacterOfGender( bool fMale );

/** Record a finished I.M.P. character so its gender cannot be picked again. */
void MarkIMPCharacterCreated( INT32 iVoiceId );

#endif
