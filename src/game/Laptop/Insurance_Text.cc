#include "Insurance_Text.h"
#include "EDT.h"
#include <optional>

std::optional<EDTFile> gInsuranceSingleLineTexts;
std::optional<EDTFile> gInsuranceMultiLineTexts;

void OpenInsuranceTexts() {
	gInsuranceSingleLineTexts = EDTFile(EDTFile::INSURANCE_SINGLE_LINE_TEXTS);
	gInsuranceMultiLineTexts = EDTFile(EDTFile::INSURANCE_MULTI_LINE_TEXTS);
}

void CloseInsuranceTexts() {
	gInsuranceSingleLineTexts = std::nullopt;
	gInsuranceMultiLineTexts = std::nullopt;
}

ST::string GetInsuranceText(const UINT8 ubNumber)
{
	if( ubNumber < INS_MULTI_LINE_BEGINS )
	{
		//Get and display the card saying
		return gInsuranceSingleLineTexts->at(ubNumber, 0);
	}
	else
	{
		//Get and display the card saying
		return gInsuranceMultiLineTexts->at(ubNumber - INS_MULTI_LINE_BEGINS - 1, 0);
	}
}
