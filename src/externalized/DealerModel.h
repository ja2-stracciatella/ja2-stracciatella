#pragma once

#include "Arms_Dealer.h"
#include "JA2Types.h"
#include <bitset>
#include <rapidjson/document.h>
#include <vector>

class DealerModel
{
public:
	DealerModel(UINT8 dealerID, UINT8 mercID, ST::string internalName, ArmsDealerType dealerType, 
		FLOAT buyingPrice, FLOAT sellingPrice, FLOAT repairSped, FLOAT repairCost,
		INT32 initialCash, std::bitset<ArmsDealerFlag::NUM_FLAGS> flags
	);
	
	const UINT8 dealerID;

	// Merc profile Id for the dealer
	const UINT8 profileID;            

	// Internal merc name of the dealer
	const ST::string internalName;

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
	ST::string getInventoryDataFileName() const;

	static const DealerModel* deserialize(rapidjson::Value& val, UINT8 dealerIndex);

	static void validateData(std::vector<const DealerModel*> models);

protected:
	// various flags which control the dealer's operations
	const std::bitset<ArmsDealerFlag::NUM_FLAGS> flags;
};

