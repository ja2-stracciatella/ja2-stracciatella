#pragma once

#include "rapidjson/document.h"
#include <string_theory/string>

class JsonObject
{
public:
	JsonObject(rapidjson::Document::AllocatorType& allocator)
		:m_alloc(allocator)
	{
		m_value.SetObject();
	}

	void AddMember(const char *name, const char *value) { m_value.AddMember(rapidjson::StringRef(name), rapidjson::StringRef(value), m_alloc); }
	void AddMember(const char *name, int         value) { m_value.AddMember<int>(rapidjson::StringRef(name), value, m_alloc); }
	void AddMember(const char *name, uint8_t     value) { m_value.AddMember<uint8_t>(rapidjson::StringRef(name), value, m_alloc); }
	void AddMember(const char *name, uint16_t    value) { m_value.AddMember<uint16_t>(rapidjson::StringRef(name), value, m_alloc); }
	void AddMember(const char *name, uint32_t    value) { m_value.AddMember<uint32_t>(rapidjson::StringRef(name), value, m_alloc); }
	void AddMember(const char *name, bool        value) { m_value.AddMember<bool>(rapidjson::StringRef(name), value, m_alloc); }
	void AddMember(const char *name, rapidjson::Value &value) { m_value.AddMember(rapidjson::StringRef(name), value, m_alloc); }

	void AddMember(const char *name, const ST::string &value)
	{
		m_value.AddMember(rapidjson::StringRef(name), rapidjson::StringRef(value.c_str()), m_alloc);
	}

	void addOptionalBool(const char *name, bool value)
	{
		if(value)
		{
			AddMember(name, value);
		}
	}

	rapidjson::Value& getValue()
	{
		return m_value;
	}

	rapidjson::Document::AllocatorType& getAllocator()
	{
		return m_alloc;
	}
protected:
	rapidjson::Value m_value;
	rapidjson::Document::AllocatorType& m_alloc;
};

class JsonObjectReader
{
public:
	JsonObjectReader(const rapidjson::Value &value)
		:m_value(value)
	{
	}

	const char* GetString(const char *name) const
	{
		return m_value[name].GetString();
	}

	int GetInt(const char *name) const
	{
		return m_value[name].GetInt();
	}

	unsigned int GetUInt(const char* name) const
	{
		return m_value[name].GetUint();
	}

	bool GetBool(const char *name) const
	{
		return m_value[name].GetBool();
	}

	double GetDouble(const char *name) const
	{
		return m_value[name].GetDouble();
	}

	const rapidjson::Value &GetValue(const char *name) const
	{
		return m_value[name];
	}

	bool getOptionalBool(const char *name, bool defaultValue = false) const
	{
		if(m_value.HasMember(name))
		{
			return m_value[name].GetBool();
		}
		else
		{
			return defaultValue;
		}
	}

	double getOptionalDouble(const char* name, double defaultValue = 0) const
	{
		if (m_value.HasMember(name))
		{
			return m_value[name].GetDouble();
		}
		else
		{
			return defaultValue;
		}
	}

	int getOptionalInt(const char* name, int defaultValue = 0) const
	{
		if (m_value.HasMember(name))
		{
			return m_value[name].GetInt();
		}
		else
		{
			return defaultValue;
		}
	}

	unsigned int getOptionalUInt(const char* name, unsigned int defaultValue = 0) const
	{
		if (m_value.HasMember(name))
		{
			return m_value[name].GetUint();
		}
		else
		{
			return defaultValue;
		}
	}

	const char * getOptionalString(const char *name) const
	{
		if(m_value.HasMember(name))
		{
			return m_value[name].GetString();
		}
		else
		{
			return NULL;
		}
	}

protected:
	const rapidjson::Value &m_value;
};
