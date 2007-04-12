/* The text->font map table. */
static const UINT16 TranslationTable[] = {
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'!',
	'@',
	'#',
	'$',
	'%',
	'^',
	'&',
	'*',
	'(',
	')',
	'-',
	'_',
	'+',
	'=',
	'|',
	'\\',
	'{',
	'}',// 80
	'[',
	']',
	':',
	';',
	'"',
	'\'',
	'<',
	'>',
	',',
	'.',
	'?',
	'/',
	' ', //93

#ifdef JA2
	196, // "A" umlaut
	214, // "O" umlaut
	220, // "U" umlaut
	228, // "a" umlaut
	246, // "o" umlaut
	252, // "u" umlaut
	223, // double-s that looks like a beta/B  // 100
	// START OF FUNKY RUSSIAN STUFF
	1101,
	1102,
	1103,
	1104,
	1105,
	1106,
	1107,
	1108,
	1109,
	1110,
	1111,
	1112,
	1113,
	1114,
	1115,
	1116,
	1117,
	1118,
	1119,
	1120,
	1121,
	1122,
	1123,
	1124,
	1125,
	1126,
	1127,
	1128,
	1129,
	1130, // 130
	1131,
	1132,
	// END OF FUNKY RUSSIAN STUFF
	196, // Ä
	192, // À
	193, // Á
	194, // Â
	199, // Ç
	203, // Ë
	200, // È
	201, // É				140
	202, // Ê
	207, // Ï
	214, // Ö
	210, // Ò
	211, // Ó
	212, // Ô
	220, // Ü
	217, // Ù
	218, // Ú
	219, // Û				150
	228, // ä
	224, // à
	225, // á
	226, // â
	231, // ç
	235, // ë
	232, // è
	233, // é
	234, // ê
	239, // ï				160
	246, // ö
	242, // ò
	243, // ó
	244, // ô
	252, // ü
	249, // ù
	250, // ú
	251, // û
	204, // Ì
	206, // Î				170
	236, // ì
	238  // î

#else
	// Windows Code Page 1252 Western Standard Character Set

	193, // "A" acute
	192, // "A" grave
	193, // "A" circumflex
	196, // "A" umlaut
	195, // "A" tilde
	197, // "A" ring
	199, // "C" cedile
	201, // "E" acute
	200, // "E" grave
	202, // "E" circumflex
	203, // "E" umlaut
	205, // "I" acute
	204, // "I" grave
	206, // "I" circumflex
	207, // "I" umlaut
	209, // "N" tilde
	211, // "O" acute
	210, // "O" grave
	212, // "O" circumflex
	214, // "O" umlaut
	213, // "O" tilde
	216, // "0" O strike-through
	218, // "U" acute
	217, // "U" grave
	219, // "U" circumflex
	220, // "U" umlaut
	221, // "Y" acute
	225, // "a" acute
	224, // "a" grave
	226, // "a" circumflex
	228, // "a" umlaut
	227, // "a" tilde
	229, // "a" ring
	231, // "c" cedile
	233, // "e" acute
	232, // "e" grave
	234, // "e" circumflex
	235, // "e" umlaut
	237, // "i" acute
	236, // "i" grave
	238, // "i" circumflex
	239, // "i" umlaut
	241, // "n" tilde
	243, // "o" acute
	242, // "o" grave
	244, // "o" circumflex
	246, // "o" umlaut
	245, // "o" tilde
	248, // "o" strike-through
	250, // "u" acute
	249, // "u" grave
	251, // "u" circumflex
	252, // "u" umlaut
	254, // "y" acute
	255, // "y" umlaut
	223, // beta

	// Font glyphs for spell targeting icons
	// ATE: IMPORTANT! INcreate the array above if you add any new items here...
	FONT_GLYPH_TARGET_POINT,
	FONT_GLYPH_TARGET_CONE,
	FONT_GLYPH_TARGET_SINGLE,
	FONT_GLYPH_TARGET_GROUP,
	FONT_GLYPH_TARGET_NONE
#endif
};
#ifdef JA2
	CASSERT(lengthof(TranslationTable) == 172)
#else
	CASSERT(lengthof(TranslationTable) == 154)
#endif
