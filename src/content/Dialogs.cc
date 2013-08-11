#include "Dialogs.h"

#include <stdio.h>

#include "Build/Directories.h"
#include "Build/Tactical/Soldier_Profile.h"
#include "Build/Tactical/Soldier_Profile_Type.h"
#include "MercProfile.h"

const char* Content::GetDialogueDataFilename(const MercProfile &profile, uint16_t usQuoteNum, bool fWavFile,
                                             bool useAlternateDialogueFile,
                                             bool isCurrentlyTalking,
                                             bool isRussianVersion,
                                             bool mannyIsBartender)
{
	static char zFileName[164];
	uint8_t		ubFileNumID;

	// Are we an NPC OR an RPC that has not been recruited?
	// ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an npc quote file
	if ( useAlternateDialogueFile )
	{
		if ( fWavFile )
		{
			// build name of wav file (characternum + quotenum)
			sprintf(zFileName, NPC_SPEECHDIR "/d_%03d_%03d.wav", profile.getNum(), usQuoteNum);
		}
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, NPCDATADIR "/d_%03d.edt", profile.getNum());
		}
	}
	else if ( profile.getNum() >= FIRST_RPC
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

		// If we are character #155, check fact!
		if ( profile.getNum() == MANNY && !mannyIsBartender )
		{
			ubFileNumID = MANNY;
		}


		if ( fWavFile )
		{
			sprintf(zFileName, NPC_SPEECHDIR "/%03d_%03d.wav", ubFileNumID, usQuoteNum);
		}
		else
		{
		// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, NPCDATADIR "/%03d.edt", ubFileNumID);
		}
	}
	else
	{
		if ( fWavFile )
		{
      if(isRussianVersion)
      {
        if (profile.getNum() >= FIRST_RPC && profile.isRecruited())
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
		else
		{
			// assume EDT files are in EDT directory on HARD DRIVE
			sprintf(zFileName, MERCEDTDIR "/%03d.edt", profile.getNum());
		}
	}

	return( zFileName );
}
