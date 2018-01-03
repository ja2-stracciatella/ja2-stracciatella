#pragma once

#include "GamePolicy.h"

#include "rapidjson/document.h"

class DefaultGamePolicy : public GamePolicy
{
public:
	DefaultGamePolicy(rapidjson::Document *json);

	/** Check if a hotkey is enabled. */
	virtual bool isHotkeyEnabled(UIMode mode, HotkeyModifier modifier, uint32_t key) const;

private:
	bool extra_hotkeys;
};
