#include "JsonUtility.h"

// #include <iostream>
// #include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/filestream.h"
#include "rapidjson/prettywriter.h"

/** Write list of strings to file. */
bool JsonUtility::writeToFile(const char *name, const std::vector<std::string> &strings)
{
  FILE *f = fopen(name, "wt");
  if(f)
  {
    rapidjson::FileStream os(f);
    rapidjson::PrettyWriter<rapidjson::FileStream> writer(os);
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

  const rapidjson::Value& a = document;
  for (rapidjson::SizeType i = 0; i < a.Size(); i++)
  {
    strings.push_back(a[i].GetString());
  }
  return true;
}
