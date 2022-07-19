#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "Logger.h"


#define DEBUG_PRINT_FPS                         (0)             /**< Flag telling to print FPS (Frames per second) counter. */
#define DEBUG_PRINT_GAME_CYCLE_TIME             (0)             /**< Flag telling to print how much time every game cycle takes. */

// Don't ever define __COVERITY__. It is needed to avoid false positives during
// a coverity scan from patterns like
//
//   Assert(p);
//   p->something = 0;
//
// https://community.synopsys.com/s/question/0D534000046YuzbCAC/suppressing-assertsideeffect-for-functions-that-allow-for-sideeffects

#ifndef __COVERITY__
#define Assert(a)       (a) ? (void)0 : SLOGA("Assertion failed in {}, line {}", __FILE__, __LINE__)
#define AssertMsg(a, b) (a) ? (void)0 : SLOGA("Assertion failed in {}, line {}:\n{}", __FILE__, __LINE__, b)
#else
extern void __coverity_panic__(void);
#define Assert(condition)         (condition) ? (void)0 : __coverity_panic__()
#define AssertMsg(condition, msg) (condition) ? (void)0 : __coverity_panic__()
#endif

#endif
