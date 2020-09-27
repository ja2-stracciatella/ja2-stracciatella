#include "RPCSmallFaceModel.h"
#include <rapidjson/document.h>
#include <stdexcept>

RPCSmallFaceModel* RPCSmallFaceModel::deserialize(const rapidjson::Value& json)
{
	uint8_t ubProfileID = static_cast<uint8_t>(json["profileID"].GetUint());
	if (!ubProfileID) throw std::runtime_error("invalid profileID");

	auto eyesXY = json["eyesXY"].GetArray();
	if (eyesXY.Size() != 2) throw std::runtime_error("eyesXY must have exactly 2 elements");

	auto mouthXY = json["mouthXY"].GetArray();
	if (mouthXY.Size() != 2) throw std::runtime_error("mouthXY must have exactly 2 elements");

	auto face = new RPCSmallFaceModel();
	face->ubProfileID = ubProfileID;
	face->bEyesX = eyesXY[0].GetInt();
	face->bEyesY = eyesXY[1].GetInt();
	face->bMouthX = mouthXY[0].GetInt();
	face->bMouthY = mouthXY[1].GetInt();
	
	return face;
}
