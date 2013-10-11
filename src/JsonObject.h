#pragma once

#include "rapidjson/document.h"

class JsonObject
{
public:
  JsonObject(rapidjson::Document::AllocatorType& allocator)
    :m_alloc(allocator)
  {
    m_value.SetObject();
  }

  void AddMember(const char *name, const char *value) { m_value.AddMember(name, value, m_alloc); }
  void AddMember(const char *name, int         value) { m_value.AddMember(name, value, m_alloc); }
  void AddMember(const char *name, uint8_t     value) { m_value.AddMember(name, value, m_alloc); }
  void AddMember(const char *name, uint16_t    value) { m_value.AddMember(name, value, m_alloc); }
  void AddMember(const char *name, bool        value) { m_value.AddMember(name, value, m_alloc); }

  void AddMember(const char *name, const std::string &value)
  {
    m_value.AddMember(name, value.c_str(), m_alloc);
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

protected:
  const rapidjson::Value &m_value;
};
