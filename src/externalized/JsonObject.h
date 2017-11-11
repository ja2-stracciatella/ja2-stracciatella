#pragma once

#include <string>

#include "rapidjson/document.h"

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
	void AddMember(const char *name, bool        value) { m_value.AddMember<bool>(rapidjson::StringRef(name), value, m_alloc); }

	void AddMember(const char *name, const std::string &value)
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

	bool GetBool(const char *name) const
	{
		return m_value[name].GetBool();
	}

	bool getOptionalBool(const char *name) const
	{
		if(m_value.HasMember(name))
		{
			return m_value[name].GetBool();
		}
		else
		{
			return false;
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
