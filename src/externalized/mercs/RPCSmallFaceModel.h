#pragma once
#include "MercSystem.h"
#include <rapidjson/document.h>
#include <stdint.h>

struct RPCSmallFaceModel
{
public:
	uint8_t ubProfileID;
	int8_t bEyesX;
	int8_t bEyesY;
	int8_t bMouthX;
	int8_t bMouthY;

	static RPCSmallFaceModel* deserialize(const rapidjson::Value& json, const MercSystem* mercSystem);
};