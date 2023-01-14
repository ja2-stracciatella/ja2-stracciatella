#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "Logger.h"


#define DEBUG_PRINT_FPS                         (0)             /**< Flag telling to print FPS (Frames per second) counter. */
#define DEBUG_PRINT_GAME_CYCLE_TIME             (0)             /**< Flag telling to print how much time every game cycle takes. */

namespace SGP { inline ST::string const null{}; }
// NOTE: on switching to c++20 investigate if this can be simplified in favour of std::source_location
#define Assert(a) VerboseAssert(static_cast<bool>(a), SGP::null, __FILE__, __LINE__)
#define AssertMsg(a, b) VerboseAssert(static_cast<bool>(a), b, __FILE__, __LINE__)

inline void VerboseAssert(bool condition, ST::string const& msg, char const * file, uint16_t line)
{
	if (!condition) SLOGA("Assertion failed in {}, line {}:\n{}", file, line, msg);
}

#endif
