#ifndef IMP_ATTRIBUTE_SELECTION_H
#define IMP_ATTRIBUTE_SELECTION_H

#include "Types.h"

void EnterIMPAttributeSelection(void);
void RenderIMPAttributeSelection(void);
void ExitIMPAttributeSelection(void);
void HandleIMPAttributeSelection(void);

void RenderAttributeBoxes(void);

extern BOOLEAN fReviewStats;
extern BOOLEAN fFirstIMPAttribTime;
extern BOOLEAN fReturnStatus;

// starting point of skill boxes on bar
#define SKILL_SLIDE_START_X 186 * g_ui.m_stdScreenScale
#define SKILL_SLIDE_START_Y 100 * g_ui.m_stdScreenScale
#define SKILL_SLIDE_HEIGHT   20 * g_ui.m_stdScreenScale

#endif
