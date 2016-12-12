#pragma once

// SDL settings for various platforms

/**
 * Flag telling that SD_UpdateRect() is very expensive.
 *
 * On some systems SDL_UpdateRect() is a blocking call which can
 * take a lot of time to execute.
 *
 * For example, on Mac OS X SDL_UpdateRect calls CGContextFlush() which is tied
 * to the screen refresh rate (60MHz).  So, every call to SDL_UpdateRect() can
 * take up to 15 ms.  See:
 *    Technical Note TN2133, Coalesced Updates
 *    http://developer.apple.com/library/mac/#technotes/tn2133/_index.html
 *
 */
#ifdef __APPLE__
  #define EXPENSIVE_SDL_UPDATE_RECT    (1)
#else
  #define EXPENSIVE_SDL_UPDATE_RECT    (0)
#endif
