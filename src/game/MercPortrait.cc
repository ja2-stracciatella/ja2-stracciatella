#include "Directories.h"
#include "MercPortrait.h"
#include "Soldier_Profile_Type.h"
#include "VObject.h"


static SGPVObject* LoadPortrait(MERCPROFILESTRUCT const& p, char const* const subdir)
{
	ST::string filename = ST::format(FACESDIR "/{}{02d}.sti", subdir, p.ubFaceIndex);
	return AddVideoObjectFromFile(filename);
}


SGPVObject* Load33Portrait(   MERCPROFILESTRUCT const& p) { return LoadPortrait(p, "33face/");   }
SGPVObject* Load65Portrait(   MERCPROFILESTRUCT const& p) { return LoadPortrait(p, "65face/");   }
SGPVObject* LoadBigPortrait(  MERCPROFILESTRUCT const& p) { return LoadPortrait(p, "bigfaces/"); }
SGPVObject* LoadSmallPortrait(MERCPROFILESTRUCT const& p) { return LoadPortrait(p, "");          }
