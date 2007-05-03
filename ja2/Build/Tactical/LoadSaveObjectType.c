#include "Debug.h"
#include "LoadSaveObjectType.h"
#include "LoadSaveData.h"


const BYTE* ExtractObject(const BYTE* Src, OBJECTTYPE* o)
{
	const BYTE* S = Src;

	EXTR_U16(S, o->usItem)
	EXTR_U8(S, o->ubNumberOfObjects)
	EXTR_SKIP(S, 1)
	switch (Item[o->usItem].usItemClass)
	{
		case IC_AMMO:
			EXTR_U8A(S, o->ubShotsLeft, lengthof(o->ubShotsLeft))
			EXTR_SKIP(S, 4)
			break;

		case IC_GUN:
			EXTR_I8(S, o->bGunStatus)
			EXTR_U8(S, o->ubGunAmmoType)
			EXTR_U8(S, o->ubGunShotsLeft)
			EXTR_SKIP(S, 1)
			EXTR_U16(S, o->usGunAmmoItem)
			EXTR_I8(S, o->bGunAmmoStatus)
			EXTR_U8A(S, o->ubGunUnused, lengthof(o->ubGunUnused))
			EXTR_SKIP(S, 3)
			break;

		case IC_KEY:
			EXTR_I8A(S, o->bKeyStatus, lengthof(o->bKeyStatus))
			EXTR_U8(S, o->ubKeyID)
			EXTR_U8A(S, o->ubKeyUnused, lengthof(o->ubKeyUnused))
			EXTR_SKIP(S, 4)
			break;

		case IC_MONEY:
			EXTR_I8(S, o->bMoneyStatus)
			EXTR_SKIP(S, 3)
			EXTR_U32(S, o->uiMoneyAmount)
			EXTR_U8A(S, o->ubMoneyUnused, lengthof(o->ubMoneyUnused))
			EXTR_SKIP(S, 1)
			break;

		case IC_MISC:
			switch (o->usItem)
			{
				case ACTION_ITEM:
					EXTR_I8(S, o->bBombStatus)
					EXTR_I8(S, o->bDetonatorType)
					EXTR_U16(S, o->usBombItem)
					EXTR_I8(S, o->bFrequency) // XXX unclear when to use bDelay
					EXTR_U8(S, o->ubBombOwner)
					EXTR_U8(S, o->bActionValue)
					EXTR_U8(S, o->ubTolerance)
					EXTR_SKIP(S, 4)
					break;

				case OWNERSHIP:
					EXTR_U8(S, o->ubOwnerProfile)
					EXTR_U8(S, o->ubOwnerCivGroup)
					EXTR_U8A(S, o->ubOwnershipUnused, lengthof(o->ubOwnershipUnused))
					EXTR_SKIP(S, 4)
					break;

				case SWITCH:
					EXTR_I8(S, o->bBombStatus)
					EXTR_I8(S, o->bDetonatorType)
					EXTR_U16(S, o->usBombItem)
					EXTR_I8(S, o->bFrequency)
					EXTR_U8(S, o->ubBombOwner)
					EXTR_U8(S, o->bActionValue)
					EXTR_U8(S, o->ubTolerance)
					EXTR_SKIP(S, 4)
					break;

				default: goto extract_status;
			}

		default:
extract_status:
			EXTR_I8A(S, o->bStatus, lengthof(o->bStatus))
			EXTR_SKIP(S, 4)
			break;
	}
	EXTR_U16A(S, o->usAttachItem, lengthof(o->usAttachItem))
	EXTR_I8A(S, o->bAttachStatus, lengthof(o->bAttachStatus))
	EXTR_I8(S, o->fFlags)
	EXTR_U8(S, o->ubMission)
	EXTR_I8(S, o->bTrap)
	EXTR_U8(S, o->ubImprintID)
	EXTR_U8(S, o->ubWeight)
	EXTR_U8(S, o->fUsed)
	EXTR_SKIP(S, 2)

	Assert(S = Src + 36);
	return S;
}


BYTE* InjectObject(BYTE* Dst, const OBJECTTYPE* o)
{
	BYTE* D = Dst;

	INJ_U16(D, o->usItem)
	INJ_U8(D, o->ubNumberOfObjects)
	INJ_SKIP(D, 1)
	switch (Item[o->usItem].usItemClass)
	{
		case IC_AMMO:
			INJ_U8A(D, o->ubShotsLeft, lengthof(o->ubShotsLeft))
			INJ_SKIP(D, 4)
			break;

		case IC_GUN:
			INJ_I8(D, o->bGunStatus)
			INJ_U8(D, o->ubGunAmmoType)
			INJ_U8(D, o->ubGunShotsLeft)
			INJ_SKIP(D, 1)
			INJ_U16(D, o->usGunAmmoItem)
			INJ_I8(D, o->bGunAmmoStatus)
			INJ_U8A(D, o->ubGunUnused, lengthof(o->ubGunUnused))
			INJ_SKIP(D, 3)
			break;

		case IC_KEY:
			INJ_I8A(D, o->bKeyStatus, lengthof(o->bKeyStatus))
			INJ_U8(D, o->ubKeyID)
			INJ_U8A(D, o->ubKeyUnused, lengthof(o->ubKeyUnused))
			INJ_SKIP(D, 4)
			break;

		case IC_MONEY:
			INJ_I8(D, o->bMoneyStatus)
			INJ_SKIP(D, 3)
			INJ_U32(D, o->uiMoneyAmount)
			INJ_U8A(D, o->ubMoneyUnused, lengthof(o->ubMoneyUnused))
			INJ_SKIP(D, 1)
			break;

		case IC_MISC:
			switch (o->usItem)
			{
				case ACTION_ITEM:
					INJ_I8(D, o->bBombStatus)
					INJ_I8(D, o->bDetonatorType)
					INJ_U16(D, o->usBombItem)
					INJ_I8(D, o->bFrequency) // XXX unclear when to use bDelay
					INJ_U8(D, o->ubBombOwner)
					INJ_U8(D, o->bActionValue)
					INJ_U8(D, o->ubTolerance)
					INJ_SKIP(D, 4)
					break;

				case OWNERSHIP:
					INJ_U8(D, o->ubOwnerProfile)
					INJ_U8(D, o->ubOwnerCivGroup)
					INJ_U8A(D, o->ubOwnershipUnused, lengthof(o->ubOwnershipUnused))
					INJ_SKIP(D, 4)
					break;

				case SWITCH:
					INJ_I8(D, o->bBombStatus)
					INJ_I8(D, o->bDetonatorType)
					INJ_U16(D, o->usBombItem)
					INJ_I8(D, o->bFrequency)
					INJ_U8(D, o->ubBombOwner)
					INJ_U8(D, o->bActionValue)
					INJ_U8(D, o->ubTolerance)
					INJ_SKIP(D, 4)
					break;

				default: goto extract_status;
			}

		default:
extract_status:
			INJ_I8A(D, o->bStatus, lengthof(o->bStatus))
			INJ_SKIP(D, 4)
			break;
	}
	INJ_U16A(D, o->usAttachItem, lengthof(o->usAttachItem))
	INJ_I8A(D, o->bAttachStatus, lengthof(o->bAttachStatus))
	INJ_I8(D, o->fFlags)
	INJ_U8(D, o->ubMission)
	INJ_I8(D, o->bTrap)
	INJ_U8(D, o->ubImprintID)
	INJ_U8(D, o->ubWeight)
	INJ_U8(D, o->fUsed)
	INJ_SKIP(D, 2)

	Assert(D = Dst + 36);
	return D;
}
