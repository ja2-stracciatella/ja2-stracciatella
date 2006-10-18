
//#define UNICODE
#include "Types.h"
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include "SGP.h"
#include "memman.h"
#include "FileMan.h"
#include "Font.h"
#include "Debug.h"
#include "VSurface.h"
#include "VSurface_Private.h"
#include <DDraw.h>
#include "winfont.h"
#include "Font.h"


INT32 FindFreeWinFont( void );
BOOLEAN gfEnumSucceed = FALSE;



#define         MAX_WIN_FONTS   10

// Private struct not to be exported
// to other modules
typedef struct
{
  HFONT       hFont;
  COLORVAL    ForeColor;
  COLORVAL    BackColor;

} HWINFONT;

LOGFONT gLogFont;

HWINFONT	WinFonts[ MAX_WIN_FONTS ];


void Convert16BitStringTo8BitChineseBig5String( UINT8 *dst, const wchar_t *src )
{
	INT32 i, j;
	const char *ptr;

	i = j = 0;
	ptr = (const char*)src;
	while( ptr[j] || ptr[j + 1] )
	{
		if( ptr[j] )
		{
			dst[i] = ptr[j];
			dst[ i + 1 ] = '\0';
			i++;
		}
		j++;
	}
}



void InitWinFonts( )
{
  memset( WinFonts, 0, sizeof( WinFonts ) );
}

void ShutdownWinFonts( )
{

}

INT32 FindFreeWinFont( void )
{
  INT32 iCount;

	for( iCount = 0; iCount < MAX_WIN_FONTS; iCount++ )
  {
		if( WinFonts[ iCount ].hFont == NULL )
    {
			return( iCount );
    }
  }

	return( -1 );
}


HWINFONT *GetWinFont( INT32 iFont )
{
  if ( iFont == -1 )
  {
    return( NULL );
  }

  if ( WinFonts[ iFont ].hFont == NULL )
  {
    return( NULL );
  }
  else
  {
    return( &( WinFonts[ iFont ] ) );
  }
}

UINT16 gzFontName[32];

INT32 CreateWinFont( INT32 iHeight, INT32 iWidth, INT32 iEscapement,
                     INT32 iWeight, BOOLEAN fItalic,  BOOLEAN fUnderline,  BOOLEAN fStrikeOut, STR16 szFontName, INT32 iCharSet )
{
  INT32   iFont;
  HFONT   hFont;
	UINT8  szCharFontName[32]; //32 characters including null terminator (matches max font name length)
  // Find free slot
  iFont = FindFreeWinFont( );

  if ( iFont == -1 )
  {
    return( iFont );
  }

	//SET UP FONT WE WANT TO LOAD HERE
	wcscpy( gzFontName, szFontName );

	//ATTEMPT TO LOAD THE FONT NOW
	sprintf( szCharFontName, "%S", szFontName );
	if( DoesWinFontExistOnSystem( szFontName, iCharSet ) )
	{
		gLogFont.lfHeight = iHeight;
		gLogFont.lfWidth = 0;
		hFont = CreateFontIndirect( &gLogFont );
	}
	else
	{
		FatalError( "Cannot load subtitle Windows Font: %S.", szFontName );
		return( -1 );
	}

  if ( hFont == NULL )
  {
    return( -1 );
  }

  // Set font....
  WinFonts[ iFont ].hFont = hFont;

  return( iFont );
}

void  DeleteWinFont( INT32 iFont )
{
  HWINFONT *pWinFont;

  pWinFont = GetWinFont( iFont );

  if ( pWinFont != NULL )
  {
    DeleteObject( pWinFont->hFont );
  }
}


void SetWinFontForeColor( INT32 iFont, COLORVAL *pColor )
{
  HWINFONT *pWinFont;

  pWinFont = GetWinFont( iFont );

  if ( pWinFont != NULL )
  {
    pWinFont->ForeColor = ( *pColor );
  }
}


void SetWinFontBackColor( INT32 iFont, COLORVAL *pColor )
{
  HWINFONT *pWinFont;

  pWinFont = GetWinFont( iFont );

  if ( pWinFont != NULL )
  {
    pWinFont->BackColor = ( *pColor );
  }
}


void PrintWinFont( UINT32 uiDestBuf, INT32 iFont, INT32 x, INT32 y, UINT16 *pFontString, ...)
{
  va_list                 argptr;
  wchar_t									string2[512];
	char										string[512];
  HVSURFACE               hVSurface;
  LPDIRECTDRAWSURFACE2    pDDSurface;
  HDC                     hdc;
  RECT                    rc;
  HWINFONT                *pWinFont;
  int                     len;
  SIZE                    RectSize;

  pWinFont = GetWinFont( iFont );

  if ( pWinFont == NULL )
  {
    return;
  }

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	len = vswprintf(string2, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

#ifdef TAIWANESE
	Convert16BitStringTo8BitChineseBig5String( string, string2 );
#else
	sprintf( string, "%S", string2 );
#endif

  // Get surface...
  GetVideoSurface( &hVSurface, uiDestBuf );

  pDDSurface = GetVideoSurfaceDDSurface( hVSurface );

  IDirectDrawSurface2_GetDC( pDDSurface, &hdc );

  SelectObject(hdc, pWinFont->hFont );
  SetTextColor( hdc, pWinFont->ForeColor );
  SetBkColor(hdc, pWinFont->BackColor );
  SetBkMode(hdc, TRANSPARENT);

  GetTextExtentPoint32( hdc, string, len, &RectSize );
  SetRect(&rc, x, y, x + RectSize.cx, y + RectSize.cy );
  ExtTextOut( hdc, x, y, ETO_OPAQUE, &rc, string, len, NULL );
  IDirectDrawSurface2_ReleaseDC( pDDSurface, hdc );

}

INT16 WinFontStringPixLength(const wchar_t *string2, INT32 iFont )
{
  HWINFONT                *pWinFont;
  HDC                     hdc;
  SIZE                    RectSize;
	char			string[512];

  pWinFont = GetWinFont( iFont );

  if ( pWinFont == NULL )
  {
    return( 0 );
  }

#ifdef TAIWANESE
	Convert16BitStringTo8BitChineseBig5String( string, string2 );
#else
	sprintf( string, "%S", string2 );
#endif

  hdc = GetDC(NULL);
  SelectObject(hdc, pWinFont->hFont );
  GetTextExtentPoint32( hdc, string, strlen(string), &RectSize );
  ReleaseDC(NULL, hdc);

  return( (INT16)RectSize.cx );
}


INT16 GetWinFontHeight( UINT16 *string2, INT32 iFont )
{
  HWINFONT                *pWinFont;
  HDC                     hdc;
  SIZE                    RectSize;
	char			string[512];

  pWinFont = GetWinFont( iFont );

  if ( pWinFont == NULL )
  {
    return( 0 );
  }

#ifdef TAIWANESE
	Convert16BitStringTo8BitChineseBig5String( string, string2 );
#else
	sprintf( string, "%S", string2 );
#endif

  hdc = GetDC(NULL);
  SelectObject(hdc, pWinFont->hFont );
  GetTextExtentPoint32( hdc, string, strlen(string), &RectSize );
  ReleaseDC(NULL, hdc);

  return( (INT16)RectSize.cy );
}

UINT32	WinFont_mprintf( INT32 iFont, INT32 x, INT32 y, UINT16 *pFontString, ...)
{
  va_list                 argptr;
  wchar_t	                string[512];

	va_start(argptr, pFontString);       	// Set up variable argument pointer
	vswprintf(string, pFontString, argptr);	// process gprintf string (get output str)
	va_end(argptr);

  PrintWinFont( FontDestBuffer, iFont, x,  y, string );

  return( 1 );
}

int CALLBACK EnumFontFamProc( CONST LOGFONT *lplf,  CONST TEXTMETRIC *lptm,  DWORD dwType, LPARAM lpData )
{

  gfEnumSucceed = TRUE;

  return( TRUE );
}


int CALLBACK EnumFontFamExProc( ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )
{
	UINT8 szFontName[32];

	sprintf( szFontName, "%S", gzFontName );
	if( !strcmp( szFontName, lpelfe->elfFullName ) )
	{
		gfEnumSucceed = TRUE;
		memcpy( &gLogFont, &(lpelfe->elfLogFont), sizeof( LOGFONT ) );
	}

	return TRUE;
}


BOOLEAN DoesWinFontExistOnSystem( STR16 pTypeFaceName, INT32 iCharSet )
{
  HDC       hdc;
	char			string[512];
	LOGFONT LogFont;
  hdc = GetDC(NULL);

  gfEnumSucceed = FALSE;
	// Copy into 8-bit!
	sprintf( string, "%S", pTypeFaceName );

	memset( &LogFont, 0, sizeof( LOGFONT ) );
	LogFont.lfCharSet = iCharSet;
	lstrcpy( (LPSTR)&LogFont.lfFaceName, string );

	EnumFontFamiliesEx( hdc, &LogFont, EnumFontFamExProc, 0, 0 );

	ReleaseDC(NULL, hdc);

  return( gfEnumSucceed );
}
