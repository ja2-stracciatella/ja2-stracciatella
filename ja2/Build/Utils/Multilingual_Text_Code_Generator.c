/*
MULTILINGUAL TEXT CODE GENERATOR
This code generator is used to conveniently compare the english master text file with another foreign language
such as German and verify that the appropriate language file is in perfect synch with the English.  Verifying
that all of the strings have the correct order of printf format specifiers and the precise number.  If
different, the errors are recorded via comments proceeding the string in question in the new file.  For
simplicity, the German language will be used in examples throughout this documention.  The comments will be
specially marked with "CONFLICT#xxx:  error message" which can be searched for.  The comment will report
the format specifiers used in the english version.

ASSUMPTIONS
- No functions exist in any of the master files
- Users don't use single strings using:
		STR16 str[] = L"Single String";
	Instead use:
		STR16 str[] =
		{
			L"Single String";
		}
- Users don't use comments containing the { character later followed by the L" token.  The code generator
  will mistaken that for a string.
- Users don't use nested braces (2D text arrays)



AUTHOR:  Kris Morness
CREATED:	Feb 16, 1999
*/

#ifdef _DEBUG

#include <stdio.h>
#include "types.h"
#include "Language Defines.h"
#include "debug.h"
#include "Fileman.h"

//Currently in JA2's _EnglishText, these tokens make up all of the
//format specifiers that are actually used.  Feel free to add more,
//but make sure you change NUM_TOKENS accordingly.  These tokens assume
//the previous character is a % character.
UINT8 SupportedTokens[] =
{
	'd',
	'c',
	's',
	'S',
	'%',
};
#define NUM_TOKENS	5

enum
{
	//look for { character followed by L" before } to upgrade to INSIDE_STRING
	OUTSIDE_STRING_ARRAY,

	//look for } character to downgrade to OUTSIDE_STRING_ARRAY
	//look for L" characters to upgrade to INSIDE_STRING
	INSIDE_STRING_ARRAY,

	//look for " character to downgrade to INSIDE_STRING_ARRAY
	INSIDE_STRING,
};

//Specifies where the master english file is located relative to the exe directory
#define LCG_WORKINGDIRECTORY				"build\\utils"
#define LCG_ENGLISHMASTERFILE				"_EnglishText.c"

//The commandline argument (add different one for each language supported
//***Only one can exist at a time and it is controlled by Language Defines.h )
#define LCG_COMMANDLINEARGUMENT			"-GERMAN"
#define LCG_FOREIGNMASTERFILE				"_GermanText.c"
#define LCG_FOREIGNNEWFILE					"_NewGermanText.c"

//Given a file pointer, searches for the next DB string
UINT32 CountDoubleByteStringsInFile( UINT8 *filename );


//One function does it all.  First looks for the cmd line arg, and if it matches
//the above define, searches for the files, and processes them automatically.
BOOLEAN ProcessIfMultilingualCmdLineArgDetected( UINT8 *str )
{
	STRING512			ExecDir;
	STRING512			CurrDir;
	STRING512			Dir;
	UINT32 uiEnglishStrings, uiForeignStrings;

	//check if command line argument matches the LCG's
	if( strcmp( str, LCG_COMMANDLINEARGUMENT ) )
	{ //string is different, so return
		return FALSE;
	}

	//Record the exe directory
	GetExecutableDirectory( ExecDir );
	//Record the curr directory used (we will restore before leaving)
	GetFileManCurrentDirectory( CurrDir );

	//Build the working directory name
	sprintf( Dir, "%s\\%s", ExecDir, LCG_WORKINGDIRECTORY );

	//Set the working directory
	if( !SetFileManCurrentDirectory( Dir ) )
	{ //We failed meaning the directory name is incorrect or non-existant
		AssertMsg( 0, "Failed to set directory location while attempting to activate multilingual text code generator." );
		return FALSE;
	}

	//verify that all files exist
	if( !FileExists( LCG_ENGLISHMASTERFILE ) )
	{
		AssertMsg( 0, "Failed to find master english file while attempting to activate multilingual text code generator." );
		return FALSE;
	}
	if( !FileExists( LCG_FOREIGNMASTERFILE ) )
	{
		AssertMsg( 0, "Failed to find master foreign file while attempting to activate multilingual text code generator." );
		return FALSE;
	}

	//ALL PRELIMINARY CHECKS HAVE SUCCEEDED.
	//Begin file preparation checks...

	//STEP1:  Read the English master file and count the number of DB strings that exist
	uiEnglishStrings = CountDoubleByteStringsInFile( LCG_ENGLISHMASTERFILE );

	//STEP2:  Read the Foreigh master file and count the number of DB strings that exist
	uiForeignStrings = CountDoubleByteStringsInFile( LCG_FOREIGNMASTERFILE );

	//Make sure they match, otherwise, we can't continue.
	if( uiEnglishStrings != uiForeignStrings )
	{
		AssertMsg( 0, String( "Mismatch during LCG preparation:  English DB strings: %d, Foreign DB strings: %d",
			uiEnglishStrings, uiForeignStrings ) );
		return FALSE;
	}

	//Mission complete!  Reset the previously known directory, and return TRUE;
	SetFileManCurrentDirectory( CurrDir );
	return TRUE;
}

UINT32 CountDoubleByteStringsInFile( UINT8 *filename )
{
	FILE *fp = NULL;
	UINT32 uiNumStrings = 0;

	//open file
	fp = fopen( filename, "r" );
	if( !fp )
	{
		return 0;
	}

	fclose( fp );
	return uiNumStrings;
}

#endif //_DEBUG
