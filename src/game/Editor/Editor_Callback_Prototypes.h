#ifndef __EDITOR_CALLBACK_PROTOTYPES_H
#define __EDITOR_CALLBACK_PROTOTYPES_H

#include "Button_System.h"


// Editor Tabs
void TaskTerrainCallback(GUI_BUTTON *btn,UINT32 reason);
void TaskBuildingCallback(GUI_BUTTON *btn,UINT32 reason);
void TaskItemsCallback(GUI_BUTTON *btn,UINT32 reason);
void TaskMercsCallback(GUI_BUTTON *btn,UINT32 reason);
void TaskMapInfoCallback(GUI_BUTTON *btn,UINT32 reason);
void TaskOptionsCallback(GUI_BUTTON *btn,UINT32 reason);
// Options Tab Callbacks
void BtnLoadCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnSaveCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnCancelCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnQuitCallback( GUI_BUTTON *btn, UINT32 reason );
void BtnNewMapCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnNewBasementCallback( GUI_BUTTON *btn, UINT32 reason );
void BtnNewCavesCallback( GUI_BUTTON *btn, UINT32 reason );
void BtnChangeTilesetCallback(GUI_BUTTON *btn,UINT32 reason);
// Mercs Tab Callbacks
void MercsTogglePlayers( GUI_BUTTON *btn, UINT32 reason );
void MercsToggleEnemies( GUI_BUTTON *btn, UINT32 reason );
void MercsToggleCreatures( GUI_BUTTON *btn, UINT32 reason );
void MercsToggleRebels( GUI_BUTTON *btn, UINT32 reason );
void MercsToggleCivilians( GUI_BUTTON *btn, UINT32 reason );
void MercsPlayerTeamCallback(GUI_BUTTON *btn,UINT32 reason);
void MercsEnemyTeamCallback(GUI_BUTTON *btn,UINT32 reason);
void MercsCreatureTeamCallback(GUI_BUTTON *btn,UINT32 reason);
void MercsRebelTeamCallback(GUI_BUTTON *btn,UINT32 reason);
void MercsCivilianTeamCallback(GUI_BUTTON *btn,UINT32 reason);
void MercsDetailedPlacementCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsPriorityExistanceCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsHasKeysCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsNextCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsDeleteCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsGeneralModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsAttributesModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsInventoryModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsAppearanceModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsProfileModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetEnemyColorCodeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetOrdersCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetAttitudeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsDirectionSetCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsFindSelectedMercCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetRelativeEquipmentCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetRelativeAttributesCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsToggleColorModeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetColorsCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsSetBodyTypeCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsInventorySlotCallback( GUI_BUTTON *btn, UINT32 reason );
void MouseMovedInMercRegion( MOUSE_REGION *reg, UINT32 reason );
void MouseClickedInMercRegion( MOUSE_REGION *reg, UINT32 reason );
void MercsCivilianGroupCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleAction1Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleAction2Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleAction3Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleAction4Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData1ACallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData1BCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData2ACallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData2BCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData3ACallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData3BCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData4ACallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleData4BCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleClearCallback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleToggleVariance1Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleToggleVariance2Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleToggleVariance3Callback( GUI_BUTTON *btn, UINT32 reason );
void MercsScheduleToggleVariance4Callback( GUI_BUTTON *btn, UINT32 reason );
// Terrain Tab Callbacks
void BtnLandRaiseCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnLandLowerCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnIncBrushDensityCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnDecBrushDensityCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnFgGrndCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnBkGrndCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnObjectCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnBanksCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnRoadsCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnDebrisCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnBrushCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnObject1Callback(GUI_BUTTON *btn,UINT32 reason);
void BtnObject2Callback(GUI_BUTTON *btn,UINT32 reason);
void BtnFillCallback(GUI_BUTTON *btn,UINT32 reason);
void TerrainTileButtonRegionCallback(MOUSE_REGION *reg,UINT32 reason);
// Items Tab Callbacks
void ItemsWeaponsCallback(GUI_BUTTON *btn,UINT32 reason);
void ItemsAmmoCallback(GUI_BUTTON *btn,UINT32 reason);
void ItemsArmourCallback(GUI_BUTTON *btn,UINT32 reason);
void ItemsExplosivesCallback(GUI_BUTTON *btn,UINT32 reason);
void ItemsEquipment1Callback(GUI_BUTTON *btn,UINT32 reason);
void ItemsEquipment2Callback(GUI_BUTTON *btn,UINT32 reason);
void ItemsEquipment3Callback(GUI_BUTTON *btn,UINT32 reason);
void ItemsTriggersCallback(GUI_BUTTON *btn,UINT32 reason);
void ItemsKeysCallback(GUI_BUTTON *btn, UINT32 reason);
void ItemsLeftScrollCallback(GUI_BUTTON *btn, UINT32 reason);
void ItemsRightScrollCallback(GUI_BUTTON *btn, UINT32 reason);
void MouseMovedInItemsRegion(MOUSE_REGION *reg, UINT32 reason);
void MouseClickedInItemsRegion(MOUSE_REGION *reg, UINT32 reason);
// MapInfo Callbacks
void BtnFakeLightCallback(GUI_BUTTON *btn,UINT32 reason);
void MapInfoPrimeTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfoNightTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfo24HourTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason );
void BtnDrawLightsCallback(GUI_BUTTON *btn,UINT32 reason);
void MapInfoDrawExitGridCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfoEntryPointsCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfoNormalRadioCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfoBasementRadioCallback( GUI_BUTTON *btn, UINT32 reason );
void MapInfoCavesRadioCallback( GUI_BUTTON *btn, UINT32 reason );
// Building callbacks
void BuildingWallCallback( GUI_BUTTON *btn, UINT32 reason );
void BuildingDoorCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingWindowCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingRoofCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingCrackWallCallback( GUI_BUTTON *btn, UINT32 reason );
void BuildingFurnitureCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingDecalCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingFloorCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingToiletCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingSmartWallCallback( GUI_BUTTON *btn, UINT32 reason );
void BuildingSmartDoorCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingSmartWindowCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingSmartCrackWallCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingDoorKeyCallback(GUI_BUTTON *btn, UINT32 reason );
void BuildingNewRoomCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingNewRoofCallback(GUI_BUTTON *btn, UINT32 reason );
void BuildingSawRoomCallback( GUI_BUTTON *btn, UINT32 reason );
void BuildingKillBuildingCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingCopyBuildingCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingMoveBuildingCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingCaveDrawingCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingDrawRoomNumCallback(GUI_BUTTON *btn, UINT32 reason);
void BuildingEraseRoomNumCallback( GUI_BUTTON *btn, UINT32 reason );
void BuildingToggleRoofViewCallback(GUI_BUTTON *btn,UINT32 reason);
void BuildingToggleWallViewCallback(GUI_BUTTON *btn,UINT32 reason);
void BuildingToggleInfoViewCallback(GUI_BUTTON *btn,UINT32 reason);

// ItemStats Callbacks
void ItemStatsToggleHideCallback( GUI_BUTTON *btn, UINT32 reason );
void ItemStatsDeleteCallback( GUI_BUTTON *btn, UINT32 reason );


// Various Callbacks
void BtnUndoCallback(GUI_BUTTON *btn,UINT32 reason);
void BtnEraseCallback(GUI_BUTTON *btn,UINT32 reason);



#endif
