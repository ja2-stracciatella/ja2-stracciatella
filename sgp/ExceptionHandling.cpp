#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include <stdlib.h>
	#include <malloc.h>
	#include <stdio.h>
#endif

#include "ExceptionHandling.h"


#ifdef JA2
	#include "GameVersion.h"
#endif




//If we are to use exception handling
#ifdef ENABLE_EXCEPTION_HANDLING




const int NumCodeBytes = 16;	// Number of code bytes to record.
const int MaxStackDump = 2048;	// Maximum number of DWORDS in stack dumps.
const int StackColumns = 8;		// Number of columns in stack dump.

#define	ONEK			1024
#define	SIXTYFOURK		(64*ONEK)
#define	ONEM			(ONEK*ONEK)
#define	ONEG			(ONEK*ONEK*ONEK)








//ppp
void		ErrorLog(HWFILE LogFile, char* Format, ...);
STR			GetExceptionString( DWORD uiExceptionCode );
void		DisplayRegisters( HWFILE hFile, CONTEXT	*pContext );
BOOLEAN GetAndDisplayModuleAndSystemInfo( HWFILE hFile, CONTEXT *pContext );
BOOLEAN DisplayStack( HWFILE hFile, CONTEXT *pContext );
void		RecordModuleList(HWFILE hFile );
void		PrintTime(char *output, FILETIME TimeToPrint);
static	void ShowModuleInfo(HWFILE hFile, HINSTANCE ModuleHandle);



INT32 RecordExceptionInfo( EXCEPTION_POINTERS *pExceptInfo )
{
	EXCEPTION_RECORD Record;
	CONTEXT	Context;
	CHAR8		zFileName[512];
	CHAR8		zDate[512];
	CHAR8		zTime[512];
	HWFILE	hFile;
	CHAR8		zString[2048];
	SYSTEMTIME SysTime;
	CHAR8		zNewLine[] = "\r\n";


	//create local copies of the exception info
	memcpy( &Record, pExceptInfo->ExceptionRecord , sizeof( EXCEPTION_RECORD ) );
	memcpy( &Context, pExceptInfo->ContextRecord, sizeof( CONTEXT ) );


	//
	//	 Open a file to output the current state of the game
	//


	// Get the current time
	GetLocalTime( &SysTime );

	//create a date string
	sprintf( zDate, "%02d_%02d_%d", SysTime.wDay, SysTime.wMonth, SysTime.wYear );

	//create a time string
	sprintf( zTime, "%02d_%02d", SysTime.wHour, SysTime.wMinute );

	//create the crash file
	sprintf( zFileName, "Crash Report_%s___%s.txt", zDate, zTime );

	// create the save game file
	hFile = FileOpen( zFileName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
	if( !hFile )
	{
		FileClose( hFile );
		return( 0 );
	}

	//
	// Display the version number
	//
#ifdef JA2

	//Dispay Ja's version number
	ErrorLog( hFile, "%S: %s.  %S",zVersionLabel, czVersionNumber, zTrackingNumber );

	//Insert a new line
	ErrorLog( hFile, zNewLine );

	//Insert a new line
	ErrorLog( hFile, zNewLine );
#endif



	//
	// Write out the current state of the system
	//

	GetAndDisplayModuleAndSystemInfo( hFile, &Context );


	//Insert a new line
	ErrorLog( hFile, zNewLine );

	//Display the address of where the exception occured
	sprintf( zString, "Exception occured at address: 0x%08x\r\n", Record.ExceptionAddress );
	ErrorLog( hFile, zString );


	//if the exception was an access violation, display the offending address
	if( Record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && Record.NumberParameters != 0 )
	{
		if( Record.ExceptionInformation[0] != 0 )
		{
			//Display the address of where the access violation occured
			sprintf( zString, "\tWrite Access Violation at: 0x%08x\r\n", Record.ExceptionInformation[1] );
		}
		else
		{
			//Display the address of where the access violation occured
			sprintf( zString, "\tWrite Access Violation at: 0x%08x\r\n", Record.ExceptionInformation[1] );
		}

		ErrorLog( hFile, zString );
	}


	//Insert a new line
	ErrorLog( hFile, zNewLine );

	//Display the exception that caused this
	sprintf( zString, "Exact Error Message \r\n \"%s\"\r\n", GetExceptionString( Record.ExceptionCode ) );
	ErrorLog( hFile, zString );

	//Insert a new line
	ErrorLog( hFile, zNewLine );

	//Dispay if the code 'could' continue
	if( Record.ExceptionFlags != 0 )
		sprintf( zString, "%s\r\n", "The game 'can NOT' continue" );
	else
		sprintf( zString, "%s\r\n", "The game 'CAN' continue" );
	ErrorLog( hFile, zString );


	//Insert a new line
	ErrorLog( hFile, zNewLine );
	ErrorLog( hFile, zNewLine );


	//
	// Display the current context information
	//
	DisplayRegisters( hFile, &Context );

	ErrorLog( hFile, zNewLine );
	ErrorLog( hFile, zNewLine );

	//display the stack ( call stack + local variables )
	DisplayStack( hFile, &Context);

	//Display some spaces
	ErrorLog( hFile, zNewLine );
	ErrorLog( hFile, zNewLine );

	//Display all modules currently loaded
	RecordModuleList(hFile );


	//eee

	FileClose( hFile );

	return( EXCEPTION_EXECUTE_HANDLER );
}



void ErrorLog( HWFILE hFile, char* Format, ...)
{
	char buffer[2000];	// wvsprintf never prints more than one K.
	UINT32	uiNumBytesWritten=0;
	UINT32	uiStringWidth;
	va_list arglist;
	va_start( arglist, Format);
	wvsprintf(buffer, Format, arglist);
	va_end( arglist);

	//WriteFile(LogFile, buffer, lstrlen(buffer), &NumBytes, 0);


	uiStringWidth = lstrlen(buffer);

	//write out the string
	FileWrite( hFile, buffer, uiStringWidth, &uiNumBytesWritten );
	if( uiNumBytesWritten != uiStringWidth )
	{
		FileClose( hFile );
		return;
	}

}

STR	GetExceptionString( DWORD uiExceptionCode )
{
	switch( uiExceptionCode )
	{
		case EXCEPTION_ACCESS_VIOLATION:
			return( "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
 			return( "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
		case EXCEPTION_BREAKPOINT:
 			return( "A breakpoint was encountered.");
		case EXCEPTION_DATATYPE_MISALIGNMENT:
 			return( "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.");
		case EXCEPTION_FLT_DENORMAL_OPERAND:
 			return( "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
 			return( "The thread tried to divide a floating-point value by a floating-point divisor of zero.");
		case EXCEPTION_FLT_INEXACT_RESULT:
 			return( "The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
		case EXCEPTION_FLT_INVALID_OPERATION:
 			return( "This exception represents any floating-point exception not included in this list.");
		case EXCEPTION_FLT_OVERFLOW:
 			return( "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.");
		case EXCEPTION_FLT_STACK_CHECK:
 			return( "The stack overflowed or underflowed as the result of a floating-point operation.");
		case EXCEPTION_FLT_UNDERFLOW:
 			return( "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
		case EXCEPTION_ILLEGAL_INSTRUCTION:
 			return( "The thread tried to execute an invalid instruction.");
		case EXCEPTION_IN_PAGE_ERROR:
 			return( "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.");
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
 			return( "The thread tried to divide an integer value by an integer divisor of zero.");
		case EXCEPTION_INT_OVERFLOW:
 			return( "The result of an integer operation caused a carry out of the most significant bit of the result.");
		case EXCEPTION_INVALID_DISPOSITION:
 			return( "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
 			return( "The thread tried to continue execution after a noncontinuable exception occurred.");
		case EXCEPTION_PRIV_INSTRUCTION:
 			return( "The thread tried to execute an instruction whose operation is not allowed in the current machine mode."); 
		case EXCEPTION_SINGLE_STEP:
 			return( "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
		case EXCEPTION_STACK_OVERFLOW:
 			return( "The thread used up its stack.");

		default:
			return("Exception not in case ");
			break;
	}
}


void DisplayRegisters( HWFILE hFile, CONTEXT	*pContext )
{
	ErrorLog( hFile, "Registers:\r\n");
	ErrorLog( hFile, "\tEAX=%08x CS=%04x EIP=%08x EFLGS=%08x\r\n",
				pContext->Eax, pContext->SegCs, pContext->Eip, pContext->EFlags);
	ErrorLog( hFile, "\tEBX=%08x SS=%04x ESP=%08x EBP=%08x\r\n",
				pContext->Ebx, pContext->SegSs, pContext->Esp, pContext->Ebp);
	ErrorLog( hFile, "\tECX=%08x DS=%04x ESI=%08x FS=%04x\r\n",
				pContext->Ecx, pContext->SegDs, pContext->Esi, pContext->SegFs);
	ErrorLog( hFile, "\tEDX=%08x ES=%04x EDI=%08x GS=%04x\r\n",
				pContext->Edx, pContext->SegEs, pContext->Edi, pContext->SegGs);
//	ErrorLog( hFile, "Bytes at CS:EIP:\r\n");
}

BOOLEAN GetAndDisplayModuleAndSystemInfo( HWFILE hFile, CONTEXT *pContext )
{
	char zFileName[2048];
	char zString[2048];
	SYSTEM_INFO	SystemInfo;
	MEMORYSTATUS	MemInfo;
//	MEMORY_BASIC_INFORMATION	MemBasicInfo;
	size_t PageSize;
	size_t pageNum = 0;
	FILETIME	LastWriteTime;

	if( GetModuleFileName(0, zFileName, sizeof(zFileName) ) == 0)
	{
		return( FALSE );
	}

	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatus(&MemInfo);

	//Display the filename
	ErrorLog( hFile, "File:\r\n\t%s\r\n", zFileName);


	//Get the time the file was created
	if (GetFileTime(GetRealFileHandleFromFileManFileHandle( hFile ), 0, 0, &LastWriteTime))
	{
		PrintTime( zString, LastWriteTime);
	
		ErrorLog( hFile, "\tFile created on: %s\r\n", zString );
	}


	//Get cpu type and number
	GetSystemInfo(&SystemInfo);
	ErrorLog( hFile, "\t%d type %d processor.\r\n", SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType );

	//Get free ram
	ErrorLog( hFile, "\tTotal Physical Memory: %d Megs.\r\n", MemInfo.dwTotalPhys/(1024*1024) );


	PageSize = SystemInfo.dwPageSize;

	pageNum = 0;

/*
	//Get current instruction pointer
	if( VirtualQuery((void *)(pageNum * PageSize), &MemBasicInfo, sizeof(MemBasicInfo) ) )
	{
		if (MemBasicInfo.RegionSize > 0)
		{
			ErrorLog( hFile, "\r\n\r\nJagged is loaded into memory at: 0x%08d.\r\n", MemBasicInfo.AllocationBase );
		}
	}
*/

	ErrorLog( hFile, "Segment( CS:EIP ):\t%04x:%08x.\r\n", pContext->SegCs, pContext->Eip );

	return( TRUE );
}



//
// This code for this function is based ( stolen ) from Bruce Dawson's article in Game Developer Magazine Jan 99
//
BOOLEAN DisplayStack( HWFILE hFile, CONTEXT	*pContext  )
{
	int Count = 0;
	char	buffer[1000] = "";
	const int safetyzone = 50;
	char*	nearend = buffer + sizeof(buffer) - safetyzone;
	char*	output = buffer;

	// Time to print part or all of the stack to the error log. This allows
	// us to figure out the call stack, parameters, local variables, etc.
		ErrorLog( hFile, "Stack dump:\r\n" );

	__try
	{
		// Esp contains the bottom of the stack, or at least the bottom of
		// the currently used area.
		DWORD* pStack = (DWORD *)pContext->Esp;
		DWORD* pStackTop;
		__asm
		{
			// Load the top (highest address) of the stack from the
			// thread information block. It will be found there in
			// Win9x and Windows NT.
			mov	eax, fs:[4]
			mov pStackTop, eax
		}
		if (pStackTop > pStack + MaxStackDump)
			pStackTop = pStack + MaxStackDump;


		// Too many calls to WriteFile can take a long time, causing
		// confusing delays when programs crash. Therefore I implemented
		// simple buffering for the stack dumping code instead of calling
		// hprintf directly.

		while (pStack + 1 <= pStackTop)
		{
			char *Suffix = " ";

			if ((Count % StackColumns) == 0)
				output += wsprintf(output, "%08x: ", pStack);

			if ((++Count % StackColumns) == 0 || pStack + 2 > pStackTop)
				Suffix = "\r\n";
			output += wsprintf(output, "%08x%s", *pStack, Suffix);
			pStack++;
	
			// Check for when the buffer is almost full, and flush it to disk.
			if (output > nearend)
			{
				ErrorLog( hFile, "%s", buffer);
				buffer[0] = 0;
				output = buffer;
			}
		}
		// Print out any final characters from the cache.
		ErrorLog( hFile, "%s", buffer);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ErrorLog( hFile, "Exception encountered during stack dump.\r\n");
	}

	return( TRUE );
}


//
// This code for this function is ( stolen :) from Bruce Dawson's article in Game Developer Magazine Jan 99
//

// Print the specified FILETIME to output in a human readable format,
// without using the C run time.

void PrintTime(char *output, FILETIME TimeToPrint)
{
	WORD Date, Time;
	if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
				FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
	{
		// What a silly way to print out the file date/time. Oh well,
		// it works, and I'm not aware of a cleaner way to do it.
		wsprintf(output, "%02d/%02d/%d %02d:%02d:%02d",
					(Date / 32) & 15, Date & 31, (Date / 512) + 1980,
					(Time / 2048), (Time / 32) & 63, (Time & 31) * 2);
	}
	else
		output[0] = 0;
}







//
// This code for this function is ( stolen :) from Bruce Dawson's article in Game Developer Magazine Jan 99
//

// Scan memory looking for code modules (DLLs or EXEs). VirtualQuery is used
// to find all the blocks of address space that were reserved or committed,
// and ShowModuleInfo will display module information if they are code
// modules.


void RecordModuleList(HWFILE hFile )
{
	ErrorLog( hFile, "\r\n"
					 "Module list: names, addresses, sizes, time stamps "
			"and file times:\r\n");
	SYSTEM_INFO	SystemInfo;
	GetSystemInfo(&SystemInfo);
	const size_t PageSize = SystemInfo.dwPageSize;
	// Set NumPages to the number of pages in the 4GByte address space,
	// while being careful to avoid overflowing ints.
	const size_t NumPages = 4 * size_t(ONEG / PageSize);
	size_t pageNum = 0;
	void *LastAllocationBase = 0;
	while (pageNum < NumPages)
	{
		MEMORY_BASIC_INFORMATION	MemInfo;
		if (VirtualQuery((void *)(pageNum * PageSize), &MemInfo,
					sizeof(MemInfo)))
		{
			if (MemInfo.RegionSize > 0)
			{
				// Adjust the page number to skip over this block of memory.
				pageNum += MemInfo.RegionSize / PageSize;
				if (MemInfo.State == MEM_COMMIT && MemInfo.AllocationBase >
							LastAllocationBase)
				{
					// Look for new blocks of committed memory, and try
					// recording their module names - this will fail
					// gracefully if they aren't code modules.
					LastAllocationBase = MemInfo.AllocationBase;
					ShowModuleInfo(hFile, (HINSTANCE)LastAllocationBase);
				}
			}
			else
				pageNum += SIXTYFOURK / PageSize;
		}
		else
			pageNum += SIXTYFOURK / PageSize;
		// If VirtualQuery fails we advance by 64K because that is the
		// granularity of address space doled out by VirtualAlloc().
	}
}



//
// This code for this function is ( stolen :) from Bruce Dawson's article in Game Developer Magazine Jan 99
//
static void ShowModuleInfo(HWFILE hFile, HINSTANCE ModuleHandle)
{
	char ModName[MAX_PATH];
	__try
	{
		if (GetModuleFileName(ModuleHandle, ModName, sizeof(ModName)) > 0)
		{
			// If GetModuleFileName returns greater than zero then this must
			// be a valid code module address. Therefore we can try to walk
			// our way through its structures to find the link time stamp.
			IMAGE_DOS_HEADER *DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
		    if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic)
	    	    return;
			IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((char *)DosHeader
						+ DosHeader->e_lfanew);
		    if (IMAGE_NT_SIGNATURE != NTHeader->Signature)
	    	    return;
			// Open the code module file so that we can get its file date
			// and size.
			HANDLE ModuleFile = CreateFile(ModName, GENERIC_READ,
						FILE_SHARE_READ, 0, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL, 0);
			char TimeBuffer[100] = "";
			DWORD FileSize = 0;
			if (ModuleFile != INVALID_HANDLE_VALUE)
			{
				FileSize = GetFileSize(ModuleFile, 0);
				FILETIME	LastWriteTime;
				if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime))
				{
					wsprintf(TimeBuffer, " - file date is ");
					PrintTime(TimeBuffer + lstrlen(TimeBuffer), LastWriteTime);
				}
				CloseHandle(ModuleFile);
			}
			ErrorLog( hFile, "%-35s, loaded at 0x%08x - %7d bytes - %08x%s\r\n",
						ModName, ModuleHandle, FileSize,
						NTHeader->FileHeader.TimeDateStamp, TimeBuffer);
		}
	}
	// Handle any exceptions by continuing from this point.
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}


#endif