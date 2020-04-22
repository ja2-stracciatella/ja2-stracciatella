#include "Debug.h"
#include "Environment.h"
#include "FileMan.h"
#include "JAScreens.h"
#include "LoadSaveData.h"
#include "LoadSaveLightSprite.h"
#include "ScreenIDs.h"
#include "Sys_Globals.h"
#include "MemMan.h"

void ExtractLightSprite(HWFILE const f, UINT32 const light_time)
{
	INT16  x;
	INT16  y;
	UINT32 flags;
	UINT8  str_len;

	BYTE data[25];
	FileRead(f, data, sizeof(data));

	DataReader d{data};
	EXTR_I16(d, x)
	EXTR_I16(d, y)
	EXTR_SKIP(d, 12)
	EXTR_U32(d, flags)
	EXTR_SKIP(d, 4)
	EXTR_U8(d, str_len)
	Assert(d.getConsumed() == lengthof(data));

	char *template_name = new char[str_len]{};
	FileRead(f, template_name, str_len);
	template_name[str_len - 1] = '\0';

	LIGHT_SPRITE* const l = LightSpriteCreate(template_name);
	// if this fails, then we will ignore the light.
	// ATE: Don't add ANY lights of mapscreen util is on
	if (l != NULL && guiCurrentScreen != MAPUTILITY_SCREEN)
	{
		// power only valid lights
		if (gfEditMode ||
				(!gfCaves && (
					flags & light_time ||
					!(flags & (LIGHT_PRIMETIME | LIGHT_NIGHTTIME))
				)))
		{
			LightSpritePower(l, TRUE);
		}
		LightSpritePosition(l, x, y);
		if (flags & LIGHT_PRIMETIME)
		{
			l->uiFlags |= LIGHT_PRIMETIME;
		}
		else if (flags & LIGHT_NIGHTTIME)
		{
			l->uiFlags |= LIGHT_NIGHTTIME;
		}
	}
	delete[] template_name;
}


void InjectLightSpriteIntoFile(HWFILE const file, LIGHT_SPRITE const* const l)
{
	BYTE data[24];

	DataWriter d{data};
	INJ_I16(d, l->iX)
	INJ_I16(d, l->iY)
	INJ_SKIP(d, 12)
	INJ_U32(d, l->uiFlags)
	INJ_SKIP(d, 4)
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(file, data, sizeof(data));

	const char* const light_name = LightSpriteGetTypeName(l);
	const UINT8       str_len    = static_cast<UINT8>(strlen(light_name) + 1);
	FileWrite(file, &str_len,   sizeof(str_len));
	FileWrite(file, light_name, str_len);
}
