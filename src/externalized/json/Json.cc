#include "Json.h"

#include "string_theory/format"

JsonValue JsonValue::deserialize(const ST::string& str) {
	auto r = RJsonValue_deserialize(str.c_str());
	throwRustError(!r);
	return JsonValue(r);
}

bool JsonValue::isVec() const {
	return RJsonValue_isArray(m_value.get());
}

std::vector<JsonValue> JsonValue::toVec() const {
	RustPointer<RJsonArray> array(RJsonValue_toArray(m_value.get()));
	throwRustError(!array);
	auto length = RJsonArray_length(array.get());
	std::vector<JsonValue> vec;
	vec.reserve(length);
	for (size_t i = 0; i < length; i++) {
		RustPointer<RJsonValue> val(RJsonArray_get(array.get(), i));
		throwRustError(!val);
		vec.emplace_back(val.release());
	}
	return vec;
}

bool JsonValue::isObject() const {
	return RJsonValue_isObject(m_value.get());
}

JsonObject JsonValue::toObject() const {
	RustPointer<RJsonObject> obj(RJsonValue_toObject(m_value.get()));
	throwRustError(!obj);
	return JsonObject(obj.release());
}

bool JsonValue::isString() const {
	return RJsonValue_isString(m_value.get());
}

ST::string JsonValue::toString() const {
	RustPointer<char> str(RJsonValue_toString(m_value.get()));
	throwRustError(!str);
	return str.get();
}

bool JsonValue::isInt() const {
	return RJsonValue_isInt(m_value.get());
}

int JsonValue::toInt() const {
	bool success = false;
	auto val = RJsonValue_toInt64(m_value.get(), &success);
	throwRustError(!success);
	return val;
}

bool JsonValue::isUInt() const {
	if (!isInt()) {
		return false;
	}
	bool success = false;
	auto val = RJsonValue_toInt64(m_value.get(), &success);
	throwRustError(!success);
	return val >= 0;
}


unsigned int JsonValue::toUInt() const {
	bool success = false;
	auto val = RJsonValue_toInt64(m_value.get(), &success);
	throwRustError(!success);
	if (val < 0) {
		throw std::runtime_error(ST::format("expected uint, got {}", val).c_str());
	}
	return val;
}

bool JsonValue::isBool() const {
	return RJsonValue_isBool(m_value.get());
}

bool JsonValue::toBool() const {
	bool success = false;
	auto val = RJsonValue_toBool(m_value.get(), &success);
	throwRustError(!success);
	return val;
}

bool JsonValue::isDouble() const {
	return RJsonValue_isDouble(m_value.get());
}

double JsonValue::toDouble() const {
	bool success = false;
	auto val = RJsonValue_toDouble(m_value.get(), &success);
	throwRustError(!success);
	return val;
}

uintptr_t JsonArray::size() const {
	return RJsonArray_length(m_value.get());
}

JsonValue JsonArray::get(uintptr_t idx) const {
	RustPointer<RJsonValue> v(RJsonArray_get(m_value.get(), idx));
	throwRustError(!v);
	return JsonValue(v.release());
}

void JsonArray::push(JsonValue val) {
	return RJsonArray_push(m_value.get(), val.get());
}

JsonValue JsonArray::toValue() const {
	return JsonValue(RJsonArray_toValue(m_value.get()));
}

ST::string JsonValue::serialize(bool pretty) const {
	RustPointer<char> str(RJsonValue_serialize(m_value.get(), pretty));
	throwRustError(!str.get());
	return str.get();
}

ST::string JsonObject::GetString(const char *name) const
{
    return GetValue(name).toString();
}

int JsonObject::GetInt(const char *name) const
{
    return GetValue(name).toInt();
}

unsigned int JsonObject::GetUInt(const char* name) const
{
    return GetValue(name).toUInt();
}

bool JsonObject::GetBool(const char *name) const
{
    return GetValue(name).toBool();
}

double JsonObject::GetDouble(const char *name) const
{
    return GetValue(name).toDouble();
}

JsonValue JsonObject::GetValue(const char *name) const
{
    RustPointer<RJsonValue> prop(RJsonObject_get(m_value.get(), name));
    throwRustError(!prop);
    return JsonValue(prop.release());
}

JsonValue JsonObject::operator[](const char *name) const
{
    return GetValue(name);
}

bool JsonObject::getOptionalBool(const char *name, bool defaultValue) const
{
    if(has(name))
    {
        return GetBool(name);
    }
    else
    {
        return defaultValue;
    }
}

double JsonObject::getOptionalDouble(const char* name, double defaultValue) const
{
    if (has(name))
    {
        return GetDouble(name);
    }
    else
    {
        return defaultValue;
    }
}

int JsonObject::getOptionalInt(const char* name, int defaultValue) const
{
    if (has(name))
    {
        return GetInt(name);
    }
    else
    {
        return defaultValue;
    }
}

unsigned int JsonObject::getOptionalUInt(const char* name, unsigned int defaultValue) const
{
    if (has(name))
    {
        return GetUInt(name);
    }
    else
    {
        return defaultValue;
    }
}

const ST::string JsonObject::getOptionalString(const char* name, const ST::string defaultValue) const
{
    if(has(name))
    {
        return GetString(name);
    }
    else
    {
        return defaultValue;
    }
}

bool JsonObject::has(const char *name) const
{
    return RJsonObject_has(m_value.get(), name);
}

std::vector<ST::string> JsonObject::keys() const
{
	RustPointer<VecCString> rKeys(RJsonObject_keys(m_value.get()));
	throwRustError(!rKeys);
	auto size = VecCString_len(rKeys.get());
	std::vector<ST::string> keys;
	for (uintptr_t i = 0; i < size; i++) {
		RustPointer<char> key(VecCString_get(rKeys.get(), i));
		throwRustError(!key);
		keys.emplace_back(key.get());
	}
    return keys;
}

void JsonObject::set(const char *name, JsonValue value)
{
	RJsonObject_set(m_value.get(), name, value.get());
}

JsonValue JsonObject::toValue() const
{
	return RJsonObject_toValue(m_value.get());
}

