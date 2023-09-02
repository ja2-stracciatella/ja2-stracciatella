#pragma once

#include "Arms_Dealer.h"
#include "MercSystem.h"
#include "Json.h"

#include <bitset>
#include <vector>

class DealerModel
{
public:
	DealerModel(UINT8 dealerID, UINT8 mercID, ArmsDealerType dealerType,
		FLOAT buyingPrice, FLOAT sellingPrice, FLOAT repairSped, FLOAT repairCost,
		INT32 initialCash, std::bitset<ArmsDealerFlag::NUM_FLAGS> flags
	);

	const UINT8 dealerID;

	// Merc profile Id for the dealer
	const UINT8 profileID;

	// Whether he buys/sells, sells, buys, or repairs
	const ArmsDealerType type;

	// The price modifier used when this dealer is BUYING something.
	const FLOAT buyingPrice;

	// The price modifier used when this dealer is SELLING something.
	const FLOAT sellingPrice;

	// Modifier to the speed at which a repairman repairs things
	const FLOAT repairSpeed;

	// Modifier to the price a repairman charges for repairs
	const FLOAT repairCost;

	// How much cash dealer starts with (we now reset to this amount once / day)
	const INT32 initialCash;

	BOOLEAN hasFlag(ArmsDealerFlag flag) const;

	BOOLEAN doesRepairs() const;

	// name of the data file holding the inventory data
	ST::string getInventoryDataFileName(const MercSystem* mercSystem) const;

	static const DealerModel* deserialize(const JsonValue& val, const MercSystem* mercSystem, UINT8 dealerIndex);

	static void validateData(std::vector<const DealerModel*> models, const MercSystem* mercSystem);

protected:
	// various flags which control the dealer's operations
	const std::bitset<ArmsDealerFlag::NUM_FLAGS> flags;
};

