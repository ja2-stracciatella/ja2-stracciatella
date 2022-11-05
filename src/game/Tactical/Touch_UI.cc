#include "Touch_UI.h"
#include "Animation_Control.h"
#include "Button_System.h"
#include "Cursor_Control.h"
#include "Font_Control.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Interface_Cursors.h"
#include "Interface_Items.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "UI_Cursors.h"
#include "RenderWorld.h"
#include "Weapons.h"
#include "Items.h"
#include "Turn_Based_Input.h"
#include "Interactive_Tiles.h"
#include "Message.h"
#include "Text.h"

#include <memory>

#define TACTICAL_TOUCH_UI_BUTTON_SIZE 32

void TacticalTouchUIConfirmCallback(GUI_BUTTON*, UINT32);
void TacticalTouchUICancelCallback(GUI_BUTTON*, UINT32);

// For modifying attacks
void TacticalTouchUIIncreaseAimCallback(GUI_BUTTON*, UINT32);
void TacticalTouchUIBurstCallback(GUI_BUTTON*, UINT32);

// For modifying move
void TacticalTouchUIItemCallback(GUI_BUTTON*, UINT32);
void TacticalTouchUIRunCallback(GUI_BUTTON*, UINT32);

class TacticalTouchUI {
	public:
		TacticalTouchUI(TacticalTouchUIMode mode);
		~TacticalTouchUI();
		void changeMode(TacticalTouchUIMode mode);
		void changeCursor(UINT16 const crsr);
		bool isPointerOver() const;
	private:
		TacticalTouchUIMode mode;
		std::vector<GUIButtonRef> buttons;

		void rebuiltButtons();
};

TacticalTouchUI::TacticalTouchUI(TacticalTouchUIMode mode) {
	this->mode = mode;
	rebuiltButtons();
}

TacticalTouchUI::~TacticalTouchUI() {
	for (auto& button : buttons) {
		RemoveButton(button);
	}
	SetRenderFlags(RENDER_FLAG_FULL);
}

bool TacticalTouchUI::isPointerOver() const {
	for (auto const& button : buttons) {
		if (button->Area.uiFlags & MSYS_MOUSE_IN_AREA) {
			return true;
		}
	}
	return false;
}

void TacticalTouchUI::changeMode(TacticalTouchUIMode mode) {
	if (this->mode != mode) {
		this->mode = mode;
		// If we change the mode rebuild buttons
		rebuiltButtons();
	}
}

void TacticalTouchUI::changeCursor(UINT16 const crsr) {
	for (auto& button : buttons) {
		button->SetCursor(crsr);
	}
}

void TacticalTouchUI::rebuiltButtons() {
	for (auto& button : buttons) {
		RemoveButton(button);
	}
	buttons.clear();
	auto const x = gViewportRegion.RegionBottomRightX - TACTICAL_TOUCH_UI_BUTTON_SIZE;
	auto y = gViewportRegion.RegionBottomRightY - TACTICAL_TOUCH_UI_BUTTON_SIZE;

	buttons.push_back(CreateTextButton("Do", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIConfirmCallback));
	y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
	switch (mode) {
		case TacticalTouchUIMode::ConfirmShoot: {
			buttons.push_back(CreateTextButton("Aim", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIIncreaseAimCallback));
			y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
			buttons.push_back(CreateTextButton("Mode", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIBurstCallback));
			y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
			break;
		}
		case TacticalTouchUIMode::ConfirmAction: {
			buttons.push_back(CreateTextButton("Run", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIRunCallback));
			y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
			break;
		}
		case TacticalTouchUIMode::ConfirmMove: {
			buttons.push_back(CreateTextButton("Item", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIItemCallback));
			y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
			buttons.push_back(CreateTextButton("Run", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUIRunCallback));
			y -= TACTICAL_TOUCH_UI_BUTTON_SIZE;
			break;
		}
		default:
			break;
	}
	buttons.push_back(CreateTextButton("X", FONT12POINT1, FONT_BLACK, FONT_BLACK, x, y, TACTICAL_TOUCH_UI_BUTTON_SIZE, TACTICAL_TOUCH_UI_BUTTON_SIZE, MSYS_PRIORITY_HIGH, TacticalTouchUICancelCallback));
}

std::unique_ptr<TacticalTouchUI> gTacticalTouchUI = nullptr;
SGPPoint gLastViewPortCursorPosition = SGPPoint{};

void ShowTacticalTouchUI(TacticalTouchUIMode mode) {
	// Never show touch ui on non-touch devices
	if (!gfIsUsingTouch) return;

	if (gTacticalTouchUI) {
		gTacticalTouchUI->changeMode(mode);
		gTacticalTouchUI->changeCursor(gUICursors[guiCurrentUICursor].usFreeCursorName);
	} else {
		gTacticalTouchUI.reset(new TacticalTouchUI(mode));
		gTacticalTouchUI->changeCursor(gUICursors[guiCurrentUICursor].usFreeCursorName);
	}
	SetManualCursorPos(gLastViewPortCursorPosition);
}

void HideTacticalTouchUI() {
	if (gTacticalTouchUI) {
		gTacticalTouchUI.reset();
		ClearManualCursorPos();
	}
}

bool IsPointerOnTacticalTouchUI() {
	if (!gTacticalTouchUI) {
		return false;
	}
	return gTacticalTouchUI->isPointerOver();
}

void RegisterViewPortPointerPosition(INT16 x, INT16 y) {
	gLastViewPortCursorPosition.iX = x;
	gLastViewPortCursorPosition.iY = y;
}

void TacticalTouchUIConfirmCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		auto const& selected = GetSelectedMan();

		if (guiCurrentCursorGridNo == NOWHERE || selected == NULL) return;

		if ( gpItemPointer != NULL ) {
			if ( HandleItemPointerClick( guiCurrentCursorGridNo ) )
			{
				// getout of mode
				EndItemPointer( );

				guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
			}
		} else {
			switch( gCurrentUIMode )
			{
				case CONFIRM_MOVE_MODE: {
					auto returnCode = HandleMoveModeInteractiveClick(guiCurrentCursorGridNo);
					if (gTacticalStatus.uiFlags & INCOMBAT) {
						if ( returnCode == -2 )
						{
							if ( SelectedMercCanAffordMove(  )  )
							{
								guiPendingOverrideEvent = C_MOVE_MERC;
							}
						}
						else if ( returnCode == 0 )
						{
							if ( gsCurrentActionPoints == 0 )
							{
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
							}
							else if ( SelectedMercCanAffordMove(  )  )
							{
								const BOOLEAN fResult = UIOKMoveDestination(selected, guiCurrentCursorGridNo);
								if (fResult == 1)
								{
									// ATE: CHECK IF WE CAN GET TO POSITION
									// Check if we are not in combat
									if ( gsCurrentActionPoints == 0 )
									{
										ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
									}
									else
									{
										guiPendingOverrideEvent = C_MOVE_MERC;
									}
								}
								else if (fResult == 2)
								{
									ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NOBODY_USING_REMOTE_STR]);
								}
							}
						}
					} else {
						if (returnCode == -2) {
							BeginDisplayTimedCursor(GetInteractiveTileCursor( guiCurrentUICursor, TRUE ), 300);

							if (selected->usAnimState != RUNNING)
							{
								guiPendingOverrideEvent = C_MOVE_MERC;
							}
							else if (GetCurInteractiveTile() != NULL)
							{
								selected->fUIMovementFast = TRUE;
								guiPendingOverrideEvent = C_MOVE_MERC;
							}
						} else if (returnCode == 0) {
							if (UIOKMoveDestination(selected, guiCurrentCursorGridNo) == 1)
							{
								if ( gsCurrentActionPoints != 0 )
								{
									guiPendingOverrideEvent = C_MOVE_MERC;
								}
							}
						}
					}
					break;
				}
				case CONFIRM_ACTION_MODE:
					guiPendingOverrideEvent = CA_MERC_SHOOT;
					break;
				case LOOKCURSOR_MODE:
					guiPendingOverrideEvent = LC_LOOK;
					break;
				case TALKCURSOR_MODE:
					if ( HandleTalkInit( ) )
					{
						guiPendingOverrideEvent = TA_TALKINGMENU;
					}
					break;
				case HANDCURSOR_MODE:
					HandleHandCursorClick( guiCurrentCursorGridNo, &guiPendingOverrideEvent );
				default:
					break;
			}
		}
	}
}

void TacticalTouchUICancelCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		ResetCurrentCursorTarget();
		guiPendingOverrideEvent = I_CHANGE_TO_IDLE;
	}
}

void TacticalTouchUIIncreaseAimCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		auto const& selected = GetSelectedMan();

		if (guiCurrentCursorGridNo == NOWHERE || selected == NULL) return;

		HandleRightClickAdjustCursor(selected, guiCurrentCursorGridNo);
	}
}

void TacticalTouchUIBurstCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		auto const& selected = GetSelectedMan();

		if (guiCurrentCursorGridNo == NOWHERE || selected == NULL) return;

		ChangeWeaponMode(selected);
	}
}

void TacticalTouchUIItemCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		auto const& sel = GetSelectedMan();

		if (sel == NULL || gpItemPointer != NULL) return;

		guiPendingOverrideEvent = A_ON_TERRAIN;
	}
}

void TacticalTouchUIRunCallback(GUI_BUTTON*, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) {
		auto const& sel = GetSelectedMan();

		if (guiCurrentCursorGridNo == NOWHERE || sel == NULL) return;
		if (MercInWater(sel) || (sel->uiStatusFlags & SOLDIER_ROBOT)) return;

		// Change selected merc to run
		if (sel->usUIMovementMode != WALKING && sel->usUIMovementMode != RUNNING)
		{
			UIHandleSoldierStanceChange(sel, ANIM_STAND);
		}
		else
		{
			sel->usUIMovementMode = RUNNING;
			gfPlotNewMovement     = TRUE;
		}
		sel->fUIMovementFast = 1;
	}
}
