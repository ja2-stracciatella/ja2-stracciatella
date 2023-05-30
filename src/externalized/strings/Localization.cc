#include "Localization.h"
#include "Json.h"
#include "stracciatella.h"
#include <string_theory/format>
#include <string_theory/string>
#include <algorithm>
#include <exception>
#include <utility>


namespace L10n
{

char const * GetSuffix(VanillaVersion const version, bool const combineRusAndRusGold)
{
	switch(version)
	{
	case VanillaVersion::SIMPLIFIED_CHINESE: return "-chs";
	case VanillaVersion::DUTCH:        return "-dut";
	case VanillaVersion::ENGLISH:      return "-eng";
	case VanillaVersion::FRENCH:       return "-fr";
	case VanillaVersion::GERMAN:       return "-ger";
	case VanillaVersion::ITALIAN:      return "-it";
	case VanillaVersion::POLISH:       return "-pl";

	case VanillaVersion::RUSSIAN:
	case VanillaVersion::RUSSIAN_GOLD:
		return combineRusAndRusGold ? "-rus" : "-rusgold";

	default:
		throw std::runtime_error(ST::format("unknown game version {}", static_cast<int>(version)).c_str());
	}
}


void GetArray(ST::string * const arrayBegin,
              std::size_t const arraySize,
              JsonObject const& jObject,
              char const * const arrayName)
{
	auto strings = jObject.GetValue(arrayName).toVec();
	if (strings.size() != arraySize)
	{
		throw std::runtime_error(ST::format(
			"Translation of {} has wrong length: expected {}, actual {}",
			arrayName, arraySize, strings.size()).c_str());
	}

	std::transform(
		strings.cbegin(), strings.cend(), arrayBegin,
		[](JsonValue const& jval) { return jval.toString(); });
}


L10n_t::L10n_t(SGPFile * const translationFile)
{
	auto const json{ JsonValue::deserialize(translationFile->readStringToEnd()).toObject() };

#define GetString(stringname) stringname = std::move(json.GetString(#stringname))
#define GetArray(arrayname) GetArray(arrayname.begin(), arrayname.size(), json, #arrayname)

	GetArray(WeaponType);
	GetArray(Message);
	GetArray(TeamTurnString);
	GetArray(pAssignMenuStrings);
	GetArray(pTrainingStrings);
	GetArray(pTrainingMenuStrings);
	GetArray(pAttributeMenuStrings);
	GetArray(pVehicleStrings);
	GetArray(pShortAttributeStrings);
	GetArray(pContractStrings);
	GetArray(pAssignmentStrings);
	GetArray(pConditionStrings);
	GetArray(pPersonnelScreenStrings);
	GetArray(pUpperLeftMapScreenStrings);
	GetArray(pTacticalPopupButtonStrings);
	GetArray(pSquadMenuStrings);
	GetArray(pDoorTrapStrings);
	GetArray(pLongAssignmentStrings);
	GetArray(pMapScreenMouseRegionHelpText);
	GetArray(pNoiseVolStr);
	GetArray(pNoiseTypeStr);
	GetArray(pDirectionStr);
	GetArray(pRemoveMercStrings);
	GetArray(sTimeStrings);
	GetArray(pInvPanelTitleStrings);
	GetArray(pPOWStrings);
	GetArray(pMilitiaString);
	GetArray(pMilitiaButtonString);
	GetArray(pEpcMenuStrings);
	GetArray(pRepairStrings);
	GetArray(sPreStatBuildString);
	GetArray(sStatGainStrings);
	GetArray(pHelicopterEtaStrings);
	GetString(sMapLevelString);
	GetString(gsLoyalString);
	GetString(gsUndergroundString);
	GetArray(gsTimeStrings);
	GetArray(sFacilitiesStrings);
	GetArray(pMapPopUpInventoryText);
	GetArray(pwTownInfoStrings);
	GetArray(pwMineStrings);
	GetArray(pwMiscSectorStrings);
	GetArray(pMapInventoryErrorString);
	GetArray(pMapInventoryStrings);
	GetArray(pMovementMenuStrings);
	GetArray(pUpdateMercStrings);
	GetArray(pMapScreenBorderButtonHelpText);
	GetArray(pMapScreenBottomFastHelp);
	GetString(pMapScreenBottomText);
	GetString(pMercDeadString);
	GetArray(pSenderNameList);
	GetString(pNewMailStrings);
	GetArray(pDeleteMailStrings);
	GetArray(pEmailHeaders);
	GetString(pEmailTitleText);
	GetString(pFinanceTitle);
	GetArray(pFinanceSummary);
	GetArray(pFinanceHeaders);
	GetArray(pTransactionText);
	GetArray(pMoralStrings);
	GetArray(pSkyriderText);
	GetString(str_left_equipment);
	GetArray(pMapScreenStatusStrings);
	GetArray(pMapScreenPrevNextCharButtonHelpText);
	GetString(pEtaString);
	GetArray(pShortVehicleStrings);
	GetArray(pTrashItemText);
	GetArray(pMapErrorString);
	GetArray(pMapPlotStrings);
	GetArray(pBullseyeStrings);
	GetArray(pMiscMapScreenMouseRegionHelpText);
	GetString(str_he_leaves_where_drop_equipment);
	GetString(str_she_leaves_where_drop_equipment);
	GetString(str_he_leaves_drops_equipment);
	GetString(str_she_leaves_drops_equipment);
	GetArray(pImpPopUpStrings);
	GetArray(pImpButtonText);
	GetArray(pExtraIMPStrings);
	GetString(pFilesTitle);
	GetArray(pFilesSenderList);
	GetString(pHistoryLocations);
	GetArray(pHistoryStrings);
	GetArray(pHistoryHeaders);
	GetString(pHistoryTitle);
	GetArray(pShowBookmarkString);
	GetArray(pWebPagesTitles);
	GetString(pWebTitle);
	GetString(pPersonnelString);
	GetString(pErrorStrings);
	GetArray(pDownloadString);
	GetArray(pBookMarkStrings);
	GetArray(pLaptopIcons);
	GetArray(gsAtmStartButtonText);
	GetArray(pPersonnelTeamStatsStrings);
	GetArray(pPersonnelCurrentTeamStatsStrings);
	GetArray(pPersonelTeamStrings);
	GetArray(pPersonnelDepartedStateStrings);
	GetArray(pMapHortIndex);
	GetArray(pMapVertIndex);
	GetArray(pMapDepthIndex);
	GetArray(pLaptopTitles);
	GetString(pDayStrings);
	GetArray(pMilitiaConfirmStrings);
	GetString(pSkillAtZeroWarning);
	GetString(pIMPBeginScreenStrings);
	GetString(pIMPFinishButtonText);
	GetString(pIMPFinishStrings);
	GetString(pIMPVoicesStrings);
	GetString(pPersTitleText);
	GetArray(pPausedGameText);
	GetArray(zOptionsToggleText);
	GetArray(zOptionsScreenHelpText);
	GetArray(pDoctorWarningString);
	GetArray(pMilitiaButtonsHelpText);
	GetString(pMapScreenJustStartedHelpText);
	GetString(pLandMarkInSectorString);
	GetArray(gzMercSkillText);
	GetArray(gzNonPersistantPBIText);
	GetArray(gzMiscString);
	GetArray(wMapScreenSortButtonHelpText);
	GetArray(pNewNoiseStr);
	GetArray(gzLateLocalizedString);
	GetString(pAntiHackerString);
	GetArray(pMessageStrings);
	GetArray(ItemPickupHelpPopup);
	GetArray(TacticalStr);
	GetArray(LargeTacticalStr);
	GetString(zDialogActions);
	GetArray(zDealerStrings);
	GetArray(zTalkMenuStrings);
	GetArray(gzMoneyAmounts);
	GetString(gzProsLabel);
	GetString(gzConsLabel);
	GetArray(gMoneyStatsDesc);
	GetArray(gWeaponStatsDesc);
	GetArray(sKeyDescriptionStrings);
	GetArray(zHealthStr);
	GetArray(zVehicleName);
	GetArray(pExitingSectorHelpText);
	GetArray(InsContractText);
	GetArray(InsInfoText);
	GetArray(MercAccountText);
	GetArray(MercInfo);
	GetArray(MercNoAccountText);
	GetArray(MercHomePageText);
	GetArray(sFuneralString);
	GetArray(sFloristText);
	GetArray(sOrderFormText);
	GetArray(sFloristGalleryText);
	GetArray(sFloristCards);
	GetArray(BobbyROrderFormText);
	GetArray(BobbyRText);
	GetString(str_bobbyr_guns_num_guns_that_use_ammo);
	GetArray(BobbyRaysFrontText);
	GetArray(AimSortText);
	GetString(str_aim_sort_price);
	GetString(str_aim_sort_experience);
	GetString(str_aim_sort_marksmanship);
	GetString(str_aim_sort_medical);
	GetString(str_aim_sort_explosives);
	GetString(str_aim_sort_mechanical);
	GetString(str_aim_sort_ascending);
	GetString(str_aim_sort_descending);
	GetArray(AimPolicyText);
	GetArray(AimMemberText);
	GetArray(CharacterInfo);
	GetArray(VideoConfercingText);
	GetArray(AimPopUpText);
	GetString(AimLinkText);
	GetArray(AimHistoryText);
	GetArray(AimFiText);
	GetArray(AimAlumniText);
	GetArray(AimScreenText);
	GetArray(AimBottomMenuText);
	GetArray(zMarksMapScreenText);
	GetArray(gpStrategicString);
	GetString(gpGameClockString);
	GetArray(SKI_Text);
	GetArray(SkiMessageBoxText);
	GetArray(zSaveLoadText);
	GetArray(zOptionsText);
	GetArray(gzGIOScreenText);
	GetString(gzHelpScreenText);
	GetArray(gzLaptopHelpText);
	GetArray(gzMoneyWithdrawMessageText);
	GetString(gzCopyrightText);
	GetArray(BrokenLinkText);
	GetArray(gzBobbyRShipmentText);
	GetArray(zGioDifConfirmText);
	GetArray(gzCreditNames);
	GetArray(gzCreditNameTitle);
	GetArray(gzCreditNameFunny);
	GetString(pContractButtonString);
	GetString(gzIntroScreen);
	GetArray(pUpdatePanelButtons);
	GetArray(sRepairsDoneString);
	GetString(str_ceramic_plates_smashed);
	GetString(str_arrival_rerouted);
	GetString(str_stat_health);
	GetString(str_stat_agility);
	GetString(str_stat_dexterity);
	GetString(str_stat_strength);
	GetString(str_stat_leadership);
	GetString(str_stat_wisdom);
	GetString(str_stat_exp_level);
	GetString(str_stat_marksmanship);
	GetString(str_stat_mechanical);
	GetString(str_stat_explosive);
	GetString(str_stat_medical);
	GetArray(str_stat_list);
	GetArray(str_aim_sort_list);
	GetArray(zNewTacticalMessages);
	GetString(str_iron_man_mode_warning);
	GetString(str_dead_is_dead_mode_warning);
	GetString(str_dead_is_dead_mode_enter_name);
	GetArray(gs_dead_is_dead_mode_tab_name);
	GetArray(gzIMPSkillTraitsText);

#undef GetString
#undef GetArray
}

} // namespace