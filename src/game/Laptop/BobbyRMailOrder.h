#ifndef __BOBBYRMAILORDER_H
#define __BOBBYRMAILORDER_H

#include "LaptopSave.h"

#include <vector>


//enums for the various destinations that are available in the bobbyR dest drop down box
enum
{
	BR_AUSTIN,
	BR_BAGHDAD,
	BR_DRASSEN,
	BR_HONG_KONG,
	BR_BEIRUT,
	BR_LONDON,
	BR_LOS_ANGELES,
	BR_MEDUNA,
	BR_METAVIRA,
	BR_MIAMI,
	BR_MOSCOW,
	BR_NEW_YORK,
	BR_OTTAWA,
	BR_PARIS,
	BR_TRIPOLI,
	BR_TOKYO,
	BR_VANCOUVER,
};


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
