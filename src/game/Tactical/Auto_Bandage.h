#ifndef _AUTO_BANDAGE_H
#define _AUTO_BANDAGE_H

#include "Types.h"
//#define VISIBLE_AUTO_BANDAGE

void AutoBandage( BOOLEAN fStart );
void BeginAutoBandage(void);

BOOLEAN HandleAutoBandage(void);

void ShouldBeginAutoBandage(void);

void SetAutoBandagePending( BOOLEAN fSet );
void HandleAutoBandagePending(void);

// ste the autobandage as complete
void SetAutoBandageComplete( void );

#endif
