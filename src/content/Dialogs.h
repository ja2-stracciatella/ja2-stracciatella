#pragma once

#include <stdint.h>

class MercProfile;

namespace Content
{
  const char* GetDialogueDataFilename(const MercProfile &profile, uint16_t usQuoteNum, bool fWavFile,
                                      bool useAlternateDialogueFile,
                                      bool isCurrentlyTalking,
                                      bool isRussianVersion,
                                      bool mannyIsBartender);
}
