#ifndef __AIMSORT_H_
#define __AIMSORT_H_


extern UINT8 gubCurrentSortMode;
extern UINT8 gubCurrentListMode;


#define		AIM_ASCEND									6
#define		AIM_DESCEND									7


void GameInitAimSort(void);
BOOLEAN EnterAimSort(void);
void ExitAimSort(void);
void HandleAimSort(void);
void RenderAimSort(void);

#endif
