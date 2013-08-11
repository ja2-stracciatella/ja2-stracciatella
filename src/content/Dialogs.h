#pragma once

#include <stdint.h>

class MercProfile;

namespace Content
{
  const char* GetDialogueTextFilename(const MercProfile &profile,
                                      bool useAlternateDialogueFile,
                                      bool isCurrentlyTalking);

  const char* GetDialogueVoiceFilename(const MercProfile &profile, uint16_t usQuoteNum,
                                       bool useAlternateDialogueFile,
                                       bool isCurrentlyTalking,
                                       bool isRussianVersion);
}
