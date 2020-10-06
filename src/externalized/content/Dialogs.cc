#include "Dialogs.h"
#include "Directories.h"
#include "MercProfile.h"
#include "Soldier_Profile.h"
#include <string_theory/format>

ST::string Content::GetDialogueTextFilename(const MercProfile &profile,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking)
{
	ST::string	zFileName;
	uint8_t		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( useAlternateDialogueFile )
	{
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			zFileName = ST::format("{}/d_{03d}.edt", NPCDATADIR, profile.getID());
		}
	}
	else if (profile.isNPCorRPC()
			&& ( !profile.isRecruited()
				|| isCurrentlyTalking
				|| profile.isForcedNPCQuote()))
	{
		ubFileNumID = profile.getID();

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if (profile.getID() >= HERVE && profile.getID() <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		{
		// assume EDT files are in EDT directory on HARD DRIVE
			zFileName = ST::format("{}/{03d}.edt", NPCDATADIR, ubFileNumID);
		}
	}
	else
	{
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			zFileName = ST::format("{}/{03d}.edt", MERCEDTDIR, profile.getID());
		}
	}

	return zFileName;
}

ST::string Content::GetDialogueVoiceFilename(const MercProfile &profile, uint16_t usQuoteNum,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking,
						bool isRussianVersion)
{
	ST::string	zFileName;
	uint8_t		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( useAlternateDialogueFile )
	{
		{
			// build name of wav file (characternum + quotenum)
			zFileName =  ST::format("{}/d_{03d}_{03d}.wav", NPC_SPEECHDIR, profile.getID(), usQuoteNum);
		}
	}
	else if (profile.isNPCorRPC()
			&& ( !profile.isRecruited()
				|| isCurrentlyTalking
				|| profile.isForcedNPCQuote()))
	{
		ubFileNumID = profile.getID();

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if (profile.getID() >= HERVE && profile.getID() <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		zFileName = ST::format("{}/{03d}_{03d}.wav", NPC_SPEECHDIR, ubFileNumID, usQuoteNum);
	}
	else
	{
		{
			if(isRussianVersion)
			{
				if (profile.isNPCorRPC() && profile.isRecruited())
				{
					zFileName = ST::format("{}/r_{03d}_{03d}.wav", SPEECHDIR, profile.getID(), usQuoteNum);
				}
				else
				{
					// build name of wav file (characternum + quotenum)
					zFileName = ST::format("{}/{03d}_{03d}.wav", SPEECHDIR, profile.getID(), usQuoteNum);
				}
			}
			else
			{
				// build name of wav file (characternum + quotenum)
				zFileName = ST::format("{}/{03d}_{03d}.wav", SPEECHDIR, profile.getID(), usQuoteNum);
			}
		}
	}

	return zFileName;
}
