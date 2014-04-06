#include <strings.h>
#include "Smack_Stub.h"
extern "C" {
#include "smacker/smacker.h"

}

Smack* SmackOpen(const char* Name, UINT32 Flags, UINT32 ExtraBuf)
{
  Smack* flickhandle;
  unsigned long frame;
  unsigned long framecount;
  unsigned long width;
  unsigned long height;
  double usf;
  char status;
  unsigned char scale;
  union smk_read_t fp;

  fp.file = (FILE*)Name;
  flickhandle = (Smack*)malloc (sizeof (Smack)); 

  // open file with given filehandle
  flickhandle->Smacker = smk_open_generic(1, fp, 0, SMK_MODE_MEMORY);
  if ( ! flickhandle->Smacker ) return NULL;
  
  status = smk_info_video (flickhandle->Smacker, &width, &height, &scale);
  status = smk_info_all   (flickhandle->Smacker, &frame, &framecount, &usf);

  printf ("Smackerinfo -- Width: %lu Height: %lu Frames: %lu Framecount: %lu Frames/Second: %f Scale: %s \n", width, height, framecount, frame, usf, &scale);

  flickhandle->Frames=framecount;
  flickhandle->FrameNum=frame;
  flickhandle->Height=height;

  //smk_enable_all(flickhandle->Smacker,SMK_VIDEO_TRACK);
  smk_enable_video (flickhandle->Smacker, SMK_VIDEO_TRACK);
  smk_first(flickhandle->Smacker);
  return flickhandle;

}


UINT32 SmackDoFrame(Smack* Smk)
{
  
  return 0;
}


void SmackNextFrame(Smack* Smk)
{
  unsigned long int cur_frame; 
  smk_next(Smk->Smacker);
  smk_info_all(Smk->Smacker, &cur_frame, NULL, NULL);
  //printf ("Frame: %d cur_frame: %lu\n", Smk->FrameNum, cur_frame);
  Smk->FrameNum = cur_frame;
}


UINT32 SmackWait(Smack* Smk)
{
	return 0;
}


void SmackClose(Smack* Smk)
{
  smk_close (Smk->Smacker);
}


void SmackToBuffer(Smack* Smk, UINT32 Left, UINT32 Top, UINT32 Pitch, UINT32 DestHeight,  void* Buf, UINT32 Flags)
{
  unsigned char* frame;
  unsigned char* pal;
  int i,*a;
  a = (int*)Buf;
  frame = smk_get_video(Smk->Smacker);
  pal = smk_get_palette (Smk->Smacker);
  // need to find out howto copy data correctly to buffer .. for now at least
  // sometihing can be seen (correctly decoded smack)
  for (i = 0; i<640*480;i+=1){
    a[i]=frame[i];
  }
}


SmackBuf* SmackBufferOpen(UINT32 BlitType, UINT32 Width, UINT32 Height, UINT32 ZoomW, UINT32 ZoomH)
{
  //att just allocate some space for testing
  SmackBuf* buffer = (SmackBuf*) malloc (Width * Height * 3 * sizeof(UINT32));
  return buffer;
}


void SmackBufferClose(SmackBuf* SBuf)
{
  free (SBuf);
}


UINT32 SmackUseMMX(UINT32 Flag)
{
	return 0;
}
