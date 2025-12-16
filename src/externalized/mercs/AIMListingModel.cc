#include "AIMListingModel.h"
#include "Directories.h"
#include "Exceptions.h"

namespace BioStrings {
	constexpr const char* BINARY_STRING_FILE = BINARYDATADIR "/aimbios.edt";
	constexpr uint32_t BINARY_DESCRIPTION_SIZE = 400;
	constexpr uint32_t BINARY_ADDITIONAL_INFORMATION_SIZE = 160;
	constexpr uint32_t BINARY_ITEM_TOTAL_SIZE = BINARY_DESCRIPTION_SIZE + BINARY_ADDITIONAL_INFORMATION_SIZE;
}

AIMListingModel::AIMListingModel(uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_) : profileID(profileID_), description(std::move(description_)), additionalInformation(std::move(additionalInformation_)) {}

std::unique_ptr<AIMListingModel> AIMListingModel::deserialize(const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader)
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

	return std::make_unique<AIMListingModel>(
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
