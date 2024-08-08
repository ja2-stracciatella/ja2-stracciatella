#pragma once

#include "RustInterface.h"

#include <string_theory/string>
#include <vector>

class JsonObject;

class JsonValue {
	public:
		// Takes ownership of RJsonValue
		JsonValue(RJsonValue* value) : m_value(RustPointer<RJsonValue>(value)) {}
		JsonValue(int value) : m_value(RustPointer<RJsonValue>(RJsonValue_fromInt(value))) {}
		JsonValue(unsigned int value) : m_value(RustPointer<RJsonValue>(RJsonValue_fromUInt(value))) {}
		JsonValue(double value) : m_value(RustPointer<RJsonValue>(RJsonValue_fromDouble(value))) {}
		JsonValue(bool value) : m_value(RustPointer<RJsonValue>(RJsonValue_fromBool(value))) {}
		JsonValue(const ST::string& value) : m_value(RustPointer<RJsonValue>(RJsonValue_fromString(value.c_str()))) {}

		static JsonValue deserialize(const ST::string& str);
		static JsonValue deserialize(const ST::string& vanillaStr, const ST::string& patchStr);

		ST::string serialize(bool pretty = false) const;
		bool isInt() const;
		int toInt() const;
		bool isUInt() const;
		unsigned int toUInt() const;
		bool isBool() const;
		bool toBool() const;
		bool isDouble() const;
		double toDouble() const;
		bool isString() const;
		ST::string toString() const;
		bool isVec() const;
		std::vector<JsonValue> toVec() const;
		bool isObject() const;
		JsonObject toObject() const;

		const RJsonValue* get() const {
			return m_value.get();
		}
	private:
		RustPointer<RJsonValue> m_value;
};

class JsonArray
{
	public:
		JsonArray() {
			m_value.reset(RJsonArray_new());
		}
		JsonArray(RJsonArray* value)
		{
			m_value.reset(value);
		}

		uintptr_t size() const;
		JsonValue get(uintptr_t idx) const;
		void push(JsonValue val);
		JsonValue toValue() const;
	private:
		RustPointer<RJsonArray> m_value;
};

class JsonObject
{
	public:
		JsonObject() {
			m_value.reset(RJsonObject_new());
		}
		JsonObject(RJsonObject* value)
		{
			m_value.reset(value);
		}

		ST::string GetString(const char *name) const;
		int GetInt(const char *name) const;
		unsigned int GetUInt(const char* name) const;
		bool GetBool(const char *name) const;
		double GetDouble(const char *name) const;
		JsonValue GetValue(const char *name) const;
		JsonValue operator[](const char *name) const;
		bool getOptionalBool(const char *name, bool defaultValue = false) const;
		double getOptionalDouble(const char* name, double defaultValue = 0) const;
		int getOptionalInt(const char* name, int defaultValue = 0) const;
		unsigned int getOptionalUInt(const char* name, unsigned int defaultValue = 0) const;
		const ST::string getOptionalString(const char *name) const;
		bool has(const char* name) const;
		std::vector<ST::string> keys() const;
		void set(const char* name, JsonValue value);
		JsonValue toValue() const;
	protected:
		RustPointer<RJsonObject> m_value;
};
