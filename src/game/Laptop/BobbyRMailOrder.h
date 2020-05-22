#ifndef __BOBBYRMAILORDER_H
#define __BOBBYRMAILORDER_H

#include "LaptopSave.h"

#include <vector>



void GameInitBobbyRMailOrder(void);
void EnterBobbyRMailOrder(void);
void ExitBobbyRMailOrder(void);
void HandleBobbyRMailOrder(void);
void RenderBobbyRMailOrder(void);

void BobbyRayMailOrderEndGameShutDown(void);
void EnterInitBobbyRayOrder(void);
void AddJohnsGunShipment(void);

void CreateBobbyRayOrderTitle(void);
void DestroyBobbyROrderTitle(void);
void DrawBobbyROrderTitle(void);

void DisplayPurchasedItems( BOOLEAN fCalledFromOrderPage, UINT16 usGridX, UINT16 usGridY, BobbyRayPurchaseStruct *pBobbyRayPurchase, BOOLEAN fJustDisplayTitles, INT32 iOrderNum );


struct NewBobbyRayOrderStruct
{
	BOOLEAN fActive;
	UINT8   ubDeliveryLoc;				// the city the shipment is going to
	UINT8   ubDeliveryMethod;			// type of delivery: next day, 2 days ...
	BobbyRayPurchaseStruct BobbyRayPurchase[ MAX_PURCHASE_AMOUNT ];
	UINT8   ubNumberPurchases;

	UINT32  uiPackageWeight;
	UINT32  uiOrderedOnDayNum;

	BOOLEAN fDisplayedInShipmentPage;

	UINT8   ubFiller[7]; // XXX HACK000B
};


extern std::vector<NewBobbyRayOrderStruct> gpNewBobbyrShipments;

UINT16 CountNumberOfBobbyPurchasesThatAreInTransit(void);

void NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(HWFILE);
void NewWayOfSavingBobbyRMailOrdersToSaveGameFile(HWFILE);

#endif
