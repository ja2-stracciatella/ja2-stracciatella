#include <stdint.h>

#include "Debug.h"

float EaseInCubic(uint32_t uiStartTime, uint32_t uiEndTime, uint32_t uiCurrentTime) {
	Assert(uiEndTime >= uiStartTime);
	Assert(uiCurrentTime <= uiEndTime);

	float fProgress = (float)(uiCurrentTime - uiStartTime) / (float)(uiEndTime - uiStartTime);

	if (fProgress < 0.5) {
		return 4.0f * fProgress * fProgress * fProgress;
	} else {
		return (fProgress - 1.0f) * (2.0f * fProgress - 2.0f) * (2.0f * fProgress - 2.0f) + 1.0f;
	}
}
