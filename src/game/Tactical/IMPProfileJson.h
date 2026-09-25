#pragma once

#include "Item_Types.h"
#include "Soldier_Control.h"
#include "Soldier_Profile_Type.h"

#include <string_theory/string>

/* A saved I.M.P. is kept as JSON, so it does not depend on how the compiler
 * lays out MERCPROFILESTRUCT, and a field added later is simply a key that an
 * older file lacks. Items are named by their internal names, so the file still
 * means the same items when a mod numbers them differently.
 *
 * Raise the version when a key changes its meaning. A key that is only added
 * does not need it: read it as optional instead. */
constexpr int IMP_PROFILE_JSON_VERSION = 1;

/* Writes the profile and the inventory the character carries. Empty slots are
 * left out, and so are the few pseudo items that only exist on the map. */
ST::string SerializeIMPProfile(MERCPROFILESTRUCT const& profile, OBJECTTYPE const (&inv)[NUM_INV_SLOTS]);

/* Reads what SerializeIMPProfile wrote. Throws DataError, naming the key, if
 * the file is not a profile this game can take on: a key is missing, has the
 * wrong type or a value out of range, or names an item the game does not have.
 * The outputs are only written once the whole file has been read. */
void DeserializeIMPProfile(ST::string const& json, MERCPROFILESTRUCT& profile, OBJECTTYPE (&inv)[NUM_INV_SLOTS]);
