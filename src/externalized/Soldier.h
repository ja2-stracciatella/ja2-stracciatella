#pragma once

#include "Types.h"

#include "Tactical/Soldier_Control.h"

#include <memory>

struct SOLDIERTYPE;

class Soldier
{
public:
	Soldier(SOLDIERTYPE* s);

	/** Remove pending action. */
	void removePendingAction();

	/** Remove any pending animation. */
	void removePendingAnimation();

	bool hasPendingAction(UINT8 action = NO_PENDING_ACTION) const;
	static bool anyoneHasPendingAction(UINT8 action, UINT8 team = OUR_TEAM);

	void setPendingAction(UINT8 action);

	/**
	 * Handle pending action.
	 * Return true, when all further processing should be stopped. */
	bool handlePendingAction(bool inCombat);

	/** Switch to the best night goggles. */
	void putNightHeadGear();

	/** Switch to sungoggles or remove the night goggles. */
	void putDayHeadGear();

protected:

	void switchHeadGear(int switchDirection);

	/** Get the profile name for the solider **/
	const ST::string& getPofileName() const;

	/** Get free head slot or NO_SLOT if the both are occupied. */
	int8_t getFreeHeadSlot() const;

	/** Find free pocket.  Return the slot number or NO_SLOT. */
	int8_t getFreePocket() const;

	/** Swap inventory slots. */
	void swapInventorySlots(int8_t firstSlot, int8_t secondSlot);

private:
	SOLDIERTYPE* mSoldier;
};

typedef std::shared_ptr<Soldier> SoldierSP;

/** Get soldier object from the structure. */
std::shared_ptr<Soldier> GetSoldier(struct SOLDIERTYPE* s);

/** Get soldier object from the structure. */
std::shared_ptr<const Soldier> GetSoldier(const struct SOLDIERTYPE* s);
