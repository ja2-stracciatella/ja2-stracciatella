#ifndef ARMS_DEALER_H
#define ARMS_DEALER_H

//the enums for the different kinds of arms dealers
enum ArmsDealerType
{
	NOT_VALID_DEALER = -1,
	ARMS_DEALER_BUYS_SELLS = 0,
	ARMS_DEALER_SELLS_ONLY,
	ARMS_DEALER_BUYS_ONLY,
	ARMS_DEALER_REPAIRS,
};

// various flags which control the dealer's operations
enum ArmsDealerFlag
{
	ONLY_USED_ITEMS,
	SOME_USED_ITEMS,       //The arms dealer can have used items in his inventory
	GIVES_CHANGE,          //The arms dealer will give the required change when doing a transaction
	ACCEPTS_GIFTS,         //The arms dealer is the kind of person who will accept gifts
	HAS_NO_INVENTORY,      //The arms dealer does not carry any inventory
	DELAYS_REPAIR,
	REPAIRS_ELECTRONICS,
	SELLS_ALCOHOL,
	SELLS_FUEL,
	BUYS_EVERYTHING,       //these guys will buy nearly anything from the player, regardless of what they carry for sale!

	NUM_FLAGS
};

//enums for the various arms dealers
enum ArmsDealerID
{
	ARMS_DEALER_INVALID = -1,
	ARMS_DEALER_BOBBYR  = ARMS_DEALER_INVALID,

	ARMS_DEALER_FIRST   = 0,
	ARMS_DEALER_TONY    = ARMS_DEALER_FIRST,
	ARMS_DEALER_FRANZ,
	ARMS_DEALER_KEITH,
	ARMS_DEALER_JAKE,
	ARMS_DEALER_GABBY,

	ARMS_DEALER_DEVIN,
	ARMS_DEALER_HOWARD,
	ARMS_DEALER_SAM,
	ARMS_DEALER_FRANK,

	ARMS_DEALER_BAR_BRO_1,
	ARMS_DEALER_BAR_BRO_2,
	ARMS_DEALER_BAR_BRO_3,
	ARMS_DEALER_BAR_BRO_4,

	ARMS_DEALER_MICKY,

	ARMS_DEALER_ARNIE,
	ARMS_DEALER_FREDO,
	ARMS_DEALER_PERKO,

	// added only in GameVersion 54
	ARMS_DEALER_ELGIN,

	// added only in GameVersion 55
	ARMS_DEALER_MANNY,

	NUM_ARMS_DEALERS
};

static inline ArmsDealerID operator ++(ArmsDealerID& a)
{
	return a = (ArmsDealerID)(a + 1);
}

#endif
