#ifndef __CHARPROFILE_H
#define __CHARPROFILE_H

#include "Types.h"

#include <string_theory/string>


void GameInitCharProfile(void);
void EnterCharProfile(void);
void ExitCharProfile(void);
void HandleCharProfile(void);
void RenderCharProfile(void);
void ResetCharacterStats( void );
void InitIMPSubPageList( void );

extern BOOLEAN fButtonPendingFlag;
extern BOOLEAN fReDrawCharProfile;
extern INT32 iCurrentImpPage;

// attributes
extern INT32 iStrength;
extern INT32 iDexterity;
extern INT32 iAgility;
extern INT32 iWisdom;
extern INT32 iLeadership;
extern INT32 iHealth;

// skills
extern INT32 iMarksmanship;
extern INT32 iMedical;
extern INT32 iExplosives;
extern INT32 iMechanical;

// sex?
extern BOOLEAN fCharacterIsMale;

// name?
extern ST::string pFullName;
extern ST::string pNickName;

// skills
extern INT32 iSkillA;
extern INT32 iSkillB;

// persoanlity
extern INT32 iPersonality;

// attitude
extern INT32 iAttitude;

enum{
	IMP_HOME_PAGE,
	IMP_BEGIN,
	IMP_FINISH,
	IMP_MAIN_PAGE,
	IMP_PERSONALITY,
	IMP_PERSONALITY_QUIZ,
	IMP_SKILLTRAITS,
	IMP_PERSONALITY_FINISH,
	IMP_ATTRIBUTE_ENTRANCE,
	IMP_ATTRIBUTE_PAGE,
	IMP_ATTRIBUTE_FINISH,
	IMP_PORTRAIT,
	IMP_VOICE,
	IMP_ABOUT_US,
	IMP_CONFIRM,

	IMP_NUM_PAGES,
};

#define COST_OF_PROFILE 3000

extern BOOLEAN fLoadingCharacterForPreviousImpProfile;

#endif
