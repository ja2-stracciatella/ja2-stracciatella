#include "TranslationTable.h"

#include <algorithm>
#include <iterator>
#include <string.h>

CTranslationTable::CTranslationTable()
{
	std::fill(std::begin(m_table), std::end(m_table), 0);
}

/** Translation table for the English version of the game. */
class CTranslationTableEnglish : public CTranslationTable
{
public:
	/** Constructor. */
	CTranslationTableEnglish()
	{
		m_table[L'A'] =   0;
		m_table[L'B'] =   1;
		m_table[L'C'] =   2;
		m_table[L'D'] =   3;
		m_table[L'E'] =   4;
		m_table[L'F'] =   5;
		m_table[L'G'] =   6;
		m_table[L'H'] =   7;
		m_table[L'I'] =   8;
		m_table[L'J'] =   9;
		m_table[L'K'] =  10;
		m_table[L'L'] =  11;
		m_table[L'M'] =  12;
		m_table[L'N'] =  13;
		m_table[L'O'] =  14;
		m_table[L'P'] =  15;
		m_table[L'Q'] =  16;
		m_table[L'R'] =  17;
		m_table[L'S'] =  18;
		m_table[L'T'] =  19;
		m_table[L'U'] =  20;
		m_table[L'V'] =  21;
		m_table[L'W'] =  22;
		m_table[L'X'] =  23;
		m_table[L'Y'] =  24;
		m_table[L'Z'] =  25;

		m_table[L'a'] =  26;
		m_table[L'b'] =  27;
		m_table[L'c'] =  28;
		m_table[L'd'] =  29;
		m_table[L'e'] =  30;
		m_table[L'f'] =  31;
		m_table[L'g'] =  32;
		m_table[L'h'] =  33;
		m_table[L'i'] =  34;
		m_table[L'j'] =  35;
		m_table[L'k'] =  36;
		m_table[L'l'] =  37;
		m_table[L'm'] =  38;
		m_table[L'n'] =  39;
		m_table[L'o'] =  40;
		m_table[L'p'] =  41;
		m_table[L'q'] =  42;
		m_table[L'r'] =  43;
		m_table[L's'] =  44;
		m_table[L't'] =  45;
		m_table[L'u'] =  46;
		m_table[L'v'] =  47;
		m_table[L'w'] =  48;
		m_table[L'x'] =  49;
		m_table[L'y'] =  50;
		m_table[L'z'] =  51;

		m_table[L'0'] =  52;
		m_table[L'1'] =  53;
		m_table[L'2'] =  54;
		m_table[L'3'] =  55;
		m_table[L'4'] =  56;
		m_table[L'5'] =  57;
		m_table[L'6'] =  58;
		m_table[L'7'] =  59;
		m_table[L'8'] =  60;
		m_table[L'9'] =  61;

		m_table[L'!'] =  62;
		m_table[L'@'] =  63;
		m_table[L'#'] =  64;
		m_table[L'$'] =  65;
		m_table[L'%'] =  66;
		m_table[L'^'] =  67;
		m_table[L'&'] =  68;
		m_table[L'*'] =  69;
		m_table[L'('] =  70;
		m_table[L')'] =  71;
		m_table[L'-'] =  72;
		m_table[L'_'] =  73;
		m_table[L'+'] =  74;
		m_table[L'='] =  75;
		m_table[L'|'] =  76;
		m_table[L'\\'] =  77;
		m_table[L'{'] =  78;
		m_table[L'}'] =  79;
		m_table[L'['] =  80;
		m_table[L']'] =  81;
		m_table[L':'] =  82;
		m_table[L';'] =  83;
		m_table[L'"'] =  84;
		m_table[L'\''] =  85;
		m_table[L'<'] =  86;
		m_table[L'>'] =  87;
		m_table[L','] =  88;
		m_table[L'.'] =  89;
		m_table[L'?'] =  90;
		m_table[L'/'] =  91;
		m_table[L' '] =  92;
		m_table[196] =  93; // A umlaut
		m_table[214] =  94; // O umlaut
		m_table[220] =  95; // U umlaut
		m_table[228] =  96; // a umlaut
		m_table[246] =  97; // o umlaut
		m_table[252] =  98; // u umlaut
		m_table[223] =  99; // sharp s

		// duplicate 196, // Ä
		m_table[192] = 133; // À
		m_table[193] = 134; // Á
		m_table[194] = 135; // Â
		m_table[199] = 136; // Ç
		m_table[203] = 137; // Ë
		m_table[200] = 138; // È
		m_table[201] = 139; // É        140
		m_table[202] = 140; // Ê
		m_table[207] = 141; // Ï
		// duplicate 214, // Ö
		m_table[210] = 143; // Ò
		m_table[211] = 144; // Ó
		m_table[212] = 145; // Ô
		// duplicate 220, // Ü
		m_table[217] = 147; // Ù
		m_table[218] = 148; // Ú
		m_table[219] = 149; // Û        150
		// duplicate 228, // ä
		m_table[224] = 151; // à
		m_table[225] = 152; // á
		m_table[226] = 153; // â
		m_table[231] = 154; // ç
		m_table[235] = 155; // ë
		m_table[232] = 156; // è
		m_table[233] = 157; // é
		m_table[234] = 158; // ê
		m_table[239] = 159; // ï        160
		// duplicate 246, // ö
		m_table[242] = 161; // ò
		m_table[243] = 162; // ó
		m_table[244] = 163; // ô
		// duplicate 252, // ü
		m_table[249] = 165; // ù
		m_table[250] = 166; // ú
		m_table[251] = 167; // û

		m_table[0x00CC] = 168; // Ì
		m_table[0x00EC] = 169; // ì
		m_table[0x0104] = 170; // Ą
		m_table[0x0106] = 171; // Ć
		m_table[0x0118] = 172; // Ę
		m_table[0x0141] = 173; // Ł
		m_table[0x0143] = 174; // Ń
		//[0x00D3] = 175, // Ó (duplicate)
		m_table[0x015A] = 176; // Ś
		m_table[0x017B] = 177; // Ż
		m_table[0x0179] = 178; // Ź
		m_table[0x0105] = 179; // ą
		m_table[0x0107] = 180; // ć
		m_table[0x0119] = 181; // ę
		m_table[0x0142] = 182; // ł
		m_table[0x0144] = 183; // ń
		//[0x00F3] = 184, // ó (duplicate)
		m_table[0x015B] = 185; // ś
		m_table[0x017C] = 186; // ż
		m_table[0x017A] = 187; // ź

		m_table[0x0410] = 100; // cyrillic A
		m_table[0x0411] = 101; // cyrillic BE
		m_table[0x0412] = 102; // cyrillic VE
		m_table[0x0413] = 103; // cyrillic GHE
		m_table[0x0414] = 104; // cyrillic DE
		m_table[0x0415] = 105; // cyrillic IE
		m_table[0x0401] = 106; // cyrillic IO
		m_table[0x0416] = 107; // cyrillic ZHE
		m_table[0x0417] = 108; // cyrillic ZE
		m_table[0x0418] = 109; // cyrillic I
		m_table[0x0419] = 110; // cyrillic short I
		m_table[0x041A] = 111; // cyrillic KA
		m_table[0x041B] = 112; // cyrillic EL
		m_table[0x041C] = 113; // cyrillic EM
		m_table[0x041D] = 114; // cyrillic EN
		m_table[0x041E] = 115; // cyrillic O
		m_table[0x041F] = 116; // cyrillic PE
		m_table[0x0420] = 117; // cyrillic ER
		m_table[0x0421] = 118; // cyrillic ES
		m_table[0x0422] = 119; // cyrillic TE
		m_table[0x0423] = 120; // cyrillic U
		m_table[0x0424] = 121; // cyrillic EF
		m_table[0x0425] = 122; // cyrillic HA
		m_table[0x0426] = 123; // cyrillic TSE
		m_table[0x0427] = 124; // cyrillic CHE
		m_table[0x0428] = 125; // cyrillic SHA
		m_table[0x0429] = 126; // cyrillic SHCHA
		m_table[0x042A] = 128; // cyrillic capital hard sign, glyph is missing, mapped to soft sign
		m_table[0x042B] = 127; // cyrillic YERU
		m_table[0x042C] = 128; // cyrillic capital soft sign
		m_table[0x042D] = 129; // cyrillic E
		m_table[0x042E] = 130; // cyrillic YU
		m_table[0x042F] = 131; // cyrillic YA

		/* There are no lowercase cyrillic glyphs in the fonts (at least neither in
		 * the german nor the polish datafiles), so reuse the uppercase cyrillic
		 * glyphs. */
		m_table[0x0430] = 100; // cyrillic a
		m_table[0x0431] = 101; // cyrillic be
		m_table[0x0432] = 102; // cyrillic ve
		m_table[0x0433] = 103; // cyrillic ghe
		m_table[0x0434] = 104; // cyrillic de
		m_table[0x0435] = 105; // cyrillic ie
		m_table[0x0451] = 106; // cyrillic io
		m_table[0x0436] = 107; // cyrillic zhe
		m_table[0x0437] = 108; // cyrillic ze
		m_table[0x0438] = 109; // cyrillic i
		m_table[0x0439] = 110; // cyrillic short i
		m_table[0x043A] = 111; // cyrillic ka
		m_table[0x043B] = 112; // cyrillic el
		m_table[0x043C] = 113; // cyrillic em
		m_table[0x043D] = 114; // cyrillic en
		m_table[0x043E] = 115; // cyrillic o
		m_table[0x043F] = 116; // cyrillic pe
		m_table[0x0440] = 117; // cyrillic er
		m_table[0x0441] = 118; // cyrillic es
		m_table[0x0442] = 119; // cyrillic te
		m_table[0x0443] = 120; // cyrillic u
		m_table[0x0444] = 121; // cyrillic ef
		m_table[0x0445] = 122; // cyrillic ha
		m_table[0x0446] = 123; // cyrillic tse
		m_table[0x0447] = 124; // cyrillic che
		m_table[0x0448] = 125; // cyrillic sha
		m_table[0x0449] = 126; // cyrillic shcha
		m_table[0x044A] = 128; // cyrillic lowercase hard sign, glyph is missing, mapped to soft sign
		m_table[0x044B] = 127; // cyrillic yeru
		m_table[0x044C] = 128; // cyrillic lowercase soft sign
		m_table[0x044D] = 129; // cyrillic e
		m_table[0x044E] = 130; // cyrillic yu
		m_table[0x044F] = 131;  // cyrillic ya
	}
};


/** Translation table for the French version of the game. */
class CTranslationTableFrench : public CTranslationTable
{
public:
	/** Constructor. */
	CTranslationTableFrench()
	{
		m_table[L'A'] =   0;
		m_table[L'B'] =   1;
		m_table[L'C'] =   2;
		m_table[L'D'] =   3;
		m_table[L'E'] =   4;
		m_table[L'F'] =   5;
		m_table[L'G'] =   6;
		m_table[L'H'] =   7;
		m_table[L'I'] =   8;
		m_table[L'J'] =   9;
		m_table[L'K'] =  10;
		m_table[L'L'] =  11;
		m_table[L'M'] =  12;
		m_table[L'N'] =  13;
		m_table[L'O'] =  14;
		m_table[L'P'] =  15;
		m_table[L'Q'] =  16;
		m_table[L'R'] =  17;
		m_table[L'S'] =  18;
		m_table[L'T'] =  19;
		m_table[L'U'] =  20;
		m_table[L'V'] =  21;
		m_table[L'W'] =  22;
		m_table[L'X'] =  23;
		m_table[L'Y'] =  24;
		m_table[L'Z'] =  25;

		m_table[L'a'] =  26;
		m_table[L'b'] =  27;
		m_table[L'c'] =  28;
		m_table[L'd'] =  29;
		m_table[L'e'] =  30;
		m_table[L'f'] =  31;
		m_table[L'g'] =  32;
		m_table[L'h'] =  33;
		m_table[L'i'] =  34;
		m_table[L'j'] =  35;
		m_table[L'k'] =  36;
		m_table[L'l'] =  37;
		m_table[L'm'] =  38;
		m_table[L'n'] =  39;
		m_table[L'o'] =  40;
		m_table[L'p'] =  41;
		m_table[L'q'] =  42;
		m_table[L'r'] =  43;
		m_table[L's'] =  44;
		m_table[L't'] =  45;
		m_table[L'u'] =  46;
		m_table[L'v'] =  47;
		m_table[L'w'] =  48;
		m_table[L'x'] =  49;
		m_table[L'y'] =  50;
		m_table[L'z'] =  51;

		m_table[L'0'] =  52;
		m_table[L'1'] =  53;
		m_table[L'2'] =  54;
		m_table[L'3'] =  55;
		m_table[L'4'] =  56;
		m_table[L'5'] =  57;
		m_table[L'6'] =  58;
		m_table[L'7'] =  59;
		m_table[L'8'] =  60;
		m_table[L'9'] =  61;

		m_table[L'!'] =  62;
		m_table[L'@'] =  63;
		m_table[L'#'] =  64;
		m_table[L'$'] =  65;
		m_table[L'%'] =  66;
		m_table[L'^'] =  67;
		m_table[L'&'] =  68;
		m_table[L'*'] =  69;
		m_table[L'('] =  70;
		m_table[L')'] =  71;
		m_table[L'-'] =  72;
		m_table[L'_'] =  73;
		m_table[L'+'] =  74;
		m_table[L'='] =  75;
		m_table[L'|'] =  76;
		m_table[L'\\'] =  77;
		m_table[L'{'] =  78;
		m_table[L'}'] =  79;
		m_table[L'['] =  80;
		m_table[L']'] =  81;
		m_table[L':'] =  82;
		m_table[L';'] =  83;
		m_table[L'"'] =  84;
		m_table[L'\''] =  85;
		m_table[L'<'] =  86;
		m_table[L'>'] =  87;
		m_table[L','] =  88;
		m_table[L'.'] =  89;
		m_table[L'?'] =  90;
		m_table[L'/'] =  91;
		m_table[L' '] =  92;
		m_table[196] =  93; // A umlaut
		m_table[214] =  94; // O umlaut
		m_table[220] =  95; // U umlaut
		m_table[228] =  96; // a umlaut
		m_table[246] =  97; // o umlaut
		m_table[252] =  98; // u umlaut
		m_table[223] =  99; // sharp s

		// duplicate 196, // Ä
		m_table[192] = 133; // À
		m_table[193] = 134; // Á
		m_table[194] = 135; // Â
		m_table[199] = 136; // Ç
		m_table[203] = 137; // Ë
		m_table[200] = 138; // È
		m_table[201] = 139; // É        140
		m_table[202] = 140; // Ê
		m_table[207] = 141; // Ï
		// duplicate 214, // Ö
		m_table[210] = 143; // Ò
		m_table[211] = 144; // Ó
		m_table[212] = 145; // Ô
		// duplicate 220, // Ü
		m_table[217] = 147; // Ù
		m_table[218] = 148; // Ú
		m_table[219] = 149; // Û        150
		// duplicate 228, // ä
		m_table[224] = 151; // à
		m_table[225] = 152; // á
		m_table[226] = 153; // â
		m_table[231] = 154; // ç
		m_table[235] = 155; // ë
		m_table[232] = 156; // è
		m_table[233] = 157; // é
		m_table[234] = 158; // ê
		m_table[239] = 159; // ï        160
		// duplicate 246, // ö
		m_table[242] = 161; // ò
		m_table[243] = 162; // ó
		m_table[244] = 163; // ô
		// duplicate 252, // ü
		m_table[249] = 165; // ù
		m_table[250] = 166; // ú
		m_table[251] = 167; // û

		m_table[0x00CC] = 168; // Ì
		m_table[0x00CE] = 169; // Î
		m_table[0x00EC] = 170; // ì
		m_table[0x00EE] = 171; // î
		m_table[0x0104] = 172; // Ą
		m_table[0x0106] = 173; // Ć
		m_table[0x0118] = 174; // Ę
		m_table[0x0141] = 175; // Ł
		m_table[0x0143] = 176; // Ń
		//[0x00D3] = 177, // Ó (duplicate)
		m_table[0x015A] = 178; // Ś
		m_table[0x017B] = 179; // Ż
		m_table[0x0179] = 180; // Ź
		m_table[0x0105] = 181; // ą
		m_table[0x0107] = 182; // ć
		m_table[0x0119] = 183; // ę
		m_table[0x0142] = 184; // ł
		m_table[0x0144] = 185; // ń
		//[0x00F3] = 186, // ó (duplicate)
		m_table[0x015B] = 187; // ś
		m_table[0x017C] = 188; // ż
		m_table[0x017A] = 189; // ź

		m_table[0x0410] = 100; // cyrillic A
		m_table[0x0411] = 101; // cyrillic BE
		m_table[0x0412] = 102; // cyrillic VE
		m_table[0x0413] = 103; // cyrillic GHE
		m_table[0x0414] = 104; // cyrillic DE
		m_table[0x0415] = 105; // cyrillic IE
		m_table[0x0401] = 106; // cyrillic IO
		m_table[0x0416] = 107; // cyrillic ZHE
		m_table[0x0417] = 108; // cyrillic ZE
		m_table[0x0418] = 109; // cyrillic I
		m_table[0x0419] = 110; // cyrillic short I
		m_table[0x041A] = 111; // cyrillic KA
		m_table[0x041B] = 112; // cyrillic EL
		m_table[0x041C] = 113; // cyrillic EM
		m_table[0x041D] = 114; // cyrillic EN
		m_table[0x041E] = 115; // cyrillic O
		m_table[0x041F] = 116; // cyrillic PE
		m_table[0x0420] = 117; // cyrillic ER
		m_table[0x0421] = 118; // cyrillic ES
		m_table[0x0422] = 119; // cyrillic TE
		m_table[0x0423] = 120; // cyrillic U
		m_table[0x0424] = 121; // cyrillic EF
		m_table[0x0425] = 122; // cyrillic HA
		m_table[0x0426] = 123; // cyrillic TSE
		m_table[0x0427] = 124; // cyrillic CHE
		m_table[0x0428] = 125; // cyrillic SHA
		m_table[0x0429] = 126; // cyrillic SHCHA
		m_table[0x042A] = 128; // cyrillic capital hard sign, glyph is missing, mapped to soft sign
		m_table[0x042B] = 127; // cyrillic YERU
		m_table[0x042C] = 128; // cyrillic capital soft sign
		m_table[0x042D] = 129; // cyrillic E
		m_table[0x042E] = 130; // cyrillic YU
		m_table[0x042F] = 131; // cyrillic YA

		/* There are no lowercase cyrillic glyphs in the fonts (at least neither in
		 * the german nor the polish datafiles), so reuse the uppercase cyrillic
		 * glyphs. */
		m_table[0x0430] = 100; // cyrillic a
		m_table[0x0431] = 101; // cyrillic be
		m_table[0x0432] = 102; // cyrillic ve
		m_table[0x0433] = 103; // cyrillic ghe
		m_table[0x0434] = 104; // cyrillic de
		m_table[0x0435] = 105; // cyrillic ie
		m_table[0x0451] = 106; // cyrillic io
		m_table[0x0436] = 107; // cyrillic zhe
		m_table[0x0437] = 108; // cyrillic ze
		m_table[0x0438] = 109; // cyrillic i
		m_table[0x0439] = 110; // cyrillic short i
		m_table[0x043A] = 111; // cyrillic ka
		m_table[0x043B] = 112; // cyrillic el
		m_table[0x043C] = 113; // cyrillic em
		m_table[0x043D] = 114; // cyrillic en
		m_table[0x043E] = 115; // cyrillic o
		m_table[0x043F] = 116; // cyrillic pe
		m_table[0x0440] = 117; // cyrillic er
		m_table[0x0441] = 118; // cyrillic es
		m_table[0x0442] = 119; // cyrillic te
		m_table[0x0443] = 120; // cyrillic u
		m_table[0x0444] = 121; // cyrillic ef
		m_table[0x0445] = 122; // cyrillic ha
		m_table[0x0446] = 123; // cyrillic tse
		m_table[0x0447] = 124; // cyrillic che
		m_table[0x0448] = 125; // cyrillic sha
		m_table[0x0449] = 126; // cyrillic shcha
		m_table[0x044A] = 128; // cyrillic lowercase hard sign, glyph is missing, mapped to soft sign
		m_table[0x044B] = 127; // cyrillic yeru
		m_table[0x044C] = 128; // cyrillic lowercase soft sign
		m_table[0x044D] = 129; // cyrillic e
		m_table[0x044E] = 130; // cyrillic yu
		m_table[0x044F] = 131;  // cyrillic ya
	}
};


/** Translation table for the Russian version of the game. */
class CTranslationTableRussian : public CTranslationTable
{
public:
	/** Constructor. */
	CTranslationTableRussian()
	{
		/* This is the table for the BUKA Agonia Vlasti release. The glyph set is,
		 * except for two gaps (0x00-0x1F, 0x80-0xBF), identical to CP1251. */
		m_table[L' '] =   0;
		m_table[L'!'] =   1;
		m_table[L'"'] =   2;
		m_table[L'#'] =   3;
		m_table[L'$'] =   4;
		m_table[L'%'] =   5;
		m_table[L'&'] =   6;
		m_table[L'\''] =   7;
		m_table[L'('] =   8;
		m_table[L')'] =   9;
		m_table[L'*'] =  10;
		m_table[L'+'] =  11;
		m_table[L','] =  12;
		m_table[L'-'] =  13;
		m_table[L'.'] =  14;
		m_table[L'/'] =  15;
		m_table[L'0'] =  16;
		m_table[L'1'] =  17;
		m_table[L'2'] =  18;
		m_table[L'3'] =  19;
		m_table[L'4'] =  20;
		m_table[L'5'] =  21;
		m_table[L'6'] =  22;
		m_table[L'7'] =  23;
		m_table[L'8'] =  24;
		m_table[L'9'] =  25;
		m_table[L':'] =  26;
		m_table[L';'] =  27;
		m_table[L'<'] =  28;
		m_table[L'='] =  29;
		m_table[L'>'] =  30;
		m_table[L'?'] =  31;
		m_table[L'@'] =  32;
		m_table[L'A'] =  33;
		m_table[L'B'] =  34;
		m_table[L'C'] =  35;
		m_table[L'D'] =  36;
		m_table[L'E'] =  37;
		m_table[L'F'] =  38;
		m_table[L'G'] =  39;
		m_table[L'H'] =  40;
		m_table[L'I'] =  41;
		m_table[L'J'] =  42;
		m_table[L'K'] =  43;
		m_table[L'L'] =  44;
		m_table[L'M'] =  45;
		m_table[L'N'] =  46;
		m_table[L'O'] =  47;
		m_table[L'P'] =  48;
		m_table[L'Q'] =  49;
		m_table[L'R'] =  50;
		m_table[L'S'] =  51;
		m_table[L'T'] =  52;
		m_table[L'U'] =  53;
		m_table[L'V'] =  54;
		m_table[L'W'] =  55;
		m_table[L'X'] =  56;
		m_table[L'Y'] =  57;
		m_table[L'Z'] =  58;
		m_table[L'['] =  59;
		m_table[L'\\'] =  60;
		m_table[L']'] =  61;
		m_table[L'^'] =  62;
		m_table[L'_'] =  63;
		m_table[L'`'] =  64;
		m_table[L'a'] =  65;
		m_table[L'b'] =  66;
		m_table[L'c'] =  67;
		m_table[L'd'] =  68;
		m_table[L'e'] =  69;
		m_table[L'f'] =  70;
		m_table[L'g'] =  71;
		m_table[L'h'] =  72;
		m_table[L'i'] =  73;
		m_table[L'j'] =  74;
		m_table[L'k'] =  75;
		m_table[L'l'] =  76;
		m_table[L'm'] =  77;
		m_table[L'n'] =  78;
		m_table[L'o'] =  79;
		m_table[L'p'] =  80;
		m_table[L'q'] =  81;
		m_table[L'r'] =  82;
		m_table[L's'] =  83;
		m_table[L't'] =  84;
		m_table[L'u'] =  85;
		m_table[L'v'] =  86;
		m_table[L'w'] =  87;
		m_table[L'x'] =  88;
		m_table[L'y'] =  89;
		m_table[L'z'] =  90;
		m_table[L'{'] =  91;
		m_table[L'|'] =  92;
		m_table[L'}'] =  93;
		m_table[L'~'] =  94;
		m_table[0x007F] =  95; // DELETE

		m_table[0x0410] =  96; // CYRILLIC CAPITAL LETTER A
		m_table[0x0411] =  97; // CYRILLIC CAPITAL LETTER BE
		m_table[0x0412] =  98; // CYRILLIC CAPITAL LETTER VE
		m_table[0x0413] =  99; // CYRILLIC CAPITAL LETTER GHE
		m_table[0x0414] = 100; // CYRILLIC CAPITAL LETTER DE
		m_table[0x0415] = 101; // CYRILLIC CAPITAL LETTER IE
		m_table[0x0416] = 102; // CYRILLIC CAPITAL LETTER ZHE
		m_table[0x0417] = 103; // CYRILLIC CAPITAL LETTER ZE
		m_table[0x0418] = 104; // CYRILLIC CAPITAL LETTER I
		m_table[0x0419] = 105; // CYRILLIC CAPITAL LETTER SHORT I
		m_table[0x041A] = 106; // CYRILLIC CAPITAL LETTER KA
		m_table[0x041B] = 107; // CYRILLIC CAPITAL LETTER EL
		m_table[0x041C] = 108; // CYRILLIC CAPITAL LETTER EM
		m_table[0x041D] = 109; // CYRILLIC CAPITAL LETTER EN
		m_table[0x041E] = 110; // CYRILLIC CAPITAL LETTER O
		m_table[0x041F] = 111; // CYRILLIC CAPITAL LETTER PE
		m_table[0x0420] = 112; // CYRILLIC CAPITAL LETTER ER
		m_table[0x0421] = 113; // CYRILLIC CAPITAL LETTER ES
		m_table[0x0422] = 114; // CYRILLIC CAPITAL LETTER TE
		m_table[0x0423] = 115; // CYRILLIC CAPITAL LETTER U
		m_table[0x0424] = 116; // CYRILLIC CAPITAL LETTER EF
		m_table[0x0425] = 117; // CYRILLIC CAPITAL LETTER HA
		m_table[0x0426] = 118; // CYRILLIC CAPITAL LETTER TSE
		m_table[0x0427] = 119; // CYRILLIC CAPITAL LETTER CHE
		m_table[0x0428] = 120; // CYRILLIC CAPITAL LETTER SHA
		m_table[0x0429] = 121; // CYRILLIC CAPITAL LETTER SHCHA
		m_table[0x042A] = 122; // CYRILLIC CAPITAL LETTER HARD SIGN
		m_table[0x042B] = 123; // CYRILLIC CAPITAL LETTER YERU
		m_table[0x042C] = 124; // CYRILLIC CAPITAL LETTER SOFT SIGN
		m_table[0x042D] = 125; // CYRILLIC CAPITAL LETTER E
		m_table[0x042E] = 126; // CYRILLIC CAPITAL LETTER YU
		m_table[0x042F] = 127; // CYRILLIC CAPITAL LETTER YA
		m_table[0x0430] = 128; // CYRILLIC SMALL LETTER A
		m_table[0x0431] = 129; // CYRILLIC SMALL LETTER BE
		m_table[0x0432] = 130; // CYRILLIC SMALL LETTER VE
		m_table[0x0433] = 131; // CYRILLIC SMALL LETTER GHE
		m_table[0x0434] = 132; // CYRILLIC SMALL LETTER DE
		m_table[0x0435] = 133; // CYRILLIC SMALL LETTER IE
		m_table[0x0436] = 134; // CYRILLIC SMALL LETTER ZHE
		m_table[0x0437] = 135; // CYRILLIC SMALL LETTER ZE
		m_table[0x0438] = 136; // CYRILLIC SMALL LETTER I
		m_table[0x0439] = 137; // CYRILLIC SMALL LETTER SHORT I
		m_table[0x043A] = 138; // CYRILLIC SMALL LETTER KA
		m_table[0x043B] = 139; // CYRILLIC SMALL LETTER EL
		m_table[0x043C] = 140; // CYRILLIC SMALL LETTER EM
		m_table[0x043D] = 141; // CYRILLIC SMALL LETTER EN
		m_table[0x043E] = 142; // CYRILLIC SMALL LETTER O
		m_table[0x043F] = 143; // CYRILLIC SMALL LETTER PE
		m_table[0x0440] = 144; // CYRILLIC SMALL LETTER ER
		m_table[0x0441] = 145; // CYRILLIC SMALL LETTER ES
		m_table[0x0442] = 146; // CYRILLIC SMALL LETTER TE
		m_table[0x0443] = 147; // CYRILLIC SMALL LETTER U
		m_table[0x0444] = 148; // CYRILLIC SMALL LETTER EF
		m_table[0x0445] = 149; // CYRILLIC SMALL LETTER HA
		m_table[0x0446] = 150; // CYRILLIC SMALL LETTER TSE
		m_table[0x0447] = 151; // CYRILLIC SMALL LETTER CHE
		m_table[0x0448] = 152; // CYRILLIC SMALL LETTER SHA
		m_table[0x0449] = 153; // CYRILLIC SMALL LETTER SHCHA
		m_table[0x044A] = 154; // CYRILLIC SMALL LETTER HARD SIGN
		m_table[0x044B] = 155; // CYRILLIC SMALL LETTER YERU
		m_table[0x044C] = 156; // CYRILLIC SMALL LETTER SOFT SIGN
		m_table[0x044D] = 157; // CYRILLIC SMALL LETTER E
		m_table[0x044E] = 158; // CYRILLIC SMALL LETTER YU
		m_table[0x044F] = 159; // CYRILLIC SMALL LETTER YA
	}
};


/** Translation table for the RussianGold version of the game. */
class CTranslationTableRussianGold : public CTranslationTable
{
public:
	/** Constructor. */
	CTranslationTableRussianGold()
	{
		m_table    [L'A'] =   0;
		m_table[L'B'] =   1;
		m_table[L'C'] =   2;
		m_table[L'D'] =   3;
		m_table[L'E'] =   4;
		m_table[L'F'] =   5;
		m_table[L'G'] =   6;
		m_table[L'H'] =   7;
		m_table[L'I'] =   8;
		m_table[L'J'] =   9;
		m_table[L'K'] =  10;
		m_table[L'L'] =  11;
		m_table[L'M'] =  12;
		m_table[L'N'] =  13;
		m_table[L'O'] =  14;
		m_table[L'P'] =  15;
		m_table[L'Q'] =  16;
		m_table[L'R'] =  17;
		m_table[L'S'] =  18;
		m_table[L'T'] =  19;
		m_table[L'U'] =  20;
		m_table[L'V'] =  21;
		m_table[L'W'] =  22;
		m_table[L'X'] =  23;
		m_table[L'Y'] =  24;
		m_table[L'Z'] =  25;

		m_table[L'a'] =  26;
		m_table[L'b'] =  27;
		m_table[L'c'] =  28;
		m_table[L'd'] =  29;
		m_table[L'e'] =  30;
		m_table[L'f'] =  31;
		m_table[L'g'] =  32;
		m_table[L'h'] =  33;
		m_table[L'i'] =  34;
		m_table[L'j'] =  35;
		m_table[L'k'] =  36;
		m_table[L'l'] =  37;
		m_table[L'm'] =  38;
		m_table[L'n'] =  39;
		m_table[L'o'] =  40;
		m_table[L'p'] =  41;
		m_table[L'q'] =  42;
		m_table[L'r'] =  43;
		m_table[L's'] =  44;
		m_table[L't'] =  45;
		m_table[L'u'] =  46;
		m_table[L'v'] =  47;
		m_table[L'w'] =  48;
		m_table[L'x'] =  49;
		m_table[L'y'] =  50;
		m_table[L'z'] =  51;

		m_table[L'0'] =  52;
		m_table[L'1'] =  53;
		m_table[L'2'] =  54;
		m_table[L'3'] =  55;
		m_table[L'4'] =  56;
		m_table[L'5'] =  57;
		m_table[L'6'] =  58;
		m_table[L'7'] =  59;
		m_table[L'8'] =  60;
		m_table[L'9'] =  61;

		m_table[L'!'] =  62;
		m_table[L'@'] =  63;
		m_table[L'#'] =  64;
		m_table[L'$'] =  65;
		m_table[L'%'] =  66;
		m_table[L'^'] =  67;
		m_table[L'&'] =  68;
		m_table[L'*'] =  69;
		m_table[L'('] =  70;
		m_table[L')'] =  71;
		m_table[L'-'] =  72;
		m_table[L'_'] =  73;
		m_table[L'+'] =  74;
		m_table[L'='] =  75;
		m_table[L'|'] =  76;
		m_table[L'\\'] =  77;
		m_table[L'{'] =  78;
		m_table[L'}'] =  79;
		m_table[L'['] =  80;
		m_table[L']'] =  81;
		m_table[L':'] =  82;
		m_table[L';'] =  83;
		m_table[L'"'] =  84;
		m_table[L'\''] =  85;
		m_table[L'<'] =  86;
		m_table[L'>'] =  87;
		m_table[L','] =  88,
		m_table[L'.'] =  89;
		m_table[L'?'] =  90;
		m_table[L'/'] =  91;
		m_table[L' '] =  92;
		m_table[196] =  93; // A umlaut
		m_table[214] =  94; // O umlaut
		m_table[220] =  95; // U umlaut
		m_table[228] =  96; // a umlaut
		m_table[246] =  97; // o umlaut
		m_table[252] =  98; // u umlaut
		m_table[223] =  99; // sharp s

		// duplicate 196, // Ä
		m_table[192] = 165; // À
		m_table[193] = 166; // Á
		m_table[194] = 167; // Â
		m_table[199] = 168; // Ç
		m_table[203] = 169; // Ë
		m_table[200] = 170; // È
		m_table[201] = 171; // É
		m_table[202] = 172; // Ê
		m_table[207] = 173; // Ï
		// duplicate 214, // Ö
		m_table[210] = 175; // Ò
		m_table[211] = 176; // Ó
		m_table[212] = 177; // Ô
		// duplicate 220, // Ü
		m_table[217] = 179; // Ù
		m_table[218] = 180; // Ú
		m_table[219] = 181; // Û

		// missing lowercase glyphs, reuse uppercase from above
		// duplicate 228, // ä
		m_table[224] = 165; // à
		m_table[225] = 166; // á
		m_table[226] = 167; // â
		m_table[231] = 168; // ç
		m_table[235] = 169; // ë
		m_table[232] = 170; // è
		m_table[233] = 171; // é
		m_table[234] = 172; // ê
		m_table[239] = 173; // ï
		// duplicate 246, // ö
		m_table[242] = 175; // ò
		m_table[243] = 176; // ó
		m_table[244] = 177; // ô
		// duplicate 252, // ü
		m_table[249] = 179; // ù
		m_table[250] = 180; // ú
		m_table[251] = 181; // û

		m_table[0x00CC] = 182; // I with grave
		m_table[0x00EC] = 183; // i with grave
		m_table[0x0104] = 184; // A with ogonek
		m_table[0x0106] = 185; // C with acute
		m_table[0x0118] = 186; // E with ogonek
		m_table[0x0141] = 187; // L with stroke
		m_table[0x0143] = 188; // N with acute
		m_table[0x00D3] = 189; // O with acute (duplicate)
		m_table[0x015A] = 190; // S with acute
		m_table[0x017B] = 191; // Z with dot above
		m_table[0x0179] = 192; // Z with acute
		m_table[0x0105] = 193; // a with ogonek
		m_table[0x0107] = 194; // c with acute
		m_table[0x0119] = 195; // e with ogonek
		m_table[0x0142] = 196; // l with stroke
		m_table[0x0144] = 197; // n with acute
		m_table[0x00F3] = 198; // o with acute (duplicate)
		m_table[0x015B] = 199; // s with acute
		m_table[0x017C] = 200; // z with dot above
		m_table[0x017A] = 201; // z with acute

		m_table[0x0410] = 100; // CYRILLIC CAPITAL LETTER A
		m_table[0x0411] = 101; // CYRILLIC CAPITAL LETTER BE
		m_table[0x0412] = 102; // CYRILLIC CAPITAL LETTER VE
		m_table[0x0413] = 103; // CYRILLIC CAPITAL LETTER GHE
		m_table[0x0414] = 104; // CYRILLIC CAPITAL LETTER DE
		m_table[0x0415] = 105; // CYRILLIC CAPITAL LETTER IE
		m_table[0x0416] = 106; // CYRILLIC CAPITAL LETTER ZHE
		m_table[0x0417] = 107; // CYRILLIC CAPITAL LETTER ZE
		m_table[0x0418] = 108; // CYRILLIC CAPITAL LETTER I
		m_table[0x0419] = 109; // CYRILLIC CAPITAL LETTER SHORT I
		m_table[0x041A] = 110; // CYRILLIC CAPITAL LETTER KA
		m_table[0x041B] = 111; // CYRILLIC CAPITAL LETTER EL
		m_table[0x041C] = 112; // CYRILLIC CAPITAL LETTER EM
		m_table[0x041D] = 113; // CYRILLIC CAPITAL LETTER EN
		m_table[0x041E] = 114; // CYRILLIC CAPITAL LETTER O
		m_table[0x041F] = 115; // CYRILLIC CAPITAL LETTER PE
		m_table[0x0420] = 116; // CYRILLIC CAPITAL LETTER ER
		m_table[0x0421] = 117; // CYRILLIC CAPITAL LETTER ES
		m_table[0x0422] = 118; // CYRILLIC CAPITAL LETTER TE
		m_table[0x0423] = 119; // CYRILLIC CAPITAL LETTER U
		m_table[0x0424] = 120; // CYRILLIC CAPITAL LETTER EF
		m_table[0x0425] = 121; // CYRILLIC CAPITAL LETTER HA
		m_table[0x0426] = 122; // CYRILLIC CAPITAL LETTER TSE
		m_table[0x0427] = 123; // CYRILLIC CAPITAL LETTER CHE
		m_table[0x0428] = 124; // CYRILLIC CAPITAL LETTER SHA
		m_table[0x0429] = 125; // CYRILLIC CAPITAL LETTER SHCHA
		m_table[0x042A] = 126; // CYRILLIC CAPITAL LETTER HARD SIGN
		m_table[0x042B] = 127; // CYRILLIC CAPITAL LETTER YERU
		m_table[0x042C] = 128; // CYRILLIC CAPITAL LETTER SOFT SIGN
		m_table[0x042D] = 129; // CYRILLIC CAPITAL LETTER E
		m_table[0x042E] = 130; // CYRILLIC CAPITAL LETTER YU
		m_table[0x042F] = 131; // CYRILLIC CAPITAL LETTER YA
		m_table[0x0430] = 132; // CYRILLIC SMALL LETTER A
		m_table[0x0431] = 133; // CYRILLIC SMALL LETTER BE
		m_table[0x0432] = 134; // CYRILLIC SMALL LETTER VE
		m_table[0x0433] = 135; // CYRILLIC SMALL LETTER GHE
		m_table[0x0434] = 136; // CYRILLIC SMALL LETTER DE
		m_table[0x0435] = 137; // CYRILLIC SMALL LETTER IE
		m_table[0x0436] = 138; // CYRILLIC SMALL LETTER ZHE
		m_table[0x0437] = 139; // CYRILLIC SMALL LETTER ZE
		m_table[0x0438] = 140; // CYRILLIC SMALL LETTER I
		m_table[0x0439] = 141; // CYRILLIC SMALL LETTER SHORT I
		m_table[0x043A] = 142; // CYRILLIC SMALL LETTER KA
		m_table[0x043B] = 143; // CYRILLIC SMALL LETTER EL
		m_table[0x043C] = 144; // CYRILLIC SMALL LETTER EM
		m_table[0x043D] = 145; // CYRILLIC SMALL LETTER EN
		m_table[0x043E] = 146; // CYRILLIC SMALL LETTER O
		m_table[0x043F] = 147; // CYRILLIC SMALL LETTER PE
		m_table[0x0440] = 148; // CYRILLIC SMALL LETTER ER
		m_table[0x0441] = 149; // CYRILLIC SMALL LETTER ES
		m_table[0x0442] = 150; // CYRILLIC SMALL LETTER TE
		m_table[0x0443] = 151; // CYRILLIC SMALL LETTER U
		m_table[0x0444] = 152; // CYRILLIC SMALL LETTER EF
		m_table[0x0445] = 153; // CYRILLIC SMALL LETTER HA
		m_table[0x0446] = 154; // CYRILLIC SMALL LETTER TSE
		m_table[0x0447] = 155; // CYRILLIC SMALL LETTER CHE
		m_table[0x0448] = 156; // CYRILLIC SMALL LETTER SHA
		m_table[0x0449] = 157; // CYRILLIC SMALL LETTER SHCHA
		m_table[0x044A] = 158; // CYRILLIC SMALL LETTER HARD SIGN
		m_table[0x044B] = 159; // CYRILLIC SMALL LETTER YERU
		m_table[0x044C] = 160; // CYRILLIC SMALL LETTER SOFT SIGN
		m_table[0x044D] = 161; // CYRILLIC SMALL LETTER E
		m_table[0x044E] = 162; // CYRILLIC SMALL LETTER YU
		m_table[0x044F] = 163; // CYRILLIC SMALL LETTER YA
	}
};



static const CTranslationTableEnglish           s_en_TranslationTable;
static const CTranslationTableFrench            s_fr_TranslationTable;
static const CTranslationTableRussian           s_rus_TranslationTable;
static const CTranslationTableRussianGold       s_rusGold_TranslationTable;

const CTranslationTable * g_en_TranslationTable          = &s_en_TranslationTable;
const CTranslationTable * g_fr_TranslationTable          = &s_fr_TranslationTable;
const CTranslationTable * g_rus_TranslationTable         = &s_rus_TranslationTable;
const CTranslationTable * g_rusGold_TranslationTable     = &s_rusGold_TranslationTable;

