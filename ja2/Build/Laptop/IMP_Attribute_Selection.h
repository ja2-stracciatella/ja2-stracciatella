#ifndef _IMP_ATTRIBUTE_SELECTION_H
#define _IMP_ATTRIBUTE_SELECTION_H

void EnterIMPAttributeSelection( void );
void RenderIMPAttributeSelection( void );
void ExitIMPAttributeSelection( void );
void HandleIMPAttributeSelection( void );

void RenderAttributeBoxes( void );

extern BOOLEAN fReviewStats;
extern BOOLEAN fSkillAtZeroWarning;

extern BOOLEAN fFirstIMPAttribTime;

extern BOOLEAN fReturnStatus;
// starting point of skill boxes on bar
#define SKILL_SLIDE_START_X 186

#define SKILL_SLIDE_START_Y 100
#define SKILL_SLIDE_HEIGHT 20


#endif
