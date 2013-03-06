#include "Encrypted_File.h"
#include "FileMan.h"
#include "GameRes.h"
#include "MemMan.h"

void LoadEncryptedData(HWFILE const File, wchar_t* DestString, UINT32 const seek_chars, UINT32 const read_chars)
{
	FileSeek(File, seek_chars * 2, FILE_SEEK_FROM_START);

	UINT16 *Str = MALLOCN(UINT16, read_chars);
	FileRead(File, Str, sizeof(UINT16) * read_chars);

	Str[read_chars - 1] = '\0';
	for (const UINT16* i = Str; *i != '\0'; ++i)
	{
		/* "Decrypt" the ROT-1 "encrypted" data */
		wchar_t c = (*i > 33 ? *i - 1 : *i);

    if(isRussianVersion() || isRussianGoldVersion())
    {
      /* The Russian data files are incorrectly encoded. The original texts seem to
       * be encoded in CP1251, but then they were converted from CP1252 (!) to
       * UTF-16 to store them in the data files. Undo this damage here. */
      if (0xC0 <= c && c <= 0xFF) c += 0x0350;
    }
    else
    {
      if(isEnglishVersion())
      {
        /* The English data files are incorrectly encoded. The original texts seem
         * to be encoded in CP437, but then they were converted from CP1252 (!) to
         * UTF-16 to store them in the data files. Undo this damage here. This
         * problem only occurs for a few lines by Malice. */
        switch (c)
        {
        case 128: c = 0x00C7; break; // Ç
        case 130: c = 0x00E9; break; // é
        case 135: c = 0x00E7; break; // ç
        }
      }
      else if(isPolishVersion())
      {
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
      }

      /* Cyrillic texts (by Ivan Dolvich) in the non-Russian versions are encoded
       * in some wild manner. Undo this damage here. */
      if (0x044D <= c && c <= 0x0452) // cyrillic A to IE
      {
        c += -0x044D + 0x0410;
      }
      else if (c == 0x0453) // cyrillic IO
      {
        c = 0x0401;
      }
      else if (0x0454 <= c && c <= 0x0467) // cyrillic ZHE to SHCHA
      {
        c += -0x0454 + 0x0416;
      }
      else if (0x0468 <= c && c <= 0x046C) // cyrillic YERU to YA
      {
        c += -0x0468 + 0x042B;
      }
    }

		*DestString++ = c;
	}
	*DestString = L'\0';
  MemFree(Str);
}


void LoadEncryptedDataFromFile(char const* const Filename, wchar_t DestString[], UINT32 const seek_chars, UINT32 const read_chars)
{
	AutoSGPFile File(SmartFileOpenRO(Filename, true));
	LoadEncryptedData(File, DestString, seek_chars, read_chars);
}
