#include "JsonUtility.h"

// #include <iostream>
// #include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

/** Write list of strings to file. */
bool JsonUtility::writeToFile(const char *name, const std::vector<std::string> &strings)
{
  FILE *f = fopen(name, "wt");
  if(f)
  {
    char writeBuffer[65536];
    rapidjson::FileWriteStream os(f, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    writer.StartArray();
    for(std::vector<std::string>::const_iterator it = strings.begin(); it != strings.end(); ++it)
    {
      writer.String(it->c_str());
    }
    writer.EndArray();

    fputs("\n", f);
    return fclose(f) == 0;
  }
  return false;
}

/** Parse json to a list of strings. */
bool JsonUtility::parseJsonToListStrings(const char* jsonData, std::vector<std::string> &strings)
{
  rapidjson::Document document;
  if (document.Parse<0>(jsonData).HasParseError())
  {
    return false;
  }

  return parseListStrings(document, strings);
}

/** Parse value as list of strings. */
bool JsonUtility::parseListStrings(const rapidjson::Value &value, std::vector<std::string> &strings)
{
  if(value.IsArray()) {
    for (rapidjson::SizeType i = 0; i < value.Size(); i++)
    {
      strings.push_back(value[i].GetString());
    }
    return true;
  }
  return false;
}

