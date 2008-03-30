#include "Config.h"
#include "MemMan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Types.h"
#include "Debug.h"

#define BUFFER_SIZE 1024

struct config_entry
{
	char* key;
	char* value;
	struct config_entry* next;
};

static config_entry* config_head;

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
	for (config_entry* cfg_pointer = config_head; cfg_pointer != NULL; cfg_pointer = cfg_pointer->next)
	{
		if (strcmp(cfg_pointer->key, key) == 0)
		{
			free(cfg_pointer->value);
			cfg_pointer->value = strdup(value);
			return;
		}
	}

	fprintf(stderr, "Key %s is not registered. Discarding...\n", key);
}


const config_entry* ConfigRegisterKey(const char* key)
{
	config_entry* const new_config = MALLOC(config_entry);
	if (new_config != NULL)
	{
		new_config->key   = strdup(key);
		new_config->value = NULL;
		new_config->next  = config_head;
		config_head       = new_config;
	}
	return new_config;
}


const char* ConfigGetValue(const config_entry* key)
{
	for (const config_entry* cfg_pointer = config_head; cfg_pointer != NULL; cfg_pointer = cfg_pointer->next)
	{
		if (cfg_pointer == key)
		{
			return cfg_pointer->value;
		}
	}

	return NULL;
}


static void ConfigValueCheck(void)
{
	for (const config_entry* cfg_pointer = config_head; cfg_pointer != NULL; cfg_pointer = cfg_pointer->next)
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
