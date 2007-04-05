#ifndef __FLORIST_H
#define __FLORIST_H



#define		FLORIST_BUTTON_TEXT_FONT					FONT14ARIAL
#define		FLORIST_BUTTON_TEXT_UP_COLOR			2
#define		FLORIST_BUTTON_TEXT_DOWN_COLOR		2
#define		FLORIST_BUTTON_TEXT_SHADOW_COLOR	128


BOOLEAN EnterFlorist();
void ExitFlorist();
void HandleFlorist();
void RenderFlorist();

void RemoveFloristDefaults();
void DisplayFloristDefaults();
BOOLEAN InitFloristDefaults();



#endif
