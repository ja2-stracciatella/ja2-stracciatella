#include "AIMListingModel.h"
#include "Directories.h"
#include "Exceptions.h"
#include "Soldier_Control.h"
#include <set>

namespace BioStrings {
	constexpr const char* BINARY_STRING_FILE = BINARYDATADIR "/aimbios.edt";
	constexpr uint32_t BINARY_DESCRIPTION_SIZE = 400;
	constexpr uint32_t BINARY_ADDITIONAL_INFORMATION_SIZE = 160;
	constexpr uint32_t BINARY_ITEM_TOTAL_SIZE = BINARY_DESCRIPTION_SIZE + BINARY_ADDITIONAL_INFORMATION_SIZE;
}

AIMListingModel::AIMListingModel(uint8_t index_, uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_) : index(index_), profileID(profileID_), description(std::move(description_)), additionalInformation(std::move(additionalInformation_)) {}

AIMListingModel* AIMListingModel::deserialize(uint8_t index, const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader)
{
	using namespace BioStrings;

	auto reader = json.toObject();
	ST::string profileName = reader.GetString("profile");
	auto profile = mercSystem->getMercProfileInfoByName(profileName);
	if (profile == NULL) {
		throw DataError(ST::format("'{}' does not refer to a valid profile", profile));
	}
	if (profile->mercType != MercType::AIM) {
		throw DataError(ST::format("Profile '{}' does not refer to an AIM profile", profile));
	}

	if ((!reader.has("description") || !reader.has("additionalInformation")) && !reader.has("bioIndex")) {
		SLOGE("M.E.R.C listing {} should have bioIndex set when description or additionalInformation are not set, defaulting to 0", profileName);
	}
	auto bioIndex = static_cast<uint32_t>(reader.getOptionalUInt("bioIndex"));

	return new AIMListingModel(
		index,
		profile->profileID,
		TranslatableString::Utils::resolveOptionalProperty(
			stringLoader,
			reader,
			"description",
			std::make_unique<TranslatableString::Binary>(BINARY_STRING_FILE, BINARY_ITEM_TOTAL_SIZE * bioIndex, BINARY_DESCRIPTION_SIZE)
		),
		TranslatableString::Utils::resolveOptionalProperty(
			stringLoader,
			reader,
			"additionalInformation",
			std::make_unique<TranslatableString::Binary>(BINARY_STRING_FILE, BINARY_ITEM_TOTAL_SIZE * bioIndex + BINARY_DESCRIPTION_SIZE, BINARY_ADDITIONAL_INFORMATION_SIZE)
		)
	);

}

void AIMListingModel::validateData(const std::vector<const AIMListingModel*>& models) {
	std::set<uint8_t> uniqueProfileIDs;
	for (auto m : models)
	{
		if (m->profileID >= NO_PROFILE)
		{
			throw DataError(ST::format("Invalid profileID '{}'", m->profileID));
		}

		// Check if we have duplicates
		if (uniqueProfileIDs.find(m->profileID) != uniqueProfileIDs.end())
		{
			throw DataError(ST::format("profileID {} has been listed more than once", m->profileID));
		}
		uniqueProfileIDs.insert(m->profileID);
	}
}
