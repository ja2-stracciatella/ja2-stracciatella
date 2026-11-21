// Profiles written by the 0.22 releases. The record is the save version 104
// one without its voice and slot state (664 bytes). 0.22 kept the voice in
// ubSuspiciousDeath instead, as an offset from the first I.M.P. profile, whose
// voice it was: before voices were decoupled, profile 51 spoke with voice 51
// (SaveIMPPlayerProfiles in 0.22's SaveLoadGame.cc).

import { PROFILE_FIELDS } from "../model/fields.ts";
import type { Profile } from "../model/profile.ts";
import { binaryImporter } from "./binary.ts";

const PLAYER_GENERATED_CHARACTER_ID = 51;

const spec = {
	id: "0.22",
	label: "0.22 binary",
	fields: PROFILE_FIELDS.filter(([name]) => name !== "ubVoiceId" && name !== "impSlotState"),
	voice: (p: Profile) => PLAYER_GENERATED_CHARACTER_ID + (p.ubSuspiciousDeath as number),
};

export const v022 = binaryImporter(spec);
export const v022x32 = binaryImporter({ ...spec, bits: 32 });
