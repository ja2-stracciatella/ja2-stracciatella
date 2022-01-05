#include "RPCSmallFaceModel.h"
#include <rapidjson/document.h>
#include <stdexcept>

RPCSmallFaceModel* RPCSmallFaceModel::deserialize(const rapidjson::Value& json, const MercSystem* mercSystem)
{
	ST::string profile = json["profile"].GetString();
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		ST::string err = ST::format("`{}` does not refer to a valid profile.", profile);
		throw std::runtime_error(err.to_std_string());
	}
	auto profileId = mercProfile->profileID;

	auto eyesXY = json["eyesXY"].GetArray();
	if (eyesXY.Size() != 2) throw std::runtime_error("eyesXY must have exactly 2 elements");

	auto mouthXY = json["mouthXY"].GetArray();
	if (mouthXY.Size() != 2) throw std::runtime_error("mouthXY must have exactly 2 elements");

	auto face = new RPCSmallFaceModel();
	face->ubProfileID = profileId;
	face->bEyesX = eyesXY[0].GetInt();
	face->bEyesY = eyesXY[1].GetInt();
	face->bMouthX = mouthXY[0].GetInt();
	face->bMouthY = mouthXY[1].GetInt();
	
	return face;
}
