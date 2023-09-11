#ifndef _IMP_SKILL_TRAIT_H
#define _IMP_SKILL_TRAIT_H

#include "Types.h"

void EnterIMPSkillTrait(void);
void RenderIMPSkillTrait(void);
void ExitIMPSkillTrait(void);
void HandleIMPSkillTrait(void);


INT8 DoesPlayerHaveExtraAttibutePointsToDistributeBasedOnSkillSelection();
void AddSelectedSkillsToSkillsList();

#endif