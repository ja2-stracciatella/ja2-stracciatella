#ifndef RADMALWI
#define RADMALWI

#ifdef __RADWINEXT__


u32 GetLimit(u16 sel);
#pragma aux GetLimit = "lsl eax,ax" parm [ax];

RADEXPFUNC void PTR4* RADEXPLINK radmalloc(u32 numbytes)
{
  u32 temp;
  HGLOBAL handle;
  if ((numbytes==0) || (numbytes==0xffffffff))
    return(0);

  handle=GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE,numbytes+4);
  if (handle) {
    GlobalFix(handle);
    temp=(u32)GlobalLock(handle);
    temp=((u32)_16To32(temp));
    ((u32 PTR4*)temp)[0]=handle;
    return(((u32 PTR4*)temp)+1);
  }
  return(0);
}

RADEXPFUNC void RADEXPLINK radfree(void PTR4* ptr)
{
  HGLOBAL h=((u32 PTR4*)ptr)[-1];
  GlobalUnfix(h);
  GlobalUnlock(h);
  GlobalFree(h);
}

#else

#include "radmal.i"

#endif

#endif
