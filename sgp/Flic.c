//**************************************************************************
//
// Filename :	flic.c
//
//	Purpose :	Display System for .flc and .fli Flic Files
//
//		This program is derived from the program decribed in the article "The
//		Flic File Format" on page 18 of the March 1993 edition of Dr. Dobb's
//		Journal. It was restructured from the top down to meet the
//		requirements of Sir-Tech. The high level routines were designed to
//		provided easy integration with existing applications. Except for a few
//		identifiers, the low level routines were not modified.
//
//		All of the code for these routines in contained in this file and the
//		file "flic.h".
//
// Modification history :
//
//		06-09-94:???						- Creation
//		???:David Smoth					- ???
//		???:Bret Rowdon for RIO/GORGE - added 640x480 SVGA support
//		05-11jun96:HJH						- added routines for bitmap use, and
//												  other utilities
//    08-May-97   ARM       Adapted for Win95 Standard Gaming Platform
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

//#include "stdafx.h"
#include <conio.h>
#include "flic.h"
#include "windows.h"
#include "types.h"
#include "wcheck.h"

//**************************************************************************
//
//				Variables
//
//**************************************************************************

//Flic flic;
char pcxbuf[(640*480)];
char FlicPal[768];

//extern char buffer[(640*480)];
//extern char buffer2[(640*480)];

//**************************************************************************
//
//				Function prototypes.
//
//**************************************************************************

static ErrCode flic_next_frame(Flic *flic, BOOL fDecode);

//**************************************************************************
//
//				Functions
//
//**************************************************************************

//**************************************************************************
//
//				Low level Functions
//
//**************************************************************************

//**************************************************************************
//
// screen_put_dot
//
//		Set one dot.
//
// Parameter List :
// Return Value :
// Modification history :
//
//**************************************************************************

static void screen_put_dot(FlicScreen *s, int x, int y, Pixel color)
{
	// First clip it.
	if (x < 0 || y < 0 || x >= s->width || y >= s->height)
		return;

	// Then set it.
	s->pixels[(unsigned)(s->height-y-1) * s->width + x] = color;
}

//**************************************************************************
//
// line_clip
//
//		Clip a horizontal line segment so that it fits on the screen.
//		Return FALSE if clipped out entirely.
//
// Parameter List :
// Return Value :
// Modification history :
//
//**************************************************************************

static Boolean line_clip(FlicScreen *s, int *px, int *py, int *pwidth)
{
	int x = *px;
	int y = *py;
	int width = *pwidth;
	int xend = x + width;

	if (y < 0 || y >= s->height || xend < 0 || x >= s->width)
		return FALSE;	/* Clipped off screen. */
	if (x < 0)
	{
		*pwidth = width = width + x;		/* and shortens width. */
		*px = 0;
	}
	if (xend > s->width)
	{
		*pwidth = width = width - (xend - s->width);
	}
	if (width < 0)
		return FALSE;
	return TRUE;
}

//**************************************************************************
//
// oldscreen_copy_seg
//
//		Copy pixels from memory into screen.
//
// Parameter List :
// Return Value :
// Modification history :
//
//**************************************************************************

static void oldscreen_copy_seg(FlicScreen *s, int x, int y, Pixel *pixels, int count)
{
	Pixel *pt;
	//int xend;
	int unclipped_x = x;
	int dx;

	// First let's do some clipping.
	if (!line_clip(s, &x, &y, &count))
		return;

	dx = x - unclipped_x;	// Clipping change in start position.
	if (dx != 0)
		pixels += dx;			// Advance over clipped pixels.

	// Calculate start screen address.
	//pt = s->pixels + (unsigned)y * (unsigned)s->width + (unsigned)x;
	pt = s->pixels + (unsigned)(s->height-y-1) * (unsigned)s->width + (unsigned)x;

	// Copy pixels to display.
	while (--count >= 0)
		*pt++ = *pixels++;
}

//**************************************************************************
//
// screen_repeat_one
//
//			Draw a horizontal line of a solid color
//
// Parameter List :
//
//		FlicScreen	*s		-> screen to write to
//		int			x		-> x position to write to
//		int			y		-> y position to write to
//		Pixel			color	-> colour to write
//		int			count	-> how many times to write
//
// Return Value :
//
// Modification history :
//
//			01jun96:HJH		-> reversed the screen up and down so that the
//									bitmaps wouldn't come out upside down (I put in
//									the "s->height -")
//
//**************************************************************************

static void screen_repeat_one(FlicScreen *s, int x, int y, Pixel color, int count)
{
	Pixel *pt;

	// First let's do some clipping.
	if (!line_clip(s, &x, &y, &count))
		return;

	// Calculate start screen address.
	pt = s->pixels + (unsigned)(s->height-y-1) * (unsigned)s->width + (unsigned)x;

	// Repeat pixel on display.
	while (--count >= 0)
		*pt++ = color;
}

//**************************************************************************
//
// screen_repeat_two
//
//		Repeat 2 pixels count times on screen.
//
// Parameter List :
// Return Value :
// Modification history :
//
//**************************************************************************

static void screen_repeat_two(FlicScreen *s, int x, int y, Pixels2 pixels2, int count)
{
	Pixels2 *pt;
	int is_odd;

	// First let's do some clipping.
	count <<= 1;		// Convert from word to pixel count.
	if (!line_clip(s, &x, &y, &count))
		return;
	is_odd = (count&1);	// Did it turn odd after clipping?  Ack!
	count >>= 1;			// Convert back to word count.

	// Calculate start screen address.
	pt = (Pixels2 *)(s->pixels + (unsigned)(s->height-y-1) * (unsigned)s->width + (unsigned)x);

	while (--count >= 0)	// Go set screen 2 pixels at a time.
		*pt++ = pixels2;

	if (is_odd)				// Deal with pixel at end of screen if needed.
	{
		Pixel *end = (Pixel *)pt;
		*end = pixels2.pixels[0];
	}
}

//**************************************************************************
//
// screen_put_colors
//
//		Set count colors in color map starting at start.  RGB values
//		go from 0 to 255.
//
// Parameter List :
//
//		FlicScreen	*s				-> screen data
//		int			start			-> start position
//		Colour			*colors		-> the colors, of course
//		int			count			-> how many
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void screen_put_colors(FlicScreen *s, int start, Colour *colors, int count)
{
	int end = start + count;
	int ix;

	for (ix = start; ix < end; ++ix)
	{
		FlicPal[ix*3]=colors->r>>2;
		FlicPal[ix*3+1]=colors->g>>2;
		FlicPal[ix*3+2]=colors->b>>2;
		++colors;
	}
}

//**************************************************************************
//
// screen_put_colors_64
//
//		Set count colors in color map starting at start.  RGB values
//		go from 0 to 64. */
//
// Parameter List :
//
//		FlicScreen	*s				-> screen data
//		int			start			-> start position
//		Colour			*colors		-> the colors, of course
//		int			count			-> how many
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void screen_put_colors_64(FlicScreen *s, int start, Colour *colors, int count)
{
	int end = start + count;
	int ix;

	for (ix = start; ix < end; ++ix)
	{
		FlicPal[ix*3]=colors->r;
		FlicPal[ix*3+1]=colors->g;
		FlicPal[ix*3+2]=colors->b;

		++colors;
	}
}

//**************************************************************************
//
// decode_byte_run
//
//		Byte-run-length decompression.
//
// Parameter List :
//
//		Uchar	*data		-> data to be decoded
//		Flic	*flic		-> flic data containing where to decode to
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_byte_run(Uchar  *data, Flic *flic)
{
	int x,y;
	int width = flic->head.width;
	int height = flic->head.height;
	int psize;	// was char -> HJH
	Char  *cpt = (Char *)data;
	int end;

	y = flic->yoff;
	end = flic->xoff + width;
	while (--height >= 0)
	{
		x = flic->xoff;
		cpt += 1;	/* skip over obsolete opcount byte */
		psize = 0;
		while ((x+=psize) < end)
		{
			psize = (char)(*cpt++);
			if (psize >= 0)
			{
				screen_repeat_one(&flic->screen, x, y, *cpt++, psize);
			}
			else
			{
				psize = -psize;
				oldscreen_copy_seg(&(flic->screen),(long) x,(long) y, (unsigned char *)cpt, (long)psize);
				cpt += psize;
			}
		}
		y++;
	}
}

//**************************************************************************
//
// decode_delta_fli
//
//		Fli style delta decompression.
//
// Parameter List :
//
//		Uchar	*data		-> data to be decoded
//		Flic	*flic		-> flic data containing where to decode to
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_delta_fli(Uchar  *data, Flic *flic)
{
	int xorg = flic->xoff;
	int yorg = flic->yoff;
	Short  *wpt = (Short  *)data;
	Uchar  *cpt = (Uchar  *)(wpt + 2);
	int x,y;
	Short lines;
	Uchar opcount;
	Char psize;

	y = yorg + *wpt++;
	lines = *wpt;
	while (--lines >= 0)
	{
		x = xorg;
		opcount = *cpt++;
		while (opcount > 0)
		{
			x += *cpt++;
			psize = *cpt++;
			if (psize < 0)
			{
				psize = -psize;
				screen_repeat_one(&flic->screen, x, y, *cpt++, psize);
				x += psize;
				opcount-=1;
			}
			else
			{
				oldscreen_copy_seg(&(flic->screen), (long)x, (long)y, (unsigned char *)cpt,(long) psize);
				cpt += psize;
				x += psize;
				opcount -= 1;
			}
		}
		y++;
	}
}

//**************************************************************************
//
// decode_delta_flc
//
//		Flc-style delta decompression.  The data is word oriented though
//		a lot of the control info (how to skip, how many words to
//		copy) are byte oriented still to save space.
//
// Parameter List :
//
//		Uchar	*data		-> data to be decoded
//		Flic	*flic		-> flic data containing where to decode to
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_delta_flc(Uchar  *data, Flic *flic)
{
	int xorg = flic->xoff;
	int yorg = flic->yoff;
	int width = flic->head.width;
	int x,y;
	Short lp_count;
	Short opcount;
	int psize;
	union {Short  *w; Uchar  *ub; Char  *b; Pixels2  *p2;} wpt;
	int lastx;


	lastx = xorg + width - 1;
	wpt.ub = data;
	lp_count = *wpt.w++;
	y = yorg;
	goto LPACK;

SKIPLINES:	/* Advance over some lines. */
	y -= opcount;

LPACK:		/* do next line */
	if ((opcount = *wpt.w++) >= 0)
		goto DO_SS2OPS;
	if( ((Ushort)opcount) & 0x4000) /* skip lines */
		goto SKIPLINES;
	screen_put_dot(&flic->screen,(Uchar)opcount,lastx,(Pixel)y); /* put dot at eol with low byte */
	if((opcount = *wpt.w++) == 0)
	{
		++y;
		if (--lp_count > 0)
			goto LPACK;
		goto OUTT;
	}
DO_SS2OPS:
	x = xorg;

PPACK:				/* do next packet */
	x += *wpt.ub++;
	psize = *wpt.b++;
	if ((psize += psize) >= 0)
	{
		oldscreen_copy_seg(&(flic->screen), (long)x, (long)y, (unsigned char *)wpt.ub,(long) psize);
		x += psize;
		wpt.ub += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
	else
	{
		psize = -psize;
		screen_repeat_two(&flic->screen, x, y, *wpt.p2++, psize>>1);
		x += psize;
		if (--opcount != 0)
			goto PPACK;
		++y;
		if (--lp_count > 0)
			goto LPACK;
	}
OUTT:
	return;
}

//**************************************************************************
//
// decode_black
//
//		Decode a BLACK chunk.  Set frame to solid color 0 one
//		line at a time.
//
// Parameter List :
//
//		Uchar	*data		-> data to be decoded
//		Flic	*flic		-> flic data containing where to decode to
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_black(Uchar  *data, Flic *flic)
{
	Pixels2 black;
	int i;
	int height = flic->head.height;
	int width = flic->head.width;
	int x = flic->xoff;
	int y = flic->yoff;

	black.pixels[0] = black.pixels[1] = 0;
	for (i=0; i<height; ++i)
	{
		screen_repeat_two(&flic->screen, x, y+i, black, width/2);
		if (width & 1)	/* if odd set last pixel */
			screen_put_dot(&flic->screen, x+width-1, y+i, 0);
	}
}

//**************************************************************************
//
// decode_literal
//
//		Decode a LITERAL chunk.  Just copy data to screen one line at
//		a time.
//
// Parameter List :
//
//		Uchar	*data		-> data to be decoded
//		Flic	*flic		-> flic data containing where to decode to
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_literal(Uchar  *data, Flic *flic)
{
	int i;
	int height = flic->head.height;
	int width = flic->head.width;
	int x = flic->xoff;
	int y = flic->yoff;

	for (i=0; i<height; ++i)
	{
		oldscreen_copy_seg(&(flic->screen),(long) x,(long)(y+i), (unsigned char *)data,(long) width);
		data += width;
	}
}


typedef void ColorOut(FlicScreen *s, int start, Colour *colors, int count);
	/* This is the type of output parameter to our decode_color below.
	 * Not coincedently screen_put_color is of this type. */


//**************************************************************************
//
// decode_color
//
//		Decode color map.  Put results into output.  The two color
//		compressions are identical except for whether the RGB values
//		are 0-63 or 0-255.  Passing in an output that does the appropriate
//		shifting on the way to the real pallete lets us use the same
//		code for both COLOR_64 and COLOR_256 compression.
//
// Parameter List :
//
//		Uchar			*data			-> data to decode
//		Flic			*flic			-> the flic
//		ColorOut		*output		-> place to put decoded data
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_color(Uchar  *data, Flic *flic, ColorOut *output)
{
	int start = 0;
	Uchar *cbuf = (Uchar *)data;
	Short *wp = (Short *)cbuf;
	Short ops;
	int count;

	ops = *wp;
	cbuf += sizeof(*wp);
	while (--ops >= 0)
	{
		start += *cbuf++;
		if ((count = *cbuf++) == 0)
			count = 256;
		(*output)(&flic->screen, start, (Colour *)cbuf, count);
		cbuf += 3*count;
		start += count;
	}
}

//**************************************************************************
//
// decode_color_256
//
//		Decode COLOR_256 chunk.
//
// Parameter List :
//
//		Uchar			*data			-> data to decode
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_color_256(Uchar  *data, Flic *flic)
{
	decode_color(data, flic, screen_put_colors);
}

//**************************************************************************
//
// decode_color_64
//
//		Decode COLOR_64 chunk.
//
// Parameter List :
//
//		Uchar			*data			-> data to decode
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

static void decode_color_64(Uchar  *data, Flic *flic)
{
	decode_color(data, flic, screen_put_colors_64);
}

//**************************************************************************
//
// decode_frame
//
//		Decode a frame that is in memory already into screen.
//		Here we just loop through each chunk calling appropriate
//		chunk decoder.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//		FrameHead	*frame		-> info about the frame
//		Uchar			*data			-> data to decode
//
// Return Value :
// Modification history :
//
//**************************************************************************

static ErrCode decode_frame(Flic *flic, FrameHead *frame, Uchar  *data)
{
	int			i;
	ChunkHead	*chunk;

	for (i=0; i<frame->chunks; ++i)
	{
		chunk = (ChunkHead  *)data;
		data += chunk->size;
		switch (chunk->type)
		{
			case COLOR_256:
				if (flic->screen.change_palette)
					//decode_color_256((Uchar  *)(chunk+1), flic);
					decode_color_256( (((Uchar  *)chunk) + 4 ), flic);
				break;
			case DELTA_FLC:
				decode_delta_flc((Uchar  *)(chunk+1), flic);
				break;
			case COLOR_64:
				if (flic->screen.change_palette)
				  decode_color_64((Uchar  *)(chunk+1), flic);
				break;
			case DELTA_FLI:
				decode_delta_fli((Uchar  *)(chunk+1), flic);
				break;
			case BLACK_FRAME:
				decode_black((Uchar  *)(chunk+1), flic);
				break;
			case BYTE_RUN:
				decode_byte_run((Uchar  *)(chunk+1), flic);
				break;
			case LITERAL:
				decode_literal((Uchar  *)(chunk+1), flic);
				break;
			default:
				break;
		}
	}
	return 0;
}

static ErrCode file_read_big_block(FILE *file, char *block, Ulong size)
	/* Read in a big block.  Could be bigger than 64K. */
{
	char		*pt = block;
	unsigned	size1;

	while (size != 0)
	{
		size1 = ((size > 0xFFF0) ? 0xFFF0 : size);
		if ( fread(pt, size1, 1, file) != 1 )
			return ErrFlicRead;
		pt += size1;		// Advance pointer to next batch.
		size -= size1;		// Subtract current batch from size to go.
	}
	return 0;
}

//**************************************************************************
//
// fill_in_frame2
//
//		This figures out where the second frame of the flic is
//		(useful for playing in a loop).
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

static ErrCode fill_in_frame2(Flic *flic)
{
	FrameHead head;
	//ErrCode err;

	(*flic->seek)(flic, flic->head.oframe1);
	if ( fread(&head, sizeof(head), 1, flic->file) != 1 )
	  return ErrFlicRead;

	if ( fread(&head, sizeof(head), 1, flic->file) != 1 )
	  return ErrFlicRead;

	flic->head.oframe2 = flic->head.oframe1 + head.size;

	return 0;
}

//**************************************************************************
//
// flic_next_frame
//
//		Advance to next frame of flic.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

static ErrCode flic_next_frame(Flic *flic, BOOL fDecode)
{
	FrameHead	head;
	ErrCode		err = 0;
	long			size;

	if ( fread(&head, sizeof(head), 1, flic->file) != 1 )
	  err = ErrFlicRead;
	else
	{
		if (head.type == FRAME_TYPE)
		{
			size = head.size - sizeof(head);	/* Don't include head. */
			if (size > 0)
			{
				if (size > 500000)
				{
					//TRACE("FLC chunk too big: %d\n",size);
					//Assert(0);
         	FastDebugMsg(String("flic_next_frame: FLC chunk too big: %d", size));
					size = 64000;
				}
				if (!(err = file_read_big_block(flic->file, pcxbuf, size)))
				{
					if ( fDecode )
						err = decode_frame(flic, &head, (unsigned char *)pcxbuf);
				}
			}
		}
		else
			err = ErrFlicBadFrame;
	}
	return err;
}

//**************************************************************************
//
// flic_check_frame
//
//		A dummy routine to be called for each frame.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

static Boolean flic_check_frame(Flic *flic)
{
  return TRUE;
}

//**************************************************************************
//
//				File io
//
//**************************************************************************

static ErrCode flic_open(Flic *flic, const char *filename)
{
  return (flic->file = fopen(filename, "rb")) ? 0 : ErrFlicAccess;
}

static ErrCode flic_seek(Flic *flic, long offset)
{
  return fseek(flic->file, offset, SEEK_SET) ? ErrFlicSeek : 0;
}

//**************************************************************************
//
// FlicInit
//
//		Initializes a flic object.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

void FlicInit(Flic *flic, unsigned screen_width, unsigned screen_height, char change_palette, char *Buff)
{
  flic->file = NULL;
  //flic->lib.names = NULL;

  flic->open = flic_open;  /* Set routines for single flick file access */
  flic->seek = flic_seek;

  flic->check_frame = flic_check_frame;  /* Select a dummy routine */

  /** Info used by low level routines */

  flic->screen.pixels = (unsigned char *)Buff; // (char *) 0xa0000;
  flic->screen.width = screen_width;
  flic->screen.height = screen_height;
  flic->screen.change_palette = change_palette;
}

//**************************************************************************
//
// FlicOpen
//
//		Opens a flic file or locates a flic file in a library.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//		const char	*filename	-> filename of flic to open
//
// Return Value :
// Modification history :
//
//**************************************************************************

ErrCode FlicOpen(Flic *flic, const char *filename)
{
	ErrCode err;

	flic->xoff = flic->yoff = 0;

	if ( !(err = (*flic->open)(flic, filename)) )
	{
		if (fread(&flic->head, sizeof(flic->head), 1, flic->file) != 1)
		  err = ErrFlicRead;
		else
		{
			flic->name = filename;

			if (flic->head.type == FLC_TYPE)
			{
				/* Seek frame 1. */
				(*flic->seek)(flic, flic->head.oframe1);
				return 0;
			}
			if (flic->head.type == FLI_TYPE)
			{
				long i;
				/* Do some conversion work here. */
				flic->head.oframe1 = sizeof(flic->head);
				i=flic->head.speed * 1000L;
				flic->head.speed = i / 70;
				return 0;
			}
			else
			{
				err = ErrFlicBad;
				printf("flic was bad! \n");
				_getch();
				exit(0);
			}
		}
	}

	FlicClose(flic);		/* Close down and scrub partially opened flic. */
	return err;
}

//**************************************************************************
//
// FlicSetOrigin
//
//		Sets the location of the top left corner of the flic in terms of screen
//		coordinates.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//		unsigned		x				-> x-coord of origin
//		unsigned		y				-> y-coord of origin
//
// Return Value :
// Modification history :
//
//**************************************************************************

void FlicSetOrigin(Flic *flic, unsigned x, unsigned y)
{
  flic->xoff = x;
  flic->yoff = y;
}

//**************************************************************************
//
// FlicClose
//
//		Closes a flic file if one is open. If a library is open, this function has
//		no effect.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

void FlicClose(Flic *flic)
{
	if ( flic->file )
	{
		fclose(flic->file);
		flic->file = NULL;
	}
}

//**************************************************************************
//
// flic_play_loop_timer
//
//		This routine is called at the end of each timer period. The timer is used
//		by the FlicPlay routine. The global variable timer_flag indicates that
//		a time interval has elapsed. FlicPlay resets timer_flag for the next
//		interval.
//
//		Stack checking must be disabled.
//
// Parameter List :
// Return Value :
// Modification history :
//
//**************************************************************************

#ifdef DEBUG
#pragma off (check_stack)
#endif

static volatile int timer_flag=1;

static void flic_play_loop_timer()
{
	timer_flag = 1;
}

#ifdef DEBUG
#pragma on  (check_stack)
#endif

//**************************************************************************
//
// FlicPlay
//
//		This is the engine for the flic display system. Call this function to
//		display a flic.
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

ErrCode FlicPlay(Flic *flic, Ulong max_loop)
{
/*
	ErrCode err;
	HTIMER timer;

	flic->status.max_loop_count = max_loop;
	flic->status.max_frame_index = flic->head.frames;

	if (flic->head.oframe2 == 0)
	{
		fill_in_frame2(flic);
		//	FastBlitRegionSVGA((char *)(0xa0000),(char *)(flic->screen.pixels),(long)0,(long)640,(long)480,(long)0,(long)640);
		//	BlitSVGAPage((char *)(flic->screen.pixels));
		//	BlitRegionSVGATst((void *)0xa0000,(void *)(flic->screen.pixels),(long)640,(long)480,(long)640);
		//BlitRegionSVGA((char *)0xa0000,(char *)flic->screen.pixels,(long)0,(long)640,(long)480,(long)0,(long)640);
	}

	// Seek to first frame.
	(*flic->seek)(flic, flic->head.oframe1);
	// Save time to move on.

	//videoSetMode(3);
	//printf("speed is %d\n",flic->head.speed);
	//_getch();
	//exit(0);

	//timer = AIL_register_timer(flic_play_loop_timer);
	//AIL_set_timer_period(timer, flic->head.speed* 1000);
	//timer_flag = 0;
	//AIL_start_timer(timer);

	// Display first frame.
	if (err = flic_next_frame(flic, TRUE))
	{
		//AIL_release_timer_handle(timer);
		return err;
	}

	for (flic->status.loop_count = 0; !max_loop || flic->status.loop_count < max_loop;
		 ++flic->status.loop_count)
	{
		// Seek to second frame
		(*flic->seek)(flic, flic->head.oframe2);
		// Loop from 2nd frame thru ring frame
		for (flic->status.frame_index=0;
			  (flic->status.frame_index < (flic->head.frames-1)) ||
			  ( (flic->status.frame_index < flic->head.frames) &&
			  ( !max_loop || (flic->status.loop_count < (max_loop - 1))));
			  ++flic->status.frame_index)
		{
			while (!timer_flag);

			if (err = flic_next_frame(flic, TRUE))
				return err;
			//else
			//	FastBlitRegionSVGA((char *)(0xa0000),(char *)(flic->screen.pixels),(long)0,(long)640,(long)480,(long)0,(long)640);
			//	BlitSVGAPage((char *)(flic->screen.pixels));
			//	BlitRegionSVGATst((void *)0xa0000,(void *)(flic->screen.pixels),(long)640,(long)480,(long)640);
			//	BlitRegionSVGA((char *)0xa0000,(char *)flic->screen.pixels,(long)0,(long)640,(long)480,(long)0,(long)640);

			timer_flag = 0;
		}
	}

	//AIL_release_timer_handle(timer);
*/
	return 0;
}

void FlicSeekFirst(Flic *flic)
{
//	if (flic->head.oframe2 == 0)
//		fill_in_frame2(flic);

	// Seek to first frame.
	flic_seek(flic, flic->head.oframe1);

	// Display first frame.
	//flic_next_frame(flic, TRUE);	HJH
	flic->status.frame_index=0;//1;	HJH
}

int FlicAdvance(Flic *flic, BOOL fDecode)
{
	// Seek to second frame
	//(*flic->seek)(flic, flic->head.oframe2);
	// Loop from 2nd frame thru ring frame

	//	for (flic->status.frame_index=0;
	//		 (flic->status.frame_index < (flic->head.frames-1)) ||
	//		 ( (flic->status.frame_index < flic->head.frames) &&
	//		 ( !max_loop || (flic->status.loop_count < (max_loop - 1))));
	//		 ++flic->status.frame_index)
	//	{

	if(flic->status.frame_index < flic->head.frames)
	{
		flic_next_frame(flic, fDecode);
		flic->status.frame_index++;
		//memcpy(buffer, buffer2, (HSIZE*VSIZE));
		return(1);
	}
	else
		return(0);

	//	if (flic->check_frame && !(*flic->check_frame)(flic))
	//	  	return 0;
}

static void center_flic(Flic *flic)
	/* Set flic.xoff and flic.yoff so flic plays centered rather
	 * than in upper left corner of display. */
{
	flic->xoff = (flic->screen.width - (signed)flic->head.width)/2;
	flic->yoff = (flic->screen.height - (signed)flic->head.height)/2;
}

void set_flic_origin(Flic *flic, int x, int y)
{
  flic->xoff = x;
  flic->yoff = y;
}

int frame_check(Flic *flic)
{
/*
  char key;

	if (Esc())
   {
		//AbortScript = TRUE;
		return 0;
   }

	if (flic->status.frame_index == FlcSoundFrames[FlcSoundIndex][0])
	{
		// if the "sound" number is less than 100, then it's a VOC buffer
		if (FlcSoundFrames[FlcSoundIndex][1] < 100)
		{
			PlayVoc(FlcSoundFrames[FlcSoundIndex][1],-1);
		}
		else	// otherwise a raw sound data file to be streamed off of disk
		{
			if (FlcSoundNameList[FlcSoundNameIndex][0]!='\0')
			{
			retcode = lmLibSeek(SNDLIBINFO,FlcSoundNameList[FlcSoundNameIndex++],&SNDoffset,&SNDlength);
			if (retcode == 0)
				StPlay(glbPlayRate,IGNORE_AUDIO_GAPS);
			}
		}

		// now that the sound has been triggered, ready the next frame number
		FlcSoundIndex++;
	}
	else
	if (FlcSoundFrames[FlcSoundIndex][0] == 9998)	// start music?
	{
		PlayXmi(FlcSoundFrames[FlcSoundIndex++][1],FALSE);
		XmiVolume(1,127,0);
	}
	RefreshSound();
*/
  return 1;
}

//**************************************************************************
//
// FlicStart
//
// Parameter List :
//
//		char			*filename	-> filename of flic to start processing
//		char			*buffer		-> screen buffer to put data into
//		Flic			*flic			-> the flic
//		int			usepal		-> flag - should we use the palette? (I think)
//
// Return Value :
// Modification history :
//
//**************************************************************************

int FlicStart(char *filename, int width, int height, char *buffer, Flic *flic, char usepal)
{
	FlicInit(flic, width, height, usepal, buffer);

	if(FlicOpen(flic, filename))
		return(-1);

	FlicSetOrigin(flic, 0, 0);
	FlicSeekFirst(flic);
	FlicAdvance(flic, TRUE);		// HJH - we go to 1st, then go process it

	//memcpy(buffer, buffer2, (HSIZE*VSIZE));

	return(0);
}

//**************************************************************************
//
// FlicStop
//
// Parameter List :
//
//		Flic			*flic			-> the flic
//
// Return Value :
// Modification history :
//
//**************************************************************************

void FlicStop(Flic *flic)
{
	FlicClose(flic);
}

//**************************************************************************
//
// FlicGetStats
//
//		Gets statistics for the given file.
//
// Parameter List :
//
//		char			*filename		-> the flic filename
//		Flic			*flic				-> the flic
//		int			*piBufferSize	-> this function will fill in how large a
//												buffer is needed to store a screen of
//												this flick
//		int			*piColourPalSize->this function will fill in how large
//												the colour palette is
//
// Return Value :
//
//		ErrCode		->	0 if successfull, non-zero otherwise
//
// Modification history :
//
//		05jun96:HJH	-> creation
//
//**************************************************************************

ErrCode FlicGetStats(char *filename, int width, int height, Flic *flic, int *piBufferSize, int *piColourPalSize)
{
	ErrCode err=0;
	FlicInit(flic, width, height, 1, NULL);

	flic->xoff = flic->yoff = 0;

	if ( !(err = (*flic->open)(flic, filename)) )
	{
		if (fread(&flic->head, sizeof(flic->head), 1, flic->file) != 1)
		  err = ErrFlicRead;
	}

	if ( !err && piBufferSize )
		*piBufferSize = flic->head.width * flic->head.height;

	FlicSeekChunk( flic, 0, COLOR_256, piColourPalSize );
	FlicClose(flic);		/* Close down and scrub partially opened flic. */

	return err;
}

//**************************************************************************
//
// FlicGetColourPalette
//
//		To get the colour palette from the given flic file.
//
// Parameter List :
//
//		char			*filename	-> the flic file to extract the palette from
//		char			**ppBuffer	-> the place to put the palette
//
// Return Value :
//
//		ErrCode		->	0 if successful, non-zero otherwise
//
// Modification history :
//
//		10jun96:HJH	->	creation
//		19jun96:HJH	->	removed <<2 after colors->r/g/b
//
//**************************************************************************

ErrCode	FlicGetColourPalette(CHAR *filename, int width, int height, CHAR **ppBuffer, INT *piNumColours)
{
	Flic			flic;
	INT			iBufferSize;
	INT			iColourPaletteSize=0;
	ErrCode		err = 0;
	ChunkHead	*chunk = NULL;
	CHAR			*pcPosition;

	if ( FlicGetStats( filename, width, height, &flic, &iBufferSize, &iColourPaletteSize ) )
		err = -1;

	if ( piNumColours )
		*piNumColours = iColourPaletteSize;

	if ( !err && iColourPaletteSize )
	{
		FlicInit(&flic, flic.head.width, flic.head.height, 1, NULL);

		if(FlicOpen(&flic, filename))
			return(-1);

		FlicSetOrigin(&flic, 0, 0);

		*ppBuffer	= (CHAR *)MemAlloc( iColourPaletteSize );
		chunk			= (ChunkHead *)FlicSeekChunk( &flic, 0, COLOR_256, NULL );
		pcPosition	= (CHAR *)(((BYTE *)chunk)+6);

		if ( chunk )
		{
			INT			ix, end, count, start = 0;
			Uchar			*cbuf = (Uchar *)pcPosition;
			Short			*wp = (Short *)cbuf;
			Short			ops;
			Colour			*colors;

			ops = *wp;
			cbuf += sizeof(*wp);
			while (--ops >= 0)
			{
				start += *cbuf++;
				if ((count = *cbuf++) == 0)
					count = 256;
				{
					colors = (Colour *)cbuf;
					end	 = start + count;

					for (ix = start; ix < end; ++ix)
					{
						(*ppBuffer)[ix*3]  =colors->r;
						(*ppBuffer)[ix*3+1]=colors->g;
						(*ppBuffer)[ix*3+2]=colors->b;
						++colors;
					}
				}
				cbuf += 3*count;
				start += count;
			}
		}
	}
	else
	{
		err = -1;
	}

	return(err);
}

//**************************************************************************
//
// FlicSeekChunk
//
//		To seek to a particular chunk in the flic file.
//
// Parameter List :
//
//		Flic			*flic			-> the flic to search
//		INT			iFrame		-> frame in which to search
//		ChunkTypes	eType			-> type of chunk to search for
//		INT			*piChunkSize-> the function puts the size of the chunk here
//
// Return Value :
//
//		char *		-> the returned chunk
//
// Modification history :
//
//		05jun96:HJH	->	creation
//
//**************************************************************************

CHAR *FlicSeekChunk(Flic *flic, INT iFrame, ChunkTypes eType, INT *piChunkSize)
{
	FrameHead	head;
	LONG			lSize;
	BOOL			fFound = FALSE;
	ErrCode		err=0;
  INT       i;

	FlicSeekFirst(flic);

	for ( i=0 ; i<iFrame ; i++ )
		FlicAdvance(flic, FALSE);

	if ( fread(&head, sizeof(head), 1, flic->file) != 1 )
	  err = ErrFlicRead;
	else
	{
		if (head.type == FRAME_TYPE)
		{
			lSize = head.size - sizeof(head);	/* Don't include head. */
			if (lSize > 0)
			{
				if (lSize > 500000)
				{
					_getch();
					lSize = 64000;
				}
				if (!(err = file_read_big_block(flic->file, pcxbuf, lSize)))
				{
					FrameHead	*frame = &head;
					Uchar			*data = (unsigned char *)pcxbuf;
					int			i;
					ChunkHead	*chunk;

					for (i=0; i<head.chunks; ++i)
					{
						chunk = (ChunkHead  *)data;
						data += chunk->size;
						if ( chunk->type == eType )
						{
							if ( piChunkSize )
								*piChunkSize = chunk->size;
							return( ((char  *)chunk) );
						}
					}
				}
			}
		}
		else
			err = ErrFlicBadFrame;
	}

	return(NULL);
}

//**************************************************************************
//
// FlicFindByteRunBeforeFrame
//
//		To find the closest byte run frame before the given frame.
//
// Parameter List :
//
//		Flic			*flic			-> the flic to extract data from
//		INT			iFrame		-> the frame number before which to find a
//											byte run
//
// Return Value :
//
//		INT			->	frame # if successful, BAD_INDEX otherwise
//
// Modification history :
//
//		19jun96:HJH	->	creation
//
//**************************************************************************

INT FlicFindByteRunBeforeFrame(Flic *flic, INT iFrame)
{
	INT	iRet = BAD_INDEX;
  INT i;


	for ( i=iFrame-1 ; i>=0 ; i-- )
	{
		if ( FlicSeekChunk( flic, i, BYTE_RUN, NULL ) )
			return(i);
	}

	return( iRet );
}

//**************************************************************************
//
// FlicFillBitmapData
//
//		To put flic bitmap data for a particular frame into the given bitmap.
//
// Parameter List :
//
//		Flic			*flic			-> the flic to extract data from
//		INT			iPrevFrame	-> the previous frame decoded, BAD_INDEX if unknown
//		INT			iFrame		-> the frame number to extract bitmap data for
//		HBITMAP		hBitmap		-> place to put the bitmap data
//
// Return Value :
//
//		ErrCode		->	0 if successful, non-zero otherwise
//
// Modification history :
//
//		10jun96:HJH	->	creation
//		19jun96:HJH -> removed redundancies
//						-> added check for iPrevFrame != (iFrame-1)
//
//**************************************************************************

ErrCode FlicFillBitmapData( Flic *flic, INT iPrevFrame, INT iFrame, HBITMAP hBitmap )
{
	ErrCode		err = 0;
	ChunkHead	*chunk = NULL;
	DIBSECTION	dibSection;
	INT			i, iStart;

	GetObject( hBitmap, sizeof(DIBSECTION), &dibSection );

	flic->screen.pixels = (Pixel *)dibSection.dsBm.bmBits;

	if ( iPrevFrame == BAD_INDEX || iPrevFrame != (iFrame-1) )
	{
		iStart = FlicFindByteRunBeforeFrame( flic, iFrame );
		if ( iStart == BAD_INDEX )
			iStart = 0;
	}
	else
		iStart = iFrame;

	for ( i=iStart ; i<=iFrame ; i++ )
	{
		chunk	= (ChunkHead *)FlicSeekChunk( flic, i, DELTA_FLC, NULL );
		if ( chunk )
			decode_delta_flc( ((BYTE*)chunk)+6, flic );
		else
		{
			chunk	= (ChunkHead *)FlicSeekChunk( flic, i, BYTE_RUN, NULL );
			if ( chunk )
			{
				FlicClearBitmap( hBitmap, 0 );
				decode_byte_run( ((BYTE*)chunk)+6, flic );
			}
		}

		flic->status.frame_index = i;
	}

	return( err );
}

//**************************************************************************
//
// FlicFillFrameData
//
//		To put flic frame data for a particular frame into the given bitmap.
//		This is raw data. Either a byte run or deltas.
//
// Parameter List :
//
//		Flic			*flic			-> the flic to extract data from
//		INT			iPrevFrame	-> the previous frame decoded, BAD_INDEX if unknown
//		INT			iFrame		-> the frame number to extract bitmap data for
//		CHAR			*pcBuffer	-> place to put the frame data
//		INT			*piSize		-> size of buffer ; we return how many bytes
//											were placed in the buffer
//
// Return Value :
//
//		ErrCode		->	0 if successful, non-zero otherwise
//
// Modification history :
//
//		03jul96:HJH	->	creation
//
//**************************************************************************

ErrCode FlicFillFrameData(
	Flic	*flic,
	INT	iPrevFrame,
	INT	iFrame,
	CHAR	*pcBuffer,
	INT	*piNumBytes
	)
{
	ErrCode		err = 0;
	ChunkHead	*chunk = NULL;
	INT			i, iStart, iNumBytesInBuffer;

	Assert(piNumBytes);
	//GetObject( hBitmap, sizeof(DIBSECTION), &dibSection );

	flic->screen.pixels = (Pixel *)pcBuffer;
	iNumBytesInBuffer   = *piNumBytes;

	if ( iPrevFrame == BAD_INDEX || iPrevFrame != (iFrame-1) )
	{
		iStart = FlicFindByteRunBeforeFrame( flic, iFrame );
		if ( iStart == BAD_INDEX )
			iStart = 0;
	}
	else
		iStart = iFrame;

	for ( i=iStart ; i<=iFrame ; i++ )
	{
		chunk	= (ChunkHead *)FlicSeekChunk( flic, i, DELTA_FLC, piNumBytes );
		if ( chunk )
		{
			Assert(*piNumBytes <= iNumBytesInBuffer);
			memcpy( pcBuffer, ((BYTE*)chunk)+6, *piNumBytes );
			//decode_delta_flc( ((BYTE*)chunk)+6, flic );
		}
		else
		{
			chunk	= (ChunkHead *)FlicSeekChunk( flic, i, BYTE_RUN, piNumBytes );
			if ( chunk )
			{
				Assert(*piNumBytes <= iNumBytesInBuffer);
				//FlicClearBitmap( hBitmap, 0 );
				memcpy( pcBuffer, ((BYTE*)chunk)+6, *piNumBytes );
				//decode_byte_run( ((BYTE*)chunk)+6, flic );
			}
		}

		flic->status.frame_index = i;
	}

	return( err );
}

//**************************************************************************
//
// FlicClearBitmap
//
//		To clear the given bitmap to a certain colour index value.
//
// Parameter List :
//
//		HBITMAP		hBitmap		-> bitmap to clear
//		INT			iFrame		-> the colour index to use in clearing
//
// Return Value :
//
//		none
//
// Modification history :
//
//		11jun96:HJH	->	creation
//
//**************************************************************************

void FlicClearBitmap( HBITMAP hBitmap, INT iColourIndex )
{
	DIBSECTION	dibSection;
	INT			i;

	CHECKV(hBitmap);

	CHECKV(GetObject(hBitmap, sizeof(DIBSECTION), &dibSection));
	CHECKV(dibSection.dsBm.bmBits);
	CHECKV(dibSection.dsBm.bmHeight);
	CHECKV(dibSection.dsBm.bmWidth);

	for ( i=0 ; i<dibSection.dsBm.bmHeight*dibSection.dsBm.bmWidth ; i++ )
	{
		((CHAR *)dibSection.dsBm.bmBits)[i] = iColourIndex;
	}
}

