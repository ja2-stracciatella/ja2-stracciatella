#ifndef __WCHECK_
#define __WCHECK_

#if 0 // XXX TODO put under some flag
#	define FAIL() abort()
#else
#	define FAIL() FIXME
#endif

#define CHECKF(exp)  if (!(exp)) { FAIL(); return FALSE; }
#define CHECKV(exp)  if (!(exp)) { FAIL(); return; }
#define CHECKN(exp)  if (!(exp)) { FAIL(); return NULL; }
#define CHECKBI(exp) if (!(exp)) { FAIL(); return -1; }

#endif
