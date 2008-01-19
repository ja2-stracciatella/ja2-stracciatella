#ifndef __WCHECK_
#define __WCHECK_

#if 0 // XXX TODO put under some flag
#	define FAIL abort();
#else
#	define FAIL FIXME
#endif

#define CHECKF(exp) do { if (!(exp)) { FAIL return FALSE; } } while (0)
#define CHECKV(exp) do { if (!(exp)) { FAIL return;       } } while (0)
#define CHECKN(exp) do { if (!(exp)) { FAIL return NULL;  } } while (0)

#endif
