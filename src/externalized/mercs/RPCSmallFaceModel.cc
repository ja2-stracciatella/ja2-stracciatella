#include "RPCSmallFaceModel.h"
#include <rapidjson/document.h>
#include <stdexcept>

RPCSmallFaceModel* RPCSmallFaceModel::deserialize(const rapidjson::Value& json)
{
	uint8_t ubProfileID = static_cast<uint8_t>(json["profileID"].GetUint());
	if (!ubProfileID) throw std::runtime_error("invalid profileID");

	auto faceXY = json["faceXY"].GetArray();
	if (faceXY.Size() != 4) throw std::runtime_error("faceXY must have exactly 4 elements");

	auto face = new RPCSmallFaceModel();
	face->ubProfileID = ubProfileID;
	face->bEyesX = faceXY[0].GetInt();
	face->bEyesY = faceXY[1].GetInt();
	face->bMouthX = faceXY[2].GetInt();
	face->bMouthY = faceXY[3].GetInt();
	
	return face;
}
