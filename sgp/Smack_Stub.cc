#include <strings.h>
#include "Smack_Stub.h"
#include "Sound_Control.h"
#include "SoundMan.h"
extern "C" {
#include "smacker/smacker.h"

}


UINT32 SmackGetAudio (Smack* Smk, char* audiobuffer) {

  smk smkobj=Smk->Smacker;

  unsigned long audiolength, smacklength, bufmax = 0;
  unsigned char* smackaudio;
  const unsigned char track=0;

  ///Smk->smkaudiotrack.freq
  ///Smk->smkaudiotrack.channels
  ///Smk->smkaudiotrack.format=AUDIO_S16;
  

  if (! audiobuffer ) return 0;
  
  audiolength=0;
  // disable video
  smk_enable_video(smkobj, DISABLE);
  smk_enable_audio(smkobj,  track, ENABLE);
  smk_first(smkobj);
  do {
    smackaudio = smk_get_audio (smkobj, track);
    smacklength = smk_get_audio_size (smkobj, track);
    //memcpy (&audiobuffer+audiolength, smackaudio, smacklength);
    audiolength += smacklength;
  } while (  smk_next(smkobj) == SMK_MORE  );

  printf ("Frame: %d of %d --- smacklength %lu copied, length now %lu max: %lu\n", Smk->FrameNum, Smk->Frames, smacklength, audiolength, bufmax);
  printf ("read %lu bytes info audiobuffer\n", audiolength);
  
  // disable audio
  smk_enable_audio (smkobj,  track, DISABLE);
  return audiolength;
}


Smack* SmackOpen(const char* FileHandle, UINT32 Flags, UINT32 ExtraBuf)
{
  Smack* flickinfo;
  unsigned long frame;
  unsigned long framecount;
  unsigned long width;
  unsigned long height;

  double usf;
  char status;
  unsigned char scale;
  union smk_read_t fp;
  /* arrays for audio track metadata */
  unsigned char   a_trackmask, a_channels[7], a_depth[7];
  unsigned long   a_rate[7];
  const unsigned char track = 0;
  char* audiobuffer;
  UINT32 audiolength, audiosamples;

  fp.file = (FILE*)FileHandle;
  flickinfo = (Smack*)malloc (sizeof (Smack)); 

  // open file with given filehandle
  flickinfo->Smacker = smk_open_generic(1, fp, 0, SMK_MODE_DISK);
  if ( ! flickinfo->Smacker ) return NULL;
  
  status = smk_info_video (flickinfo->Smacker, &width, &height, &scale);
  status = smk_info_all   (flickinfo->Smacker, &frame, &framecount, &usf);

  status = smk_info_audio(flickinfo->Smacker, &a_trackmask, a_channels, a_depth, a_rate);
  
  flickinfo->smkaudiotrack.freq=a_rate[track];
  flickinfo->smkaudiotrack.channels=a_channels[track];
  flickinfo->smkaudiotrack.format=AUDIO_S16;
  flickinfo->smkaudiotrack.samples = 2048;  /* 1024.. Good low-latency value for callback */
  flickinfo->smkaudiotrack.userdata = NULL;
  flickinfo->Frames=framecount;
  flickinfo->FrameNum=frame;
  flickinfo->Height=height;

  printf ("Smackerinfo -- Width: %lu Height: %lu Frames: %lu Framecount: %lu Frames/Second: %f Scale: %d, \nAudio Freq: %d Channels %d\n ", width, height, framecount, frame, usf/1000, scale, 
          flickinfo->smkaudiotrack.freq, 
          flickinfo->smkaudiotrack.channels
          );

  audiosamples = ( ((flickinfo->Frames) / usf/1000) *  flickinfo->smkaudiotrack.freq * 16 *  flickinfo->smkaudiotrack.channels);
  audiobuffer = (char*) malloc( audiosamples );
  if ( ! audiobuffer ) return NULL;
  audiolength = SmackGetAudio (flickinfo, audiobuffer);
  //if ( audiolength > 0 ) SoundPlayFromBuffer( audiobuffer, audiolength, MAXVOLUME/4, 0, 1, NULL, NULL);

  smk_enable_video (flickinfo->Smacker, ENABLE);
  smk_first(flickinfo->Smacker);
  return flickinfo;
}


UINT32 SmackDoFrame(Smack* Smk)
{
  // play sound sample
  char* soundsample;
  UINT32 vol;

  soundsample = (char*) smk_get_audio(Smk->Smacker,0);

  if( soundsample != NULL) {
      vol = CalculateSoundEffectsVolume(90);
      return 0;// PlaySound(soundsample, vol, 0, 0, NULL, NULL);
    }
  return SOUND_ERROR;
}


CHAR8 SmackNextFrame(Smack* Smk)
{
  unsigned long int cur_frame; 
  CHAR8 smkstatus;
  smkstatus = smk_next(Smk->Smacker);
  smk_info_all(Smk->Smacker, &cur_frame, NULL, NULL);
  //printf ("Frame: %d cur_frame: %lu\n", Smk->FrameNum, cur_frame);
  Smk->FrameNum = cur_frame;
  return smkstatus;
}


UINT32 SmackWait(Smack* Smk)
{
	return 0;
}


void SmackClose(Smack* Smk)
{
  smk_close (Smk->Smacker);
}

// stolen from driver.c (which was part of libsmacker-1.0)
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

  //printf ("got audiobuffer length: %lu\n", audiolength);
  // dump_bmp (smackpal, smackframe, 640, 480, Smk->FrameNum);
  buf=(UINT16*)Buf;
  p=smackframe;
  // for now hardcoded without taking sdl into account. 
  // need to find a way to blit it later
  for (i =0; i < DestHeight ; i++) {
    for (j = 0; j <640; j++) {
      // get rbg offset of palette
      color = &smackpal[p[0]*3] ;
      // convert from rbg to rbg565 0=red 1=green 2=blue
      if (Flags == SMACKBUFFER565) {
        pixel = (color[0]>>3)<<11 | (color[1]>>2)<<5 | color[2]>>3;
      }
      else {
        pixel = (color[0]>>3)<<10 | (color[1]>>2)<<5 | color[2]>>3;
      }
      
      buf[(j+Top)+(i+Left)*Pitch/2]=pixel;
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
