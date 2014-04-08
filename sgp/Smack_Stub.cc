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

  printf ("Smackerinfo -- Width: %lu Height: %lu Frames: %lu Framecount: %lu Frames/Second: %f Scale: %d \n", width, height, framecount, frame, usf/1000, scale);

  flickhandle->Frames=framecount;
  flickhandle->FrameNum=frame;
  flickhandle->Height=height;

  smk_enable_all(flickhandle->Smacker,SMK_VIDEO_TRACK|SMK_AUDIO_TRACK_0);
  //smk_enable_video (flickhandle->Smacker, SMK_VIDEO_TRACK);
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

void dump_bmp(unsigned char *pal, unsigned char *image_data, unsigned int w, unsigned int h, unsigned int framenum)
{
        int             i;
        FILE           *fp;
        char            filename  [128];
        unsigned int    temp;
        sprintf(filename, "/tmp/out_%04u.bmp", framenum);
        fp = fopen(filename, "wb");
        fwrite("BM", 2, 1, fp);
        temp = 1078 + (w * h);
        fwrite(&temp, 4, 1, fp);
        temp = 0;
        fwrite(&temp, 4, 1, fp);
        temp = 1078;
        fwrite(&temp, 4, 1, fp);
        temp = 40;
        fwrite(&temp, 4, 1, fp);
        fwrite(&w, 4, 1, fp);
        fwrite(&h, 4, 1, fp);
        temp = 1;
        fwrite(&temp, 2, 1, fp);
        temp = 8;
        fwrite(&temp, 4, 1, fp);
        temp = 0;
        fwrite(&temp, 2, 1, fp);
        temp = w * h;
        fwrite(&temp, 4, 1, fp);
        temp = 0;
        fwrite(&temp, 4, 1, fp);
        fwrite(&temp, 4, 1, fp);
        temp = 256;
        fwrite(&temp, 4, 1, fp);
        temp = 256;
        fwrite(&temp, 4, 1, fp);
        temp = 0;
        for (i = 0; i < 256; i++)
        {
                fwrite(&pal[(i * 3) + 2], 1, 1, fp);
                fwrite(&pal[(i * 3) + 1], 1, 1, fp);
                fwrite(&pal[(i * 3)], 1, 1, fp);
                fwrite(&temp, 1, 1, fp);
        }

        for (i = h - 1; i >= 0; i--)
        {
                fwrite(&image_data[i * w], w, 1, fp);
        }

        fclose(fp);
}

void SmackToBuffer(Smack* Smk, UINT32 Left, UINT32 Top, UINT32 Pitch, UINT32 DestHeight, void* Buf, UINT32 Flags)
{
  unsigned char* smackframe, *p;
  unsigned char* smackpal;
  UINT16 i,j,pixel,*buf;
  UINT8 *color;

  smackframe = smk_get_video(Smk->Smacker);
  smackpal = smk_get_palette (Smk->Smacker);
  // dump_bmp (smackpal, smackframe, 640, 480, Smk->FrameNum);
  buf=(UINT16*)Buf;
  p=smackframe;
  // for now hardcoded without taking sdl into account. 
  // need to find a way to blit it later
  for (i =0; i < 480; i++) {
    for (j = 0; j <640; j++) {
      // get rbg offset of palette
      color = &smackpal[p[0]*3] ;
      // convert from rbg to rbg565 0=red 1=green 2=blue
      pixel = (color[0]>>3)<<11 | (color[1]>>2)<<5 | color[2]>>3;
      buf[j+i*Pitch/2]=pixel;
      p++;
    }
  }
}


// not needed
/*
SDL_Surface* SmackBufferOpen(UINT32 BlitType, UINT32 Width, UINT32 Height, UINT32 ZoomW, UINT32 ZoomH)
{
  SDL_Surface* buffer;
  buffer = SDL_CreateRGBSurface(BlitType, Width, Height, 16, 0, 0, 0, 0);
  if ( ! buffer ) {
    fprintf(stderr, "CreateRGBSurface failed: %s       ", SDL_GetError());
    exit (1);
  }
  return buffer;
}
*/

/*
void SmackBufferClose(SmackBuf* SBuf)
{
  free (SBuf);
}
*/


UINT32 SmackUseMMX(UINT32 Flag)
{
	return 0;
}
