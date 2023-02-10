#pragma once

#include "MercSystem.h"
#include <Json.h>

#include <stdint.h>

struct RPCSmallFaceModel
{
public:
	uint8_t ubProfileID;
	int8_t bEyesX;
	int8_t bEyesY;
	int8_t bMouthX;
	int8_t bMouthY;

	static RPCSmallFaceModel* deserialize(const JsonValue& json, const MercSystem* mercSystem);
};
