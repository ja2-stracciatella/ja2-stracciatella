#ifndef __CHARPROFILE_H
#define __CHARPROFILE_H

#include "types.h"
void GameInitCharProfile();
void EnterCharProfile();
void ExitCharProfile();
void HandleCharProfile();
void RenderCharProfile();
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
extern CHAR16 pFullName[];
extern CHAR16 pNickName[];

// skills
extern INT32 iSkillA;
extern INT32 iSkillB;

// persoanlity
extern INT32 iPersonality;

// attitude
extern INT32 iAttitude;

//addtivies
extern INT32 iAddStrength;
extern INT32 iAddDexterity;
extern INT32 iAddAgility;
extern INT32 iAddWisdom;
extern INT32 iAddHealth ;
extern INT32 iAddLeadership ;

extern INT32 iAddMarksmanship ;
extern INT32 iAddMedical ;
extern INT32 iAddExplosives ;
extern INT32 iAddMechanical ;

// pop up strings
extern STR16 pImpPopUpStrings[];

//extern BOOLEAN fIMPCompletedFlag;

enum{
	IMP_HOME_PAGE,
	IMP_BEGIN,
  IMP_FINISH,
	IMP_MAIN_PAGE,
	IMP_PERSONALITY,
	IMP_PERSONALITY_QUIZ,
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

#endif
