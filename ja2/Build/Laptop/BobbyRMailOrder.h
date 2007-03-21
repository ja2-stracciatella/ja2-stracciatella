#ifndef __BOBBYRMAILORDER_H
#define __BOBBYRMAILORDER_H

#include "LaptopSave.h"




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







void GameInitBobbyRMailOrder();
BOOLEAN EnterBobbyRMailOrder();
void ExitBobbyRMailOrder();
void HandleBobbyRMailOrder();
void RenderBobbyRMailOrder();

void BobbyRayMailOrderEndGameShutDown();
void EnterInitBobbyRayOrder();
void AddJohnsGunShipment();

BOOLEAN CreateBobbyRayOrderTitle();
void DestroyBobbyROrderTitle();
void DrawBobbyROrderTitle();

void DisplayPurchasedItems( BOOLEAN fCalledFromOrderPage, UINT16 usGridX, UINT16 usGridY, BobbyRayPurchaseStruct *pBobbyRayPurchase, BOOLEAN fJustDisplayTitles, INT32 iOrderNum );


typedef struct
{
	BOOLEAN	fActive;
	UINT8		ubDeliveryLoc;				// the city the shipment is going to
	UINT8		ubDeliveryMethod;			// type of delivery: next day, 2 days ...
	BobbyRayPurchaseStruct BobbyRayPurchase[ MAX_PURCHASE_AMOUNT ];
	UINT8		ubNumberPurchases;

	UINT32	uiPackageWeight;
	UINT32	uiOrderedOnDayNum;

	BOOLEAN	fDisplayedInShipmentPage;

	UINT8		ubFiller[7];
} NewBobbyRayOrderStruct;


extern	NewBobbyRayOrderStruct	*gpNewBobbyrShipments;
extern	INT32			giNumberOfNewBobbyRShipment;

UINT16	CountNumberOfBobbyPurchasesThatAreInTransit();

BOOLEAN NewWayOfLoadingBobbyRMailOrdersToSaveGameFile( HWFILE hFile );
BOOLEAN NewWayOfSavingBobbyRMailOrdersToSaveGameFile( HWFILE hFile );

#endif
