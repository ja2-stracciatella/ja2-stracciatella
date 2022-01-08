#include "Exceptions.h"
#include "RPCSmallFaceModel.h"

#include <rapidjson/document.h>

RPCSmallFaceModel* RPCSmallFaceModel::deserialize(const rapidjson::Value& json, const MercSystem* mercSystem)
{
	ST::string profile = json["profile"].GetString();
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		throw DataError(ST::format("`{}` does not refer to a valid profile.", profile));
	}
	auto profileId = mercProfile->profileID;

	auto eyesXY = json["eyesXY"].GetArray();
	auto mouthXY = json["mouthXY"].GetArray();

	auto face = new RPCSmallFaceModel();
	face->ubProfileID = profileId;
	face->bEyesX = eyesXY[0].GetInt();
	face->bEyesY = eyesXY[1].GetInt();
	face->bMouthX = mouthXY[0].GetInt();
	face->bMouthY = mouthXY[1].GetInt();
	
	return face;
}
