#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "DirectDraw Calls.h"
	#include <stdio.h>
	#include "debug.h"
	#if defined( JA2 ) || defined( UTIL )
		#include "video.h"
	#else
		#include "video2.h"
	#endif
	#include "himage.h"
	#include "vobject.h"
	#include "vobject_private.h"
	#include "video_private.h"
	#include "wcheck.h"
	#include "vobject_blitters.h"
	#include "shading.h"
#endif

BOOLEAN ShadesCalculateTables(SGPPaletteEntry *p8BPPPalette);
BOOLEAN ShadesCalculatePalette(SGPPaletteEntry *pSrcPalette, SGPPaletteEntry *pDestPalette, UINT16 usRed, UINT16 usGreen, UINT16 usBlue, BOOLEAN fMono);
void FindIndecies(SGPPaletteEntry *pSrcPalette, SGPPaletteEntry *pMapPalette, UINT8 *pTable);
void FindMaskIndecies(UINT8 *, UINT8 *, UINT8 *);

SGPPaletteEntry Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES+3][256];
UINT8 ubColorTables[HVOBJECT_SHADE_TABLES+3][256];

UINT16	IntensityTable[65536];
UINT16	ShadeTable[65536];
UINT16	White16BPPPalette[ 256 ];
FLOAT   guiShadePercent = (FLOAT)0.48;
FLOAT   guiBrightPercent = (FLOAT)1.1;

BOOLEAN ShadesCalculateTables(SGPPaletteEntry *p8BPPPalette)
{
UINT32	uiCount;

		// Green palette
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[0], 0, 255, 0, TRUE);
		// Blue palette
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES],  0, 0, 255, TRUE);
		// Yellow palette
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES+1], 255, 255, 0, TRUE);
		// Red palette
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES+2], 255, 0, 0, TRUE);

		// these are the brightening tables, 115%-150% brighter than original
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[1], 293, 293, 293, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[2], 281, 281, 281, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[3], 268, 268, 268, FALSE);

		// palette 4 is the non-modified palette.
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[4], 255, 255, 255, FALSE);

		// the rest are darkening tables, right down to all-black.
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[5], 195, 195, 195, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[6], 165, 165, 165, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[7], 135, 135, 135, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[8], 105, 105, 105, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[9], 75, 75, 75, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[10], 45, 45, 45, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[11], 36, 36, 36, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[12], 27, 27, 27, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[13], 18, 18, 18, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[14], 9, 9, 9, FALSE);
		ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[15], 0, 0, 0, FALSE);


	// Remap the shade colors to the original palette
	for(uiCount=0; uiCount < (HVOBJECT_SHADE_TABLES+3); uiCount++)
	{
		FindIndecies(Shaded8BPPPalettes[uiCount], p8BPPPalette, ubColorTables[uiCount]);
		ubColorTables[uiCount][0]=0;
	}

	return(TRUE);
}

BOOLEAN ShadesCalculatePalette(SGPPaletteEntry *pSrcPalette, SGPPaletteEntry *pDestPalette, UINT16 usRed, UINT16 usGreen, UINT16 usBlue, BOOLEAN fMono)
{
UINT32 cnt, lumin;
UINT32 rmod, gmod, bmod;

	Assert( pSrcPalette != NULL );
	Assert( pDestPalette != NULL );

	for ( cnt = 0; cnt < 256; cnt++ )
	{
		if(fMono)
		{
			lumin=(pSrcPalette[ cnt ].peRed*299/1000)+ (pSrcPalette[ cnt ].peGreen*587/1000)+(pSrcPalette[ cnt ].peBlue*114/1000);
			rmod=usRed*lumin/255;
			gmod=usGreen*lumin/255;
			bmod=usBlue*lumin/255;
		}
		else
		{
			rmod = (usRed*pSrcPalette[ cnt ].peRed/255);
			gmod = (usGreen*pSrcPalette[ cnt ].peGreen/255);
			bmod = (usBlue*pSrcPalette[ cnt ].peBlue/255);
		}

		pDestPalette[ cnt ].peRed = (UINT8)__min(rmod, 255);
		pDestPalette[ cnt ].peGreen = (UINT8)__min(gmod, 255);
		pDestPalette[ cnt ].peBlue = (UINT8)__min(bmod, 255);
	}

	return(TRUE);
}




void FindIndecies(SGPPaletteEntry *pSrcPalette, SGPPaletteEntry *pMapPalette, UINT8 *pTable)
{
UINT16 usCurIndex, usCurDelta, usCurCount;
UINT32 *pSavedPtr;

__asm {

// Assumes:
//	ESI = Pointer to source palette (shaded values)
//	EDI = Pointer to original palette (palette we'll end up using!)
//	EBX = Pointer to array of indecies

		mov		esi, pSrcPalette
		mov		edi, pMapPalette
		mov		ebx, pTable

		mov		BYTE PTR [ebx],0						; Index 0 is always 0, for trans col
		inc		ebx

		add		esi,4												; Goto next color entry
		add		edi,4
		mov		pSavedPtr, edi							; Save pointer to original pal

		mov		usCurCount, 255							; We'll check cols 1-255

DoNextIndex:

		mov		edi, pSavedPtr							; Restore saved ptr
		mov		usCurIndex, 256							; Set found index & delta to some
		mov		usCurDelta, 0ffffh					; val so we get at least 1 col.

		mov		ecx,255											; Check cols 1-255 of orig pal
		push	ebx
		xor		bx,bx

NextColor:
		xor		ah,ah                       ; Calc delta between shaded color
		mov		al,[edi]										; and a color in the orig palette.
		mov		bl,[esi]										; Formula:
		sub		ax,bx                       ;  Delta = abs(red-origred) +
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC1
		neg		ax
NC1:mov		dx,ax                       ;          abs(blue-origblue)
		xor		ah,ah
		mov		al,[edi+1]
		mov		bl,[esi+1]
		sub		ax,bx
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC2
		neg		ax
NC2:add		dx,ax
		xor		ah,ah
		mov		al,[edi+2]
		mov		bl,[esi+2]
		sub		ax,bx
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC3
		neg		ax
NC3:add		dx,ax

		cmp		dx,usCurDelta								; If delta < old delta
		jae		NotThisCol									;	Save this delta and it's
		mov		ax,256                      ;	palette index
		mov		[usCurDelta],dx
		sub   ax,cx
		mov		[usCurIndex],ax
NotThisCol:
		add		edi,4												; Try next color in orginal pal
		dec		cx
		jnz   NextColor

		pop		ebx
		mov		ax,usCurIndex								; By now, usCurIndex holds pal index
		mov		[ebx],al										; of closest color in orig pal
		inc		ebx													; so save it, then repeat above
		add		esi,4												; for the other cols in shade pal
		dec		usCurCount
		jnz		DoNextIndex
	}
}

/**********************************************************************************************
 BuildShadeTable

	Builds a 16-bit color shading table. This function should be called only after the current
	video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
	globals for masks and shifts have been initialized by that function), and before any
	blitting is done.

	Using the table is a straight lookup. The pixel to be shaded down is used as the index into
	the table and the entry at that point will be a pixel that is 25% darker.

**********************************************************************************************/
void BuildShadeTable(void)
{
	UINT16 red, green, blue;
	UINT16 index;

	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));
				ShadeTable[index]=Get16BPPColor(FROMRGB(red*guiShadePercent, green*guiShadePercent, blue*guiShadePercent));
			}

	memset( White16BPPPalette, 65535, sizeof( White16BPPPalette ) );
}


/**********************************************************************************************
 BuildIntensityTable

	Builds a 16-bit color shading table. This function should be called only after the current
	video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
	globals for masks and shifts have been initialized by that function), and before any
	blitting is done.



**********************************************************************************************/
void BuildIntensityTable(void)
{
	UINT16 red, green, blue;
	UINT16 index;
	FLOAT  dShadedPercent = (FLOAT)0.80;


#if 0

	UINT32 lumin;
	UINT32 rmod, gmod, bmod;

	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));

				lumin=( red*299/1000)+ ( green*587/1000 ) + ( blue*114/1000 );

				//lumin = __min(lumin, 255);
				rmod=(255*lumin)/256;
				gmod=(100*lumin)/256;
				bmod=(100*lumin)/256;

				//rmod = __m( 255, rmod );

				IntensityTable[index]=Get16BPPColor( FROMRGB( rmod, gmod , bmod ) );
			}
#endif



	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));
				IntensityTable[index]=Get16BPPColor(FROMRGB(red*dShadedPercent, green*dShadedPercent, blue*dShadedPercent));
			}

}



void SetShadeTablePercent( FLOAT uiShadePercent )
{
	guiShadePercent = uiShadePercent;
	BuildShadeTable( );
}


#ifdef JA2	// Jul. 23 '97 - ALEX - because Wizardry isn't using it & no longer has a version of Set8BPPPalette() available
void Init8BitTables(void)
{
SGPPaletteEntry Pal[256];
UINT32 uiCount;

	// calculate a grey-scale table for the default palette
	for(uiCount=0; uiCount < 256; uiCount++)
	{
		Pal[uiCount].peRed=(UINT8)(uiCount%128)+128;
		Pal[uiCount].peGreen=(UINT8)(uiCount%128)+128;
		Pal[uiCount].peBlue=(UINT8)(uiCount%128)+128;
	}

	Pal[0].peRed=0;
	Pal[0].peGreen=0;
	Pal[0].peBlue=0;

	Set8BPPPalette(Shaded8BPPPalettes[4]);
}

BOOLEAN Set8BitModePalette(SGPPaletteEntry *pPal)
{
	ShadesCalculateTables(pPal);
	Set8BPPPalette(pPal);
	return(TRUE);
}
#endif
