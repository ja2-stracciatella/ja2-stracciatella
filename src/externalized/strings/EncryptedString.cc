#include "EncryptedString.h"
#include "GameRes.h"
#include "SGPFile.h"


ST::string LoadEncryptedData(ST::string& err_msg, STRING_ENC_TYPE encType, SGPFile* File, UINT32 seek_chars, UINT32 read_chars)
{
	File->seek(seek_chars * 2, FILE_SEEK_FROM_START);

	ST::utf16_buffer buf(read_chars, u'\0');
	File->read(buf.data(), sizeof(char16_t) * read_chars);

	buf[read_chars - 1] = u'\0';
	for (char16_t* i = buf.data(); *i != u'\0'; ++i)
	{
		/* "Decrypt" the ROT-1 "encrypted" data */
		char16_t c = (*i > 33 ? *i - 1 : *i);

		if(encType == SE_RUSSIAN)
		{
			/* The Russian data files are incorrectly encoded. The original texts seem to
			 * be encoded in CP1251, but then they were converted from CP1252 (!) to
			 * UTF-16 to store them in the data files. Undo this damage here. */
			if (0xC0 <= c && c <= 0xFF) c += 0x0350;
		}
		else
		{
			if(encType == SE_ENGLISH)
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

			else if(encType == SE_POLISH)
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

		*i = c;
	}
	return st_checked_buffer_to_string(err_msg, buf);
}

ST::string LoadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars)
{
	ST::string err_msg;
	ST::string str = LoadEncryptedData(err_msg, getStringEncType(), File, seek_chars, read_chars);
	if (!err_msg.empty())
	{
		SLOGW("LoadEncryptedString ? {} {}: {}", seek_chars, read_chars, err_msg);
	}
	return str;
}
