#pragma once

#include <cstdint>
#include <string_theory/string>

#define FLORIST_BUTTON_TEXT_FONT		FONT14ARIAL
#define FLORIST_BUTTON_TEXT_UP_COLOR		2
#define FLORIST_BUTTON_TEXT_DOWN_COLOR		2
#define FLORIST_BUTTON_TEXT_SHADOW_COLOR	128


void EnterFlorist(void);
void ExitFlorist(void);
void RenderFlorist(void);

void RemoveFloristDefaults(void);
void DisplayFloristDefaults(void);
void InitFloristDefaults(void);
ST::string GetFloristGalleryText(uint32_t row, uint32_t col);
ST::string GetFloristCardString(uint32_t index);
