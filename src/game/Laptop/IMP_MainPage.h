#ifndef __IMP_MAINPAGE_H
#define __IMP_MAINPAGE_H
#include "Types.h"

void RenderIMPMainPage( void );
void ExitIMPMainPage( void );
void EnterIMPMainPage( void );
void HandleIMPMainPage( void );

extern INT32 iCurrentProfileMode;

SGPVObject* LoadIMPPortait();

#endif
