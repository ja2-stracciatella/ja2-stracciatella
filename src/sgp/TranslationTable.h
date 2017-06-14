#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#define TRANSLATION_TABLE_SIZE (0x452)

/**
 * The sole purpose of this class is to initialize TranslationTable
 * on program startup without using GCC C99 extenstions.
 */
class CTranslationTable
{
public:
	unsigned char m_table[TRANSLATION_TABLE_SIZE];

	/** Constructor. */
	CTranslationTable();
};

extern unsigned char const *TranslationTable;

extern const CTranslationTable * g_en_TranslationTable;
extern const CTranslationTable * g_fr_TranslationTable;
extern const CTranslationTable * g_rus_TranslationTable;
extern const CTranslationTable * g_rusGold_TranslationTable;

#endif
