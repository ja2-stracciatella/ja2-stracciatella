#ifndef __IMP_COMPILE_H
#define __IMP_COMPILE_H

#define PLAYER_GENERATED_CHARACTER_ID 51
#define ATTITUDE_LIST_SIZE 20
#define NUMBER_OF_PLAYER_PORTRAITS 16

void AddAnAttitudeToAttitudeList( INT8 bAttitude );
void CreatePlayerAttitude( void );
BOOLEAN DoesCharacterHaveAnAttitude( void );
void CreateACharacterFromPlayerEnteredStats( void );
void CreatePlayerSkills( void );
void CreatePlayersPersonalitySkillsAndAttitude( void );
void AddAPersonalityToPersonalityList( INT8 bPersonlity );
void CreatePlayerPersonality( void );
BOOLEAN DoesCharacterHaveAPersoanlity( void );
void AddSkillToSkillList( INT8 bSkill );
void ResetSkillsAttributesAndPersonality( void );
void ResetIncrementCharacterAttributes( void );
void HandleMercStatsForChangesInFace( void );


extern const char *pPlayerSelectedFaceFileNames[ NUMBER_OF_PLAYER_PORTRAITS ];
extern const char *pPlayerSelectedBigFaceFileNames[ NUMBER_OF_PLAYER_PORTRAITS ];
#endif
