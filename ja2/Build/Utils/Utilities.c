#ifdef PRECOMPILEDHEADERS
	#include "Utils_All.h"
#else
	#include "Types.h"
	#include <stdio.h>
	#include <Windows.h>
	#include "SGP.h"
	#include "time.h"
	#include "VObject.h"
	#include "FileMan.h"
	#include "Utilities.h"
	#include "Font_Control.h"
	#include "Overhead.h"
	#include "Overhead_Types.h"
	#include "WCheck.h"
	#include "Sys_Globals.h"
#endif


extern BOOLEAN GetCDromDriveLetter( STR8	pString );

#define		DATA_8_BIT_DIR	"8-Bit\\"

BOOLEAN PerformTimeLimitedCheck();


//#define	TIME_LIMITED_VERSION



void FilenameForBPP(STR pFilename, STR pDestination)
{
UINT8 Drive[128], Dir[128], Name[128], Ext[128];

	if(GETPIXELDEPTH()==16)
	{
		// no processing for 16 bit names
		strcpy(pDestination, pFilename);
	}
	else
	{
		_splitpath(pFilename, Drive, Dir, Name, Ext);

		strcat(Name, "_8");

		strcpy(pDestination, Drive);
		//strcat(pDestination, Dir);
		strcat(pDestination, DATA_8_BIT_DIR);
		strcat(pDestination, Name);
		strcat(pDestination, Ext);
	}

}

BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile )
{
  HWFILE     hFileHandle;
	BYTE			 bColHeader[ 8 ];
	UINT32		 cnt;

	//See if files exists, if not, return error
	if ( !FileExists( ColFile ) )
	{
		// Return FALSE w/ debug
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot find COL file");
		return( FALSE );
	}

	// Open and read in the file
  if ( ( hFileHandle = FileOpen( ColFile, FILE_ACCESS_READ, FALSE)) == 0)
	{
		// Return FALSE w/ debug
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Cannot open COL file");
		return( FALSE );
	}

	// Skip header
  FileRead( hFileHandle, bColHeader, sizeof( bColHeader ) , NULL);

	// Read in a palette entry at a time
	for ( cnt = 0; cnt < 256; cnt++ )
	{
	  FileRead( hFileHandle, &pPalette[ cnt ].peRed, sizeof( UINT8 ) , NULL);
	  FileRead( hFileHandle, &pPalette[ cnt ].peGreen, sizeof( UINT8 ) , NULL);
	  FileRead( hFileHandle, &pPalette[ cnt ].peBlue, sizeof( UINT8 ) , NULL);
	}

	// Close file
	FileClose( hFileHandle );

	return( TRUE );
}

BOOLEAN DisplayPaletteRep( PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface )
{
	UINT16										us16BPPColor;
	UINT32										cnt1;
	UINT8											ubSize, ubType;
	INT16											 sTLX, sTLY, sBRX, sBRY;
	UINT8											ubPaletteRep;

	// Create 16BPP Palette
	CHECKF( GetPaletteRepIndexFromID( aPalRep, &ubPaletteRep ) );

	SetFont( LARGEFONT1 );

	ubType = gpPalRep[ ubPaletteRep ].ubType;
	ubSize = gpPalRep[ ubPaletteRep ].ubPaletteSize;

	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		sTLX = ubXPos + (UINT16)( ( cnt1 % 16 ) * 20 );
		sTLY = ubYPos + (UINT16)( ( cnt1 / 16 ) * 20 );
		sBRX = sTLX + 20;
		sBRY = sTLY + 20;

		us16BPPColor = Get16BPPColor( FROMRGB( gpPalRep[ ubPaletteRep ].r[ cnt1 ], gpPalRep[ ubPaletteRep ].g[ cnt1 ], gpPalRep[ ubPaletteRep ].b[ cnt1 ] ) );

		ColorFillVideoSurfaceArea( uiDestSurface, sTLX, sTLY, sBRX, sBRY, us16BPPColor );

	}

	gprintf( ubXPos + ( 16 * 20 ), ubYPos, L"%S", gpPalRep[ ubPaletteRep ].ID );

	return( TRUE );
}


BOOLEAN	 WrapString( INT16 *pStr, INT16 *pStr2, UINT16 usWidth, INT32 uiFont )
{
	UINT32 Cur, uiLet, uiNewLet, uiHyphenLet;
	UINT16 *curletter,transletter;
	BOOLEAN	fLineSplit = FALSE;
	HVOBJECT	hFont;

	// CHECK FOR WRAP
	Cur=0;
	uiLet = 0;
	curletter=pStr;

	// GET FONT
	hFont = GetFontObject( uiFont );

	// LOOP FORWARDS AND COUNT
	while((*curletter)!=0)
	{
		transletter=GetIndex(*curletter);
		Cur+=GetWidth( hFont, transletter );

		if ( Cur > usWidth )
		{
			// We are here, loop backwards to find a space
			// Generate second string, and exit upon completion.
			uiHyphenLet = uiLet;	//Save the hyphen location as it won't change.
			uiNewLet = uiLet;
			while((*curletter)!=0)
			{
				if ( (*curletter) == 32 )
				{
					 // Split Line!
					 fLineSplit = TRUE;

					 pStr[ uiNewLet ] = (INT16)'\0';

					 wcscpy( pStr2, &(pStr[ uiNewLet + 1 ]) );
				}

				if ( fLineSplit )
					break;

				uiNewLet--;
				curletter--;

			}
			if( !fLineSplit)
			{
				//We completed the check for a space, but failed, so use the hyphen method.
				swprintf( pStr2, L"-%s", &(pStr[uiHyphenLet]) );
				pStr[uiHyphenLet] = (INT16)'/0';
				fLineSplit = TRUE;  //hyphen method
				break;
			}
		}

//		if ( fLineSplit )
//			break;

		uiLet++;
		curletter++;
	}

	return( fLineSplit );

}


BOOLEAN IfWinNT(void)
{
	OSVERSIONINFO OsVerInfo;

	OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&OsVerInfo);

	if ( OsVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return(TRUE);
	else
		return(FALSE);
}

BOOLEAN IfWin95(void)
{
	OSVERSIONINFO OsVerInfo;

	OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&OsVerInfo);

	if ( OsVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		return(TRUE);
	else
		return(FALSE);
}


void HandleLimitedNumExecutions( )
{
	// Get system directory
  HWFILE     hFileHandle;
	UINT8	ubSysDir[ 512 ];
	INT8	bNumRuns;

	GetSystemDirectory( ubSysDir, sizeof( ubSysDir ) );

	// Append filename
	strcat( ubSysDir, "\\winaese.dll" );

	// Open file and check # runs...
	if ( FileExists( ubSysDir ) )
	{
		// Open and read
		if ( ( hFileHandle = FileOpen( ubSysDir, FILE_ACCESS_READ, FALSE)) == 0)
		{
			return;
		}

		// Read value
		FileRead( hFileHandle, &bNumRuns, sizeof( bNumRuns ) , NULL);

		// Close file
		FileClose( hFileHandle );

		if ( bNumRuns <= 0 )
		{
			// Fail!
			SET_ERROR( "Error 1054: Cannot execute - contact Sir-Tech Software." );
			return;
		}

	}
	else
	{
		bNumRuns = 10;
	}

	// OK, decrement # runs...
	bNumRuns--;

	// Open and write
	if ( ( hFileHandle = FileOpen( ubSysDir, FILE_ACCESS_WRITE, FALSE)) == 0)
	{
		return;
	}

	// Write value
	FileWrite( hFileHandle, &bNumRuns, sizeof( bNumRuns ) , NULL);

	// Close file
	FileClose( hFileHandle );

}


SGPFILENAME	gCheckFilenames[] =
{
  "DATA\\INTRO.SLF",
  "DATA\\LOADSCREENS.SLF",
  "DATA\\MAPS.SLF",
	"DATA\\NPC_SPEECH.SLF",
	"DATA\\SPEECH.SLF",
};


UINT32 gCheckFileMinSizes[] =
{
  68000000,
  36000000,
  87000000,
  187000000,
  236000000
};

#if defined( JA2TESTVERSION  ) || defined( _DEBUG )
	#define NOCDCHECK
#endif

#if defined( RUSSIANGOLD )
	// CD check enabled
#else
	#define NOCDCHECK
#endif

BOOLEAN HandleJA2CDCheck( )
{
#ifdef	TIME_LIMITED_VERSION
	if( !PerformTimeLimitedCheck() )
	{
		return( FALSE );
	}
#endif


#ifdef NOCDCHECK

	return( TRUE );

#else
	BOOLEAN fFailed = FALSE;
	CHAR8		zCdLocation[ SGPFILENAME_LEN ];
	CHAR8		zCdFile[ SGPFILENAME_LEN ];
  INT32   cnt;
	HWFILE	hFile;

	// Check for a file on CD....
	if( GetCDromDriveLetter( zCdLocation ) )
	{
    for ( cnt = 0; cnt < 5; cnt++ )
    {
		  // OK, build filename
		  sprintf( zCdFile, "%s%s", zCdLocation, gCheckFilenames[ cnt ] );

			hFile = FileOpen( zCdFile, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );

		  // Check if it exists...
		  if ( !hFile )
		  {
			  fFailed = TRUE;
				FileClose( hFile );
        break;
		  }

      // Check min size
//#ifndef GERMAN
//      if ( FileGetSize( hFile ) < gCheckFileMinSizes[ cnt ] )
//      {
//			  fFailed = TRUE;
//				FileClose( hFile );
//        break;
//      }
//#endif

			FileClose( hFile );

    }
	}
  else
  {
		fFailed = TRUE;
  }

	if ( fFailed )
	{
		CHAR8	zErrorMessage[256];

		sprintf( zErrorMessage, "%S", gzLateLocalizedString[ 56 ] );
		// Pop up message boc and get answer....
		if ( MessageBox( NULL, zErrorMessage, "Jagged Alliance 2", MB_OK ) == IDOK )
		{
			return( FALSE );
		}
	}

  return( TRUE );

#endif

}


BOOLEAN HandleJA2CDCheckTwo( )
{
#ifdef NOCDCHECK

	return( TRUE );

#else
	BOOLEAN fFailed = TRUE;
	CHAR8		zCdLocation[ SGPFILENAME_LEN ];
	CHAR8		zCdFile[ SGPFILENAME_LEN ];

	// Check for a file on CD....
	if( GetCDromDriveLetter( zCdLocation ) )
	{
		// OK, build filename
		sprintf( zCdFile, "%s%s", zCdLocation, gCheckFilenames[ Random( 2 ) ] );

		// Check if it exists...
		if ( FileExists( zCdFile ) )
		{
			fFailed = FALSE;
		}
	}

	if ( fFailed )
	{
		CHAR8	zErrorMessage[256];

		sprintf( zErrorMessage, "%S", gzLateLocalizedString[ 56 ] );
		// Pop up message boc and get answer....
		if ( MessageBox( NULL, zErrorMessage, "Jagged Alliance 2", MB_OK ) == IDOK )
		{
			return( FALSE );
		}
	}
	else
	{
		return( TRUE );
	}

#endif

	return( FALSE );
}


BOOLEAN PerformTimeLimitedCheck()
{
#ifndef TIME_LIMITED_VERSION
		return( TRUE );

#else
	SYSTEMTIME sSystemTime;

	GetSystemTime( &sSystemTime );


	//if according to the system clock, we are past july 1999, quit the game
	if( sSystemTime.wYear > 1999 || sSystemTime.wMonth > 7 )
	{
		//spit out an error message
		MessageBox( NULL, "This time limited version of Jagged Alliance 2 has expired.", "Ja2 Error!", MB_OK  );
		return( FALSE );
	}

	return( TRUE );
#endif
}

BOOLEAN DoJA2FilesExistsOnDrive( CHAR8 *zCdLocation )
{
	BOOLEAN fFailed = FALSE;
	CHAR8		zCdFile[ SGPFILENAME_LEN ];
  INT32   cnt;
	HWFILE	hFile;

  for ( cnt = 0; cnt < 4; cnt++ )
  {
		// OK, build filename
		sprintf( zCdFile, "%s%s", zCdLocation, gCheckFilenames[ cnt ] );

		hFile = FileOpen( zCdFile, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );

		// Check if it exists...
		if ( !hFile )
		{
			fFailed = TRUE;
			FileClose( hFile );
      break;
		}
		FileClose( hFile );
	}

	return( !fFailed );
}
