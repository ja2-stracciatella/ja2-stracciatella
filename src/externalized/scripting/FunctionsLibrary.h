#pragma once

#include "Arms_Dealer.h"
#include "Facts.h"
#include "Item_Types.h"
#include "Types.h"
#include "Observable.h"
#include <variant>
#include <vector>

typedef std::map<std::string, std::variant<std::string, int32_t, float, bool>> ExtraGameStatesTable;

/*! \file FunctionsLibrary.h */

/*! \struct OBJECTTYPE
    \brief Representation of an inventory item in the game world */
struct OBJECTTYPE;
/*! \struct SECTORINFO
    \brief Status information of a sector */
struct SECTORINFO;
/*! \struct UNDERGROUND_SECTORINFO
    \brief Status information of an underground sector, similar to SECTORINFO but holds slightly different data  */
struct UNDERGROUND_SECTORINFO;
/*! \struct SOLDIERTYPE
    \brief Information and attributes of a soldier in game */
struct SOLDIERTYPE;
/*! \struct MERCPROFILESTRUCT
    \brief Profile of a character in game; controls soldier's name, appearance and others */
struct MERCPROFILESTRUCT;
/*! \struct STRUCTURE
    \brief An structure element on the tactical map */
struct STRUCTURE;
/*! \struct GROUP
    \brief Representation of group of mobile enemies */
struct GROUP;
/*! \struct StrategicMapElement
    \brief Strategic status of ownership and control of a map sector */
struct StrategicMapElement;
/*! \struct STRATEGICEVENT
    \brief An event to scheduled for process at a later time in game */
struct STRATEGICEVENT;

/*! \defgroup funclib-dealers Shops and arms dealers
    \brief Manage behavior, inventory and prices of dealers */

/*! \struct DEALER_ITEM_HEADER
    \brief An item in dealer's inventory */
struct DEALER_ITEM_HEADER;

/**
 * @defgroup observables Observables
 * @brief Register listeners on these observables to receive callbacks when somemthing happens in game.
 * @see RegisterListener
 */

 /**
  * Callback when a soldier is created and before the soldier is assigned to teams and placed on map.
  * @param pSoldier pointer to the soldier being created
  * @ingroup observables
  */
extern Observable<SOLDIERTYPE*> OnSoldierCreated;

/**
 * When a soldier (of any faction) has just died.
 */
extern Observable<const SOLDIERTYPE*> OnSoldierDeath;

/**
 * Callback after loading map and before setting it up (e.g. placing soldiers)
 * @ingroup observables
 */
extern Observable<> BeforePrepareSector;

/**
 * Callback right before the structure damange is inflicted.
 * @param sSectorX
 * @param sSectorY
 * @param bSectorZ
 * @param sGridNo location on the tactical map affected by the exploson
 * @param pStructure pointer to the structure to be damaged
 * @param uiDist no of grids from the source of explosion
 * @param fSkipDamage damage processing will be skipped if it is set to TRUE
 * @ingroup observables
 */
extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*> BeforeStructureDamaged;


extern Observable<INT16, OBJECTTYPE*, BOOLEAN_S*> OnItemAction;
/**
  * Callback just after a structure has just been damaged by explosives
  * @param sSectorX
  * @param sSectorY
  * @param bSectorZ
  * @param sGridNo location on map affected by the exploson
  * @param pStructure pointer to the structure being damaged by explosion
  * @param ubDamage damage amount just dealt
  * @param fIsDestroyed whether or not the structure has been destroyed
  * @ingroup observables
  */
extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN> OnStructureDamaged;

/**
 * Callback when we are about to drop from strategic layer to tactical. Implement here if you want to the scripted
 * actions to run as soon as we enter tactical.
 */
extern Observable<> OnEnterTacticalScreen;

/**
 * Hook to the screen handle function of strategic screen. This is called whenever the screen changes.
 */
extern Observable<> OnHandleStrategicScreen;

/**
 * Callback when a new campaign is started. This is the place for one-time initialization.
 */
extern Observable<> OnInitNewCampaign;

/**
 * Callback when a merc is hired and add to the team. You can add items, modify soldier attributes, etc.
 * @param pointer to the soldier just hired
 */
extern Observable<SOLDIERTYPE*> OnMercHired;

/**
 * Callback when an event is due and to be handled. Implement handlers here if custom strategic events are added.
 * @param the event to be handled
 * @param set to true if the event should not be further processed by the base game
 */
extern Observable<STRATEGICEVENT*, BOOLEAN_S*> OnStrategicEvent;

/**
 * Allows to override the player progress calculation.
 * @param the progress percentage calculated by the base game. This can be adjusted or overridden.
 */
extern Observable<UINT8_S*> OnCalcPlayerProgress;

/**
 * Give the reason on why time compression is disallowed. You can show a pop-up, play a quote, etc
 * @param set to true to skip base game logic
 * @see AllowedToTimeCompress
 */
extern Observable<BOOLEAN_S*> OnTimeCompressDisallowed;

/**
 * Callback every morning to check quests' statuses..
 * @param the current day
 * @param set to true to skip base game checks
 */
extern Observable<UINT32, BOOLEAN_S*> OnCheckQuests;

/**
 * Callback when a quest is completed.
 * @param the Quest ID
 * @param sector X
 * @param sector Y
 * @param whether or not to write an update to the laptop history page
 */
extern Observable<UINT8, INT16, INT16, BOOLEAN> OnQuestEnded;

/**
 * Callback when a NPC does an action according to the .npc script, right before the base game logic is executed.
 * @param profile ID of the NPC
 * @param action code of what NPC is doing; one of the NPC_ACTION_ enums
 * @param a soldier quote associated; the actual meaning depends on context
 * @param set to true to indicate no further processing is needed
 */
extern Observable<UINT8, UINT16, UINT8, BOOLEAN_S*> OnNPCDoAction;

/**
 * Callback when we are about to enter a sector. This happens after PrepareSector. We have all soldiers and objects ready.
 * @param sector X
 * @param sector Y
 * @param sector Z
 */
extern Observable<INT16, INT16, INT8> OnEnterSector;

/**
 * Callback when an email is about to be added to the player's inbox
 * @param Email offset - the offset to the text record in email.edt
 * @param Email length - the number of lines in email.edt to use
 * @param Time in world-minutes, of when the email is sent
 * @param ID of the sender; it should be a value from the email sender enum
 * @param whether or not the email is already mark read when added
 * @param first data field (e.g. merc profile ID) for certain templated emails
 * @param second data field (e.g. money amount) for certain templated emails
 * @param set to true to cancel adding the email
 */
extern Observable<INT32, INT32, INT32, UINT8, BOOLEAN, INT32, UINT32, BOOLEAN_S*> OnAddEmail;

/**
 * When a soldier picks or receives an item
 * @param the soldier getting the item
 * @param the item being picked up
 * @param grid no of the soldier
 * @param level (on ground or elevated, in tactical) of the soldier
 */
extern Observable<SOLDIERTYPE*, OBJECTTYPE*, INT16, INT8> OnSoldierGotItem;

/**
 * @defgroup funclib-general General 
 * @brief Functions to compose mod modules
 */

/**
 * When the game about to be saved. This is the place to persist mod game states.
 * @ingroup observables
 */
extern Observable<> BeforeGameSaved;

/**
 * Right after a game is loaded. This is the place to restore game states from a saved game.
 * @ingroup observables
 */
extern Observable<> OnGameLoaded;

/**
 * When a dealer/shopkeeper's inventory has been updated
 * @ingroup observables
 */
extern Observable<> OnDealerInventoryUpdated;

/**
 * Calls for each item transferred from a dealer in a transaction
 * @param bSelectedArmsDealerID ID of the dealer
 * @param sItemIndex ID of the item being transacted
 * @param fDealerSelling TRUE if the dealer is selling to the player. FALSE if the dealer is buying
 * @ingroup observables
 */
extern Observable<INT8, UINT16, BOOLEAN> OnItemTransacted;

/**
 * Called when an item is being priced by a shopkeeper.
 * The basic calculation already done at this point, and this gives an opportunity to modify the
 * final unit price, to give special discounts, etc.
 * @param gbSelectedArmsDealerID ID of the dealer
 * @param usItemID ID of the item being priced
 * @param fDealerSelling TRUE if the dealer is selling to the player. FALSE if the dealer is buying
 * @param uiUnitPriceAdjusted The observer can override this value to change the item price
 * @ingroup observables
 */
extern Observable<INT8, UINT16, BOOLEAN, UINT32_S*> OnItemPriced;

 /*
 * When a merc has just be hired and the soldier was just created in the world.
 * @param soldier the soldier being hired
 * @ingroup observables
 */
extern Observable<SOLDIERTYPE*> OnMercHired;

/**
 * When an RPC has been recruited into our team.
 * @param soldier the RPC just recruited
 * @ingroup observables
 */
extern Observable<SOLDIERTYPE*> OnRPCRecruited;

/**
 * @defgroup funclib-general General
 * @brief Functions to compose mod modules
 */

/**
 * @defgroup funclib-mercs Personnel
 * @brief Functions to access soldiers and characters in the game
 */

/**
 * Called when we are about to go to a web page on the Laptop.
 * @param iPageId ID of the page we want to go to
 */
extern Observable<INT32> OnGoToWebPage;

/** @defgroup funclib-sectors Map sectors
 *  @brief Access and alter sectors' strategic-level data
 */

/**
 * Returns the ID of the current sector, the a format of A11 or B12-1(if underground)
 * @ingroup funclib-sectors
 */
std::string GetCurrentSector();

std::tuple<int, int, int> GetCurrentSectorLoc();

/**
 * Returns a pointer to the SECTOR_INFO of the specified sector. Does not work for underground sectors.
 * @param sectorID the short-string sector ID, e.g. "A10"
 * @throws std::runtime_error if sectorID is empty or invalid
 * @see GetUndergroundSectorInfo
 * @ingroup funclib-sectors
 */
SECTORINFO* GetSectorInfo(std::string sectorID);

/**
 * Returns a pointer to the UNDERGROUND_SECTORINFO of the specific underground sector.
 * @param sectorID in the format of "XY-Z", e.g. A10-1
 * @see GetSectorInfo
 * @ingroup funclib-sectors
 */
UNDERGROUND_SECTORINFO* GetUndergroundSectorInfo(std::string sectorID);

/**
 * Retrums the StrategicMapElement of the sector
 * @param sectorID
 * @return
 */
StrategicMapElement* GetStrategicMapElement(std::string sectorID);

BOOLEAN SetThisSectorAsPlayerControlled(const SGPSector& s, BOOLEAN fContested);

/** @defgroup funclib-items Items and objects
 *  @brief Functions to handle items, objects and inventories
 */

/**
 * Create a new object of the given item type and returns the pointer to it.
 * @param usItem the ID of the item type
 * @param ubStatus the status of the item, which should be a percentage value between 0 and 100
 * @return a pointer to the created object
 * @ingroup funclib-items
 */
OBJECTTYPE* CreateItem(UINT16 usItem, INT8 ubStatus);

/**
 * Creates a new money object of the specified amount
 * @param amt the amount of money
 * @return a pointer to the created object
 * @ingroup funclib-items
 */
OBJECTTYPE* CreateMoney(UINT32 amt);

/**
 * Places the given object at the specified grid
 * @param sGridNo gridNo to a grid on the current tactical map
 * @param pObject pointer to an object created by CreateItem or CreateMoney
 * @param bVisibility determines if the object is known by the player. Must match one of the Visibility enum
 * @ingroup funclib-items
 */
void PlaceItem(INT16 sGridNo, OBJECTTYPE* pObject, INT8 bVisibility);
void RemoveAllUnburiedItems(INT16 sGridNo, UINT8 ubLevel);

BOOLEAN RemoveObjectFromSoldierProfile(UINT8 ubProfile, UINT16 usItem);

MERCPROFILESTRUCT* GetMercProfile(UINT8 ubProfileID);
SOLDIERTYPE* FindSoldierByProfileID(UINT8 profileID);
std::vector<SOLDIERTYPE*> ListSoldiersFromTeam(UINT8 ubTeamID);

void CenterAtGridNo(INT16 sGridNo, bool fForce);
INT16 PythSpacesAway(INT16 sGridNo1, INT16 sGridNo2);
void IgniteExplosion(SOLDIERTYPE* owner, INT16 z, INT16 sGridNo, UINT16 item, INT8 level);
void SetOffBombsByFrequency(SOLDIERTYPE* s, INT8 bFrequency);

void TriggerNPCRecord(UINT8 ubTriggerNPC, UINT8 record);
void StrategicNPCDialogue(UINT8 ubProfileID, UINT16 usQuoteNum);
BOOLEAN TacticalCharacterDialogue(const SOLDIERTYPE* pSoldier, UINT16 usQuoteNum);

/**
 * Closes the dialogue menu with an NPC.
 */
void DeleteTalkingMenu();

BOOLEAN DoMercBattleSound_(SOLDIERTYPE* s, UINT8 battle_snd_id);

void ChangeSoldierStance(SOLDIERTYPE *pSoldier, UINT8 ubDesiredStance);
void ChangeSoldierState(SOLDIERTYPE* pSoldier, UINT16 usNewState, UINT16 usStartingAniCode, BOOLEAN fForce);

UINT32 PlayJA2SampleFromFile(const char* szFileName, UINT32 ubVolume, UINT32 ubLoops, UINT32 uiPan);

/**
 * Adds a recurring event that happens at the same time every day.
 * @param ubCallbackID strategic event ID
 * @param uiStartMin the time (minutes of day) that the event
 * @param uiParam a parameter that will be passed to the event handler
 */
void AddEveryDayStrategicEvent_(UINT8 ubCallbackID, UINT32 uiStartMin, UINT32 uiParam);

/**
 * Adds an one-off strategic event
 * @param ubCallbackID strategic event ID
 * @param uiMinStamp earliest time (in world-seconds) that this event will be processed
 * @param uiParam a parameter that will be passed to the event handler
 */
void AddStrategicEvent_(UINT8 ubCallbackID, UINT32 uiMinStamp, UINT32);
UINT32 GetWorldTotalMin();
UINT32 GetWorldTotalSeconds();
UINT32 GetWorldDay();

GROUP* CreateNewEnemyGroupDepartingSector(std::string sectorID, UINT8 ubNumAdmins, UINT8 ubNumTroops, UINT8 ubNumElites);

void StartQuest_(UINT8 ubQuestID, std::string sectorID);
void EndQuest_(UINT8 ubQuest, std::string sectorID);

void SetFactTrue(Fact);
void SetFactFalse(Fact);
BOOLEAN CheckFact(Fact, UINT8);

void ChangeSoldierState(SOLDIERTYPE* pSoldier, UINT16 usNewState, UINT16 usStartingAniCode, BOOLEAN fForce);

void AddEmailMessage(INT32 iMessageOffset, INT32 iMessageLength, INT32 iDate, UINT8 ubSender, BOOLEAN fAlreadyRead, INT32 uiFirstData, UINT32 uiSecondData);
void SetBookMark(INT32 iBookId);
void AddTransactionToPlayersBook(UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, INT32 iAmount);
void AddHistoryToPlayersLog(UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, const SGPSector& s);

/**
 * Gets the Merc Profile data object by profile ID
 * @param ubProfileID a valid Profile ID
 * @return pointer to a MERCPROFILESTRUCT struct
 * @ingroup funclib-mercs
 */
MERCPROFILESTRUCT* GetMercProfile(UINT8 ubProfileID);

/**
 * Sets the current laptop and WWW modes.
 * @param uiLaptopMode
 * @param uiWWWMode
 */
void SetLaptopModes(UINT32 uiLaptopMode, UINT32 uiWWWMode);

/**
 * Retrieves a key-value mapping from saved game states.
 * @param key provide a unique key so it will not clash with other mods
 * @return
 * @ingroup funclib-general
 */
ExtraGameStatesTable GetGameStates(std::string key);

/**
 * Copies the key-value mapping into the game states. Game states are
 * persisted in game saves, and can be retrieved with GetGameStates.
 * @param key provide a unique key so it will not clash with other mods
 * @param states a map of primitive types (string, numeric or boolean)
 * @ingroup funclib-general
 */
void PutGameStates(std::string key, ExtraGameStatesTable states);

/**
 * Refreshes the stocks and cash of all dealers.
 * @ingroup funclib-dealers
 */
void DailyCheckOnItemQuantities();

/**
 * Make at least X number of the given item available to buy.
 * @param bDealerID The ID of the dealer to update
 * @param usItemIndex The index of the Item
 * @param ubNumItems At least this Number of items will exist in the dealer's inventory
 * @ingroup funclib-dealers
 */
void GuaranteeAtLeastXItemsOfIndex_(INT8, UINT16, UINT8);

/** @ingroup funclib-dealers */
void RemoveRandomItemFromDealerInventory(INT8 bArmsDealerID, UINT16 usItemIndex, UINT8 ubHowMany);

/** @ingroup funclib-dealers */
std::vector<DEALER_ITEM_HEADER*> GetDealerInventory(UINT8 ubDealerID);

/** @ingroup funclib-dealers */
BOOLEAN StartShopKeeperTalking(UINT16 usQuoteNum);

/** @ingroup funclib-dealers */
void EnterShopKeeperInterfaceScreen(UINT8 ubArmsDealer);

/**
 * @defgroup ui-control UI controls
 * @brief Functions for controlling the game UI
 */

/**
 * Pops up a basic message box with only the text and an OK button. There is no callback on close and returns the control
 * flow to the current screen. This is for messages that requires player's immediate attention.
 * @param text
 * @ingroup ui-control
 */
void DoBasicMessageBox(ST::string text);

/**
 * Displays a pop-up text box in the tactical view. This is suitable for storytelling, or describing a scene, situation
 * or items. In Lua, this function is ExecuteTacticalTextBox.
 * @param sLeftPosition On-screen X-position of the text box
 * @param sTopPosition On-screen Y-position of the text box
 * @param pString Text to display
 * @ingroup ui-control
 */
void ExecuteTacticalTextBox_(INT16 sLeftPosition, INT16 sTopPosition, ST::string pString);

void AddExitGridToWorld_(INT32 mapIndex, UINT16 gridNo, UINT8 x, UINT8 y, UINT8 z);
void RemoveExitGridFromWorld(INT32 iMapIndex);
void RemoveStructFromMap(UINT16 gridNo, UINT32 tileType, UINT16 tileSubIndex);
void AddStructToMap(UINT16 gridNo, UINT32 tileType, UINT16 tileSubIndex);
