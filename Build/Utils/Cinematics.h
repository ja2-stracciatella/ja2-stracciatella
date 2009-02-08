#ifndef CINEMATICS_H
#define CINEMATICS_H

#include "Types.h"


struct SMKFLIC;

void     SmkInitialize(void);
void     SmkShutdown(void);
SMKFLIC* SmkPlayFlic(const char* filename, UINT32 left, UINT32 top, BOOLEAN auto_close);
BOOLEAN  SmkPollFlics(void);
void     SmkCloseFlic(SMKFLIC*);

#endif
