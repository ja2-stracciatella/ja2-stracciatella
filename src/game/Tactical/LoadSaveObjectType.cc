#include "Debug.h"
#include "LoadSaveObjectType.h"
#include "LoadSaveData.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "ItemModel.h"
#include <algorithm>


static void ReplaceInvalidItem(UINT16 & usItem)
{
	auto * item{ GCM->getItem(usItem, ItemSystem::nothrow) };
	if (!item)
	{
		SLOGW("Item (index {}) is not defined and will be ignored. Maybe the file was saved for a different game version", usItem);
		usItem = NONE;
	}
}

void ExtractObject(DataReader& d, OBJECTTYPE* const o)
{
	size_t start = d.getConsumed();
	EXTR_U16(d, o->usItem)
	EXTR_U8(d, o->ubNumberOfObjects)
	EXTR_SKIP(d, 1)

	ReplaceInvalidItem(o->usItem);
	const ItemModel* item = GCM->getItem(o->usItem);
	switch (item->getItemClass())
	{
		case IC_AMMO:
			EXTR_U8A(d, o->ubShotsLeft, lengthof(o->ubShotsLeft))
			EXTR_SKIP(d, 4)
			break;

		case IC_GUN:
			EXTR_I8(d, o->bGunStatus)
			EXTR_U8(d, o->ubGunAmmoType)
			EXTR_U8(d, o->ubGunShotsLeft)
			EXTR_SKIP(d, 1)
			EXTR_U16(d, o->usGunAmmoItem)
			EXTR_I8(d, o->bGunAmmoStatus)
			EXTR_SKIP(d, 5)
			break;

		case IC_KEY:
			EXTR_I8A(d, o->bKeyStatus, lengthof(o->bKeyStatus))
			EXTR_U8(d, o->ubKeyID)
			EXTR_SKIP(d, 5)
			break;

		case IC_MONEY:
			EXTR_I8(d, o->bMoneyStatus)
			EXTR_SKIP(d, 3)
			EXTR_U32(d, o->uiMoneyAmount)
			EXTR_SKIP(d, 4)
			break;

		case IC_MISC:
			switch (o->usItem)
			{
				case ACTION_ITEM:
					EXTR_I8(d, o->bBombStatus)
					EXTR_I8(d, o->bDetonatorType)
					EXTR_U16(d, o->usBombItem)
					EXTR_I8(d, o->bFrequency) // XXX unclear when to use bDelay
					EXTR_U8(d, o->ubBombOwner)
					EXTR_U8(d, o->bActionValue)
					EXTR_U8(d, o->ubTolerance)
					EXTR_SKIP(d, 4)
					break;

				case OWNERSHIP:
					EXTR_U8(d, o->ubOwnerProfile)
					EXTR_U8(d, o->ubOwnerCivGroup)
					EXTR_SKIP(d, 10)
					break;

				case SWITCH:
					EXTR_I8(d, o->bBombStatus)
					EXTR_I8(d, o->bDetonatorType)
					EXTR_U16(d, o->usBombItem)
					EXTR_I8(d, o->bFrequency)
					EXTR_U8(d, o->ubBombOwner)
					EXTR_U8(d, o->bActionValue)
					EXTR_U8(d, o->ubTolerance)
					EXTR_SKIP(d, 4)
					break;

				default: goto extract_status;
			}
			break;

		default:
extract_status:
			EXTR_I8A(d, o->bStatus, lengthof(o->bStatus))
			EXTR_SKIP(d, 4)
			break;
	}
	EXTR_U16A(d, o->usAttachItem, lengthof(o->usAttachItem))
	EXTR_I8A(d, o->bAttachStatus, lengthof(o->bAttachStatus))
	EXTR_I8(d, o->fFlags)
	EXTR_U8(d, o->ubMission)
	EXTR_I8(d, o->bTrap)
	EXTR_U8(d, o->ubImprintID)
	EXTR_U8(d, o->ubWeight)
	EXTR_U8(d, o->fUsed)
	EXTR_SKIP(d, 2)
	Assert(d.getConsumed() == start + 36);

	// Check and remove invalid items in attachment slots
	for (UINT16 & i : o->usAttachItem)
	{
		ReplaceInvalidItem(i);
	}
}


void InjectObject(DataWriter& d, const OBJECTTYPE* o)
{
	size_t start = d.getConsumed();
	INJ_U16(d, o->usItem)
	INJ_U8(d, o->ubNumberOfObjects)
	INJ_SKIP(d, 1)
	switch (GCM->getItem(o->usItem)->getItemClass())
	{
		case IC_AMMO:
			INJ_U8A(d, o->ubShotsLeft, lengthof(o->ubShotsLeft))
			INJ_SKIP(d, 4)
			break;

		case IC_GUN:
			INJ_I8(d, o->bGunStatus)
			INJ_U8(d, o->ubGunAmmoType)
			INJ_U8(d, o->ubGunShotsLeft)
			INJ_SKIP(d, 1)
			INJ_U16(d, o->usGunAmmoItem)
			INJ_I8(d, o->bGunAmmoStatus)
			INJ_SKIP(d, 5)
			break;

		case IC_KEY:
			INJ_I8A(d, o->bKeyStatus, lengthof(o->bKeyStatus))
			INJ_U8(d, o->ubKeyID)
			INJ_SKIP(d, 5)
			break;

		case IC_MONEY:
			INJ_I8(d, o->bMoneyStatus)
			INJ_SKIP(d, 3)
			INJ_U32(d, o->uiMoneyAmount)
			INJ_SKIP(d, 4)
			break;

		case IC_MISC:
			switch (o->usItem)
			{
				case ACTION_ITEM:
					INJ_I8(d, o->bBombStatus)
					INJ_I8(d, o->bDetonatorType)
					INJ_U16(d, o->usBombItem)
					INJ_I8(d, o->bFrequency) // XXX unclear when to use bDelay
					INJ_U8(d, o->ubBombOwner)
					INJ_U8(d, o->bActionValue)
					INJ_U8(d, o->ubTolerance)
					INJ_SKIP(d, 4)
					break;

				case OWNERSHIP:
					INJ_U8(d, o->ubOwnerProfile)
					INJ_U8(d, o->ubOwnerCivGroup)
					INJ_SKIP(d, 10)
					break;

				case SWITCH:
					INJ_I8(d, o->bBombStatus)
					INJ_I8(d, o->bDetonatorType)
					INJ_U16(d, o->usBombItem)
					INJ_I8(d, o->bFrequency)
					INJ_U8(d, o->ubBombOwner)
					INJ_U8(d, o->bActionValue)
					INJ_U8(d, o->ubTolerance)
					INJ_SKIP(d, 4)
					break;

				default: goto inject_status;
			}
			break;

		default:
inject_status:
			INJ_I8A(d, o->bStatus, lengthof(o->bStatus))
			INJ_SKIP(d, 4)
			break;
	}
	INJ_U16A(d, o->usAttachItem, lengthof(o->usAttachItem))
	INJ_I8A(d, o->bAttachStatus, lengthof(o->bAttachStatus))
	INJ_I8(d, o->fFlags)
	INJ_U8(d, o->ubMission)
	INJ_I8(d, o->bTrap)
	INJ_U8(d, o->ubImprintID)
	d.writeU8(static_cast<UINT8>(std::clamp(Weight(*o), 1, 255)));
	INJ_U8(d, o->fUsed)
	INJ_SKIP(d, 2)
	Assert(d.getConsumed() == start + 36);
}
