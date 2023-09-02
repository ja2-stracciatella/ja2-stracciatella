#ifndef SLIDER_H
#define SLIDER_H

#include "Types.h"
struct SLIDER;

//defines for the different styles of sliders
enum
{
	SLIDER_DEFAULT_STYLE,
	SLIDER_VERTICAL_STEEL,
	NUM_SLIDER_STYLES
};

typedef void (*SLIDER_CHANGE_CALLBACK)(INT32);

SLIDER* AddSlider(UINT8 ubStyle, UINT16 usCursor, UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT16 usNumberOfIncrements, INT8 sPriority, SLIDER_CHANGE_CALLBACK SliderChangeCallback);
void RemoveSliderBar(SLIDER* s);

void SetSliderValue(SLIDER* s, UINT32 uiNewValue);

void InitSlider(void);
void ShutDownSlider(void);

void RenderAllSliderBars(void);

#endif
