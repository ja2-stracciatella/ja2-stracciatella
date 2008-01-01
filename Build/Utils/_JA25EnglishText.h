#ifndef JA25ENGLISHTEXT_H
#define JA25ENGLISHTEXT_H

#include <wchar.h>

enum
{
	TCTL_MSG__RANGE_TO_TARGET,
	TCTL_MSG__RANGE_TO_TARGET_AND_GUN_RANGE,
	TCTL_MSG__DISPLAY_COVER,
	TCTL_MSG__LOS,
	TCTL_MSG__IRON_MAN_CANT_SAVE_NOW,
	TCTL_MSG__CANNOT_SAVE_DURING_COMBAT,
};
extern const wchar_t* const zNewTacticalMessages[];

extern const wchar_t str_iron_man_mode_warning[];

#endif
