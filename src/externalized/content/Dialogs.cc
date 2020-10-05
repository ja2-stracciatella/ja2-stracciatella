#include "Dialogs.h"
#include "Directories.h"
#include "MercProfile.h"
#include "Soldier_Profile.h"
#include <stdio.h>

const char* Content::GetDialogueTextFilename(const MercProfile &profile,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking)
{
	static char zFileName[164];
	uint8_t		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( useAlternateDialogueFile )
	{
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, NPCDATADIR "/d_%03d.edt", profile.getNum());
		}
	}
	else if (profile.isNPCorRPC()
			&& ( !profile.isRecruited()
				|| isCurrentlyTalking
				|| profile.isForcedNPCQuote()))
	{
		ubFileNumID = profile.getNum();

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if (profile.getNum() >= HERVE && profile.getNum() <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		{
		// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, NPCDATADIR "/%03d.edt", ubFileNumID);
		}
	}
	else
	{
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, MERCEDTDIR "/%03d.edt", profile.getNum());
		}
	}

	return( zFileName );
}

const char* Content::GetDialogueVoiceFilename(const MercProfile &profile, uint16_t usQuoteNum,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking,
						bool isRussianVersion)
{
	static char zFileName[164];
	uint8_t		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( useAlternateDialogueFile )
	{
		{
			// build name of wav file (characternum + quotenum)
			sprintf(zFileName, NPC_SPEECHDIR "/d_%03d_%03d.wav", profile.getNum(), usQuoteNum);
		}
	}
	else if (profile.isNPCorRPC()
			&& ( !profile.isRecruited()
				|| isCurrentlyTalking
				|| profile.isForcedNPCQuote()))
	{
		ubFileNumID = profile.getNum();

		// ATE: If we are merc profile ID #151-154, all use 151's data....
		if (profile.getNum() >= HERVE && profile.getNum() <= CARLO)
		{
			ubFileNumID = HERVE;
		}

		{
			sprintf(zFileName, NPC_SPEECHDIR "/%03d_%03d.wav", ubFileNumID, usQuoteNum);
		}
	}
	else
	{
		{
			if(isRussianVersion)
			{
				if (profile.isNPCorRPC() && profile.isRecruited())
				{
					sprintf(zFileName, SPEECHDIR "/r_%03d_%03d.wav", profile.getNum(), usQuoteNum);
				}
				else
				{
					// build name of wav file (characternum + quotenum)
					sprintf(zFileName, SPEECHDIR "/%03d_%03d.wav", profile.getNum(), usQuoteNum);
				}
			}
			else
			{
				// build name of wav file (characternum + quotenum)
				sprintf(zFileName, SPEECHDIR "/%03d_%03d.wav", profile.getNum(), usQuoteNum);
			}
		}
	}

	return( zFileName );
}
