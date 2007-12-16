#ifndef __FLORIST_H
#define __FLORIST_H



#define		FLORIST_BUTTON_TEXT_FONT					FONT14ARIAL
#define		FLORIST_BUTTON_TEXT_UP_COLOR			2
#define		FLORIST_BUTTON_TEXT_DOWN_COLOR		2
#define		FLORIST_BUTTON_TEXT_SHADOW_COLOR	128


BOOLEAN EnterFlorist(void);
void ExitFlorist(void);
void HandleFlorist(void);
void RenderFlorist(void);

void RemoveFloristDefaults(void);
void DisplayFloristDefaults(void);
BOOLEAN InitFloristDefaults(void);



#endif
