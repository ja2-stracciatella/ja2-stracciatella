#ifndef __WCHECK_
#define __WCHECK_

#if 1 // XXX TODO put under some flag
#	define ABORT() abort()
#else
#	define ABORT()
#endif

#define CHECKF(exp)  if (!(exp)) { ABORT(); return FALSE; }
#define CHECKV(exp)  if (!(exp)) { ABORT(); return; }
#define CHECKN(exp)  if (!(exp)) { ABORT(); return NULL; }
#define CHECKBI(exp) if (!(exp)) { ABORT(); return -1; }

#endif
