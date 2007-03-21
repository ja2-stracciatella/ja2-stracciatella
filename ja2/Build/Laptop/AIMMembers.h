#ifndef __AIMMEMBERS_H_
#define __AIMMEMBERS_H_

void GameInitAIMMembers();
BOOLEAN EnterAIMMembers();
void ExitAIMMembers();
void HandleAIMMembers();
BOOLEAN RenderAIMMembers();


BOOLEAN DrawNumeralsToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, UINT32 ulFont, UINT8 ubColor);

void DisplayTextForMercFaceVideoPopUp(STR16 pString, size_t Length);
void EnterInitAimMembers();
BOOLEAN RenderAIMMembersTopLevel();
void ResetMercAnnoyanceAtPlayer( UINT8 ubMercID );
BOOLEAN DisableNewMailMessage();
void DisplayPopUpBoxExplainingMercArrivalLocationAndTime( );


// which mode are we in during video conferencing?..0 means no video conference
extern UINT8 gubVideoConferencingMode;

//TEMP!!!
#ifdef JA2TESTVERSION
void TempHiringOfMercs( UINT8 ubNumberOfMercs, BOOLEAN fReset );
#endif


#if defined ( JA2TESTVERSION ) || defined ( JA2DEMO )
	void DemoHiringOfMercs( );
#endif



#endif
