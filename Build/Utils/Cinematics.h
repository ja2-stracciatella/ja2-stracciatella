#ifndef _CINEMATICS_H_
#define _CINEMATICS_H_

#include "Types.h"

typedef struct SMKFLIC SMKFLIC;

void				SmkInitialize(UINT32 uiWidth, UINT32 uiHeight);
void				SmkShutdown(void);
SMKFLIC			*SmkPlayFlic(const char *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fAutoClose);
BOOLEAN			SmkPollFlics(void);
void				SmkCloseFlic(SMKFLIC *pSmack);

#endif
