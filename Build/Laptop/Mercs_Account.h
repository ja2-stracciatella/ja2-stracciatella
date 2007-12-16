#ifndef __MERCS_ACCOUNT_H
#define __MERCS_ACCOUNT_H

BOOLEAN EnterMercsAccount(void);
void ExitMercsAccount(void);
void HandleMercsAccount(void);
void RenderMercsAccount(void);

UINT32 CalculateHowMuchPlayerOwesSpeck(void);

#endif
