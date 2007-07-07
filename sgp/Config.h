#ifndef C_CONFIG_H
#define C_CONFIG_H

typedef struct config_entry config_entry;

/* Registers a key. ConfigParseFile will only add key/value-pairs
 * which are registered
 */
const config_entry* ConfigRegisterKey(const char* key);

/* Returns the value associated with registered key
 */
const char* ConfigGetValue(const config_entry* key);

/* Parse file, adding all registered key/value-pairs it finds.
 */
int ConfigParseFile(const char* file);

#endif
