#pragma once

#include "Types.h"

enum class TacticalTouchUIMode {
	ConfirmMove,
	ConfirmAction,
	ConfirmShoot,
};

void ShowTacticalTouchUI(TacticalTouchUIMode mode);

void HideTacticalTouchUI();

bool IsPointerOnTacticalTouchUI();

void RegisterViewPortPointerPosition(INT16 x, INT16 y);
