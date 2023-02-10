#pragma once

#include "GamePolicy.h"
#include "Json.h"

class DefaultGamePolicy : public GamePolicy
{
public:
	DefaultGamePolicy(const JsonValue& json);

	/** Check if a hotkey is enabled. */
	virtual bool isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const;

private:
	bool extra_hotkeys;
};
