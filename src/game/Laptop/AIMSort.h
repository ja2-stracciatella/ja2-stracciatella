#ifndef __AIMSORT_H_
#define __AIMSORT_H_

#include "Types.h"

extern UINT8 gubCurrentSortMode;
extern UINT8 gubCurrentListMode;


#define AIM_ASCEND	6
#define AIM_DESCEND	7


void GameInitAimSort(void);
void EnterAimSort(void);
void ExitAimSort(void);
void RenderAimSort(void);

#endif
