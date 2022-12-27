#pragma once

#include <stdint.h>
#include <string_theory/format>

class MercProfile;

namespace Content
{
	ST::string GetDialogueTextFilename(const MercProfile &profile,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking);

	ST::string GetDialogueVoiceFilename(const MercProfile &profile, uint16_t usQuoteNum,
						bool useAlternateDialogueFile,
						bool isCurrentlyTalking,
						bool isRussianVersion);
}
