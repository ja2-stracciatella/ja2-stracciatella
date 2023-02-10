#include "Exceptions.h"
#include "RPCSmallFaceModel.h"

#include <string_theory/format>

RPCSmallFaceModel* RPCSmallFaceModel::deserialize(const JsonValue& json, const MercSystem* mercSystem)
{
	auto r = json.toObject();
	ST::string profile = r.GetString("profile");
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		throw DataError(ST::format("`{}` does not refer to a valid profile.", profile));
	}
	auto profileId = mercProfile->profileID;

	auto eyesXY = r["eyesXY"].toVec();
	auto mouthXY = r["mouthXY"].toVec();

	auto face = new RPCSmallFaceModel();
	face->ubProfileID = profileId;
	face->bEyesX = eyesXY[0].toInt();
	face->bEyesY = eyesXY[1].toInt();
	face->bMouthX = mouthXY[0].toInt();
	face->bMouthY = mouthXY[1].toInt();

	return face;
}
