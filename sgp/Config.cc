#include "Config.h"
#include "MemMan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Types.h"
#include "Debug.h"

#define BUFFER_SIZE 1024

struct ConfigEntry
{
	char*        key;
	char*        value;
	ConfigEntry* next;
};

static ConfigEntry* config_head;

static char* Trim(char* string)
{
	char* helper = string + strlen(string) - 1;

	while (*helper == ' ')
	{
		*helper-- = '\0';
	}
	helper = string;
	while (*helper == ' ')
		helper++;
	return helper;
}


static void ConfigUpdateEntry(const char* key, const char* value)
{
	for (ConfigEntry* cfg_pointer = config_head; cfg_pointer != NULL; cfg_pointer = cfg_pointer->next)
	{
		if (strcmp(cfg_pointer->key, key) == 0)
		{
			ConfigSetValue(cfg_pointer, value);
			return;
		}
	}

	fprintf(stderr, "Key %s is not registered. Discarding...\n", key);
}


ConfigEntry* ConfigRegisterKey(const char* const key)
try
{
	ConfigEntry* const new_config = MALLOC(ConfigEntry);
	new_config->key   = strdup(key);
	new_config->value = NULL;
	new_config->next  = config_head;
	config_head       = new_config;
	return new_config;
}
catch (...) { return 0; }


void ConfigSetValue(ConfigEntry* const entry, const char* const value)
{
	free(entry->value);
	entry->value = strdup(value);
}


const char* ConfigGetValue(const ConfigEntry* const cfg)
{
	return cfg->value;
}


static void ConfigValueCheck(void)
{
	for (const ConfigEntry* cfg_pointer = config_head; cfg_pointer != NULL; cfg_pointer = cfg_pointer->next)
	{
		if (cfg_pointer->value == NULL)
		{
			fprintf(stderr, "Warning: Key %s has no value set.\n", cfg_pointer->key);
		}
	}
}


int ConfigParseFile(const char* file)
{
	Assert(file != NULL);

	FILE* configfile  = fopen(file, "r");

	if (configfile == NULL)
		return 1;

	char string[BUFFER_SIZE];

	while (fgets(string, lengthof(string), configfile) != NULL)
	{
		char key[BUFFER_SIZE];
		char value[BUFFER_SIZE];

		if (sscanf(string, "%s = %[A-z 0-9/\\_.:-]", key, value) == 2)
		{
			const char* trim_value = Trim(value);
			ConfigUpdateEntry(key, trim_value);
		}
	}
	fclose(configfile);
	ConfigValueCheck();
	return 0;
}
