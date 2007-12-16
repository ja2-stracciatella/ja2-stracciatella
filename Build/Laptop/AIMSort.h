#ifndef __AIMSORT_H_
#define __AIMSORT_H_

UINT8			gubCurrentSortMode;
UINT8			gubCurrentListMode;



#define		AIM_ASCEND									6
#define		AIM_DESCEND									7


void GameInitAimSort(void);
BOOLEAN EnterAimSort(void);
void ExitAimSort(void);
void HandleAimSort(void);
void RenderAimSort(void);

#endif
