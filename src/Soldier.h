#pragma once

#include "Types.h"

#include "boost/shared_ptr.hpp"

struct SOLDIERTYPE;

class Soldier
{
public:
  Soldier(SOLDIERTYPE* s);

  /** Remove pending action. */
  void removePendingAction();

  /** Remove any pending animation. */
  void removePendingAnimation();

  bool hasPendingAction() const;

  void setPendingAction(UINT8 action);

  /**
   * Handle pending action.
   * Return true, when all further processing should be stopped. */
  bool handlePendingAction(bool inCombat);

protected:

  const char* getPofileName() const;

private:
  SOLDIERTYPE* mSoldier;
};

typedef boost::shared_ptr<Soldier> SoldierSP;

/** Get soldier object from the structure. */
boost::shared_ptr<Soldier> GetSoldier(struct SOLDIERTYPE* s);

/** Get soldier object from the structure. */
boost::shared_ptr<const Soldier> GetSoldier(const struct SOLDIERTYPE* s);
