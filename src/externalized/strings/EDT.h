#pragma once

#include "ContentManager.h"
#include "Directories.h"
#include "GameInstance.h"
#include "IEDT.h"
#include <string_view>

/*
    EDTFile: support class for reading strings from EDT files.

    EDT files are essentially just one big blob of slightly obfuscated
    UTF-16 (or UCS2?) characters. They contain absolutely no metadata to
    indicate where one string ends and the next one begins. It is entirely
    up to the code reading the file to break up the blob into single strings.

    This interpretation is done in two ways in JA2:

    - The entire file is seen as an array of fixed width strings.
    - The file is seen as a table where each row has the same column layout.

    The first case could also be interpreted as a table where each row has
    just one column. To keep things simple, the IEDT interface treats all EDT
    files as tables.

    The default content manager uses the IEDT implementation provided by the
    class ClassicEDT. This implementation can only read files in the original
    .edt file format. It does not do much; it simply opens the file and
	calculates the file offset from the row and column number.

    The mod content manager uses the IEDT implementation provided by the class
    JsonEDT, where each table entry can be provided by a JSON file. The file
	name of this file is the original file name with .json appended. Strings
	that are not found in the JSON file are taken from an original .edt file
	as a fallback option.

	As already mentioned, EDT contains no metadata, therefore ClassicEDT
	requires some details about each individual file. These details are
	contained in the EDTFilesTable below which also shows which files can
	currently be overriden.

	To support a new file add an entry to the table then change the source
	file to use EDTFile instead of ContentManager::loadEncryptedString.
*/

class EDTFile
{
	struct EDTFilesTable
	{
		std::string_view     filename;
		IEDT::column_list    columns;
	};

	static inline EDTFilesTable const EDTFilesTable[]
	{
		/* Strings of the help screen.
		   123 rows, each with one column of 640 characters.
		   See HelpScreenText.h to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/help.edt", { 640 } },
		/* Strings of the credits.
		   496 rows, each with one column of 80 characters.
		   See Credits.cc to get a rough overview of the row contents and control characters.
		*/
		{ BINARYDATADIR "/credits.edt", { 80 } },
		/* Strings for the AIM page.
		   46 rows, each with one column of 400 characters.
		   See AIM.cc and AIMHistory.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/aimhist.edt", { 400 } },
		/* Strings for the AIM alumni names.
		   102 rows, each with one column of 80 characters.
		   See AIMArchives.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/alumname.edt", { 80 } },
		/* Strings for the AIM alumni descriptions.
		   102 rows, each with two column of 80 and 560 characters for full name and description.
		   See AIMArchives.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/alumni.edt", { 80, 560 } },
		/* Strings for the AIM policies.
		   92 rows, each with one column of 400 characters.
		   See AIMPolicies.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/aimpol.edt", { 400 } },
		/* Strings for single line descriptions on the insurance laptop page.
		   60 rows, each with one column of 80 characters.
		   See Insurance.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/insurancesingle.edt", { 80 } },
		/* Strings for multi line descriptions on the insurance laptop page.
		   72 rows, each with one column of 400 characters.
		   See Insurance.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/insurancemulti.edt", { 400 } },
		/* Strings for IMP.
		   478 rows, each with one column of 400 characters.
		   See IMP_Text_System.cc to get a rough overview of the row contents.
		*/
		{ BINARYDATADIR "/imptext.edt", { 400 } },
	};

	IEDT::uptr mIEDT;

public:
	enum EDTFilesList
	{
		HELP,
		CREDITS,
		AIM_TEXT,
		AIM_ALUMNI_NAMES,
		AIM_ALUMNI_TEXTS,
		AIM_POLICIES,
		INSURANCE_SINGLE_LINE_TEXTS,
		INSURANCE_MULTI_LINE_TEXTS,
		IMP_TEXTS
	};

	EDTFile(EDTFilesList const whichFile)
	{
		auto const& fileMetaData{ EDTFilesTable[whichFile] };
		mIEDT = GCM->openEDT(fileMetaData.filename, fileMetaData.columns);
	}

	auto at(unsigned const row, unsigned const column = 0) const
	{
		return mIEDT->at(row, column);
	}
};
