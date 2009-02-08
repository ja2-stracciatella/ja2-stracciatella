#ifndef C_CONFIG_H
#define C_CONFIG_H

struct ConfigEntry;

/* Registers a key. ConfigParseFile will only add key/value-pairs
 * which are registered
 */
ConfigEntry* ConfigRegisterKey(const char* key);

/* Set the value of the config entry */
void ConfigSetValue(ConfigEntry*, const char* value);

/* Returns the value associated with registered key
 */
const char* ConfigGetValue(const ConfigEntry*);

/* Parse file, adding all registered key/value-pairs it finds.
 */
int ConfigParseFile(const char* file);

#endif
