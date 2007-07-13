#include "Encrypted_File.h"
#include "FileMan.h"
#include "Debug.h"
#include "Language_Defines.h"


BOOLEAN LoadEncryptedData(HWFILE hFile, STR16 pDestString, UINT32 uiSeekFrom, UINT32 uiSeekAmount)
{
	UINT16 str[uiSeekAmount / 2];
	UINT16		i;

	if ( FileSeek( hFile, uiSeekFrom, FILE_SEEK_FROM_START ) == FALSE )
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedData: Failed FileSeek");
		return( FALSE );
	}

	if (!FileRead(hFile, str, sizeof(str)))
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedData: Failed FileRead");
		return( FALSE );
	}

	// Decrement, by 1, any value > 32
	for (i = 0; i < uiSeekAmount / 2 - 1 && str[i] != 0; i++)
	{
		wchar_t c = (str[i] > 33 ? str[i] - 1 : str[i]);
#if defined POLISH
		/* The Polish data files are incorrectly encoded. The original texts seem to
		 * be encoded in CP1250, but then they were converted from CP1252 (!) to
		 * UTF-16 to store them in the data files. Undo this damage here.
		 * Also the format code for centering texts differs. */
		switch (c)
		{
			case 143: c = 0x0179; break;
			case 163: c = 0x0141; break;
			case 165: c = 0x0104; break;
			case 175: c = 0x017B; break;
			case 179: c = 0x0142; break;
			case 182: c = 179;    break; // not a char, but a format code (centering)
			case 185: c = 0x0105; break;
			case 191: c = 0x017C; break;
			case 198: c = 0x0106; break;
			case 202: c = 0x0118; break;
			case 209: c = 0x0143; break;
			case 230: c = 0x0107; break;
			case 234: c = 0x0119; break;
			case 241: c = 0x0144; break;
			case 338: c = 0x015A; break;
			case 339: c = 0x015B; break;
			case 376: c = 0x017A; break;
		}
#elif defined RUSSIAN
		/* The Russian data files are incorrectly encoded. The original texts seem to
		 * be encoded in CP1251, but then they were converted from CP1252 (!) to
		 * UTF-16 to store them in the data files. Undo this damage here. */
		if (0xC0 <= c && c <= 0xFF) c += 0x0350;
#endif
		pDestString[i] = c;
	}
	pDestString[i] = L'\0';

	return(TRUE);
}


BOOLEAN LoadEncryptedDataFromFile(const char* Filename, wchar_t DestString[], UINT32 Seek, UINT32 ReadBytes)
{
	HWFILE File = FileOpen(Filename, FILE_ACCESS_READ);
	if (File == 0)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed to FileOpen");
		return FALSE;
	}

	BOOLEAN Ret = LoadEncryptedData(File, DestString, Seek, ReadBytes);
	FileClose(File);
	return Ret;
}
