#ifndef RADMALI
#define RADMALI


#ifdef __RAD32__
  #include "malloc.h"

  #define radmalrad malloc
  #define radfrrad free
#else
  #ifdef __RADWIN__
    #ifdef RADStatus
      #define radmalrad(num) GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE,num)
      #define radfrrad GlobalFreePtr
    #else
      #define radmalloc(num) GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE,num)
      #define radfree GlobalFreePtr
    #endif
  #else
    #include "malloc.h"
    #define radmalrad(num) _fmalloc((int)num)
    #define radfrrad _ffree
  #endif
#endif

/*

#ifndef radmalloc

RADEXPFUNC void PTR4* RADEXPLINK radmalloc(u32 numbytes)
{
  u8 PTR4* temp;
  u8 i;

  if ((numbytes==0) || (numbytes==0xffffffff))
    return(0);
  
  temp=(u8 PTR4*)radmalrad(numbytes+16);
  if (temp==0)
    return(0);

  i=(u8)(16-((u32)temp&15));
  temp+=i;
  temp[-1]=i;

  return(temp);
}

RADEXPFUNC void RADEXPLINK radfree(void PTR4* ptr)
{
  if (ptr) 
    radfrrad( ((u8 PTR4*)ptr)-((u8 PTR4*)ptr)[-1] );
}
  
#endif
*/
#endif
