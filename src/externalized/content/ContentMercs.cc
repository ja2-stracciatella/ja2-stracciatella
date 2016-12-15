#include "ContentMercs.h"

#include "externalized/ContentManager.h"
#include "Dialogs.h"
#include "externalized/MercProfile.h"

bool Content::canMercBeHired(const ContentManager *cm, uint16_t profileNum)
{
  return cm->doesGameResExists(Content::GetDialogueTextFilename(MercProfile(profileNum), false, false));
}
