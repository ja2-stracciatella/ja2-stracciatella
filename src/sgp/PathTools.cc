#include "PathTools.h"
#include <string.h>
#include <dirent.h>
#include <stdint.h> // for uint8_t instead of "sgp/Types.h"
#include <boost/filesystem.hpp>

/** Join two path components. */
std::string PathTools::joinPaths(const std::string &first, const char *second)
{
		std::string result = first;
		if((result.length() == 0) || (result[result.length()-1] != PATH_SEPARATOR))
		{
				if(second[0] != PATH_SEPARATOR)
				{
						result += PATH_SEPARATOR;
				}
		}
		result += second;
		return result;
}

/** Join two path components. */
std::string PathTools::joinPaths(const std::string &first, const std::string &second)
{
		return joinPaths(first, second.c_str());
}

/** Join two path components. */
std::string PathTools::joinPaths(const char *first, const char *second)
{
		return joinPaths(std::string(first), second);
}



#if CASE_SENSITIVE_FS
bool PathTools::findObjectCaseInsensitiveFullPath(const char *directory, const char *name, bool lookForFiles, bool lookForSubdirs, std::string &foundNameFullPath)
{
	if(PathTools::findObjectCaseInsensitive(directory, name, lookForFiles, lookForSubdirs, foundNameFullPath))
	{
		foundNameFullPath = PathTools::joinPaths(directory, foundNameFullPath);
		return true;
	}
	return false;
}

/**
 * Find an object (file or subdirectory) in the given directory in case-independent manner.
 * @return true when found, return the found name using foundName. */
bool PathTools::findObjectCaseInsensitive(const char *directory, const char *name, bool lookForFiles, bool lookForSubdirs, std::string &foundName)
{
	bool result = false;

	// if name contains directories, than we have to find actual case-sensitive name of the directory
	// and only then look for a file
	const char *splitter = strstr(name, "/");
	int dirNameLen = (int)(splitter - name);
	if(splitter && (dirNameLen > 0) && splitter[1] != 0)
	{
		// we have directory in the name
		// let's find its correct name first
		char newDirectory[128];
		std::string actualSubdirName;
		strncpy(newDirectory, name, sizeof(newDirectory));
		newDirectory[dirNameLen] = 0;

		if(findObjectCaseInsensitive(directory, newDirectory, false, true, actualSubdirName))
		{
			// found subdirectory; let's continue the full search
			std::string pathInSubdir;
			std::string newDirectory = PathTools::joinPaths(directory, actualSubdirName.c_str());
			if(findObjectCaseInsensitive(newDirectory.c_str(), splitter + 1,
							lookForFiles, lookForSubdirs, pathInSubdir))
			{
				// found name in subdir
				foundName = PathTools::joinPaths(actualSubdirName, pathInSubdir);
				result = true;
			}
		}
	}
	else
	{
		// name contains only file, no directories
		DIR *d;
		struct dirent *entry;
		uint8_t objectTypes = (lookForFiles ? DT_REG : 0) | (lookForSubdirs ? DT_DIR : 0);

		d = opendir(directory);
		if (d)
		{
			while ((entry = readdir(d)) != NULL)
			{
				if((entry->d_type & objectTypes)
					&& !strcasecmp(name, entry->d_name))
				{
					foundName = entry->d_name;
					result = true;
				}
			}
			closedir(d);
		}
	}

	// SLOGI(DEBUG_TAG_FILEMAN,"Looking for %s/[ %s ] : %s", directory, name, result ? "success" : "failure");
	return result;
}
#endif


/** Get parent path (e.g. directory path from the full path). */
std::string PathTools::getParentPath(const std::string &_path, bool absolute)
{
	boost::filesystem::path path(_path);
	boost::filesystem::path parent = path.parent_path();
	if(absolute)
	{
		parent = boost::filesystem::absolute(parent);
	}
	return parent.string();
}



