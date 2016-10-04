#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "slog/slog.h"


#define DEBUG_PRINT_FPS                         (0)             /**< Flag telling to print FPS (Frames per second) counter. */
#define DEBUG_PRINT_GAME_CYCLE_TIME             (0)             /**< Flag telling to print how much time every game cycle takes. */

#define Assert(a)       (a) ? (void)0 : SLOGE(DEBUG_TAG_ASSERTS, "Assertion failed in %s, line %d", __FILE__, __LINE__)
#define AssertMsg(a, b) (a) ? (void)0 : SLOGE(DEBUG_TAG_ASSERTS, "Assertion failed in %s, line %d:\n%s", __FILE__, __LINE__, b)

const char* String(const char* fmt, ...);

#endif
