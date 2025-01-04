#pragma once

#include "Types.h"

// A very trivial implementation of a cache for SGPVObject.

// Please note: the cache key is not copied so the string must
// remain valid until the object was removed. Use only string
// literals as the key!
using cache_key_t = const char *;

// Returns a pointer to an already cached vobject if possible, otherwise
// tries to load the vobject.
SGPVObject * GetVObject(cache_key_t filename);

// Remove the vobject from the cache. It is safe to use this function for
// a vobject that was already removed or never added to the cache.
// Returns whether an object was actually removed.
bool RemoveVObject(cache_key_t filename);

// Blits a cached video object to a surface. Attempts to load the object if
// it is not found in the cache.
void BltVideoObject(SGPVSurface * dst, cache_key_t filename, UINT16 subIndex, int x, int y);
