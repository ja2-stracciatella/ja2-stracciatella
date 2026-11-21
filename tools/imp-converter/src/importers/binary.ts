// What every old binary profile shares: a record laid out as MERCPROFILESTRUCT
// lay in memory, then the 19 carried objects. A layout only differs in which
// fields its record has, where it kept the voice, and whether a 32 or a 64-bit
// build wrote it: the strings in the record are smaller in a 32-bit one. The
// objects hold no pointer, so they are the same in both.

import { type Bits, type FieldDef, layout, read, type SavedString, type Value } from "../binary/layout.ts";
import type { ItemTable } from "../gamedata/items.ts";
import { NOT_IN_JSON } from "../model/fields.ts";
import type { Json, Profile } from "../model/profile.ts";
import { readInventory, SAVED_OBJECT_SIZE } from "./inventory.ts";
import type { Import, Importer } from "./types.ts";

const STRINGS = new Set(["zName", "zNickname", "PANTS", "VEST", "SKIN", "HAIR"]);
const NUM_INV_SLOTS = 19;

export function binaryImporter(o: {
	id: string;
	label: string;
	fields: readonly FieldDef[];
	bits?: Bits;
	/** For a layout without ubVoiceId: the voice, from the rest of the profile. */
	voice?: (p: Profile) => number;
}): Importer {
	const bits = o.bits ?? 64;
	const record = layout(o.fields, bits);
	return {
		id: bits === 64 ? o.id : `${o.id}-32`,
		label: bits === 64 ? o.label : `${o.label}, 32-bit`,
		size: record.size + NUM_INV_SLOTS * SAVED_OBJECT_SIZE,
		recordSize: record.size,

		read(bytes: Uint8Array, items: ItemTable): Import {
			const values = read(record, bytes);
			const warnings: string[] = [];
			const lostNames: string[] = [];
			const profile: Profile = {};
			for (const [key, value] of Object.entries(values)) {
				if (NOT_IN_JSON.has(key)) continue;
				if (STRINGS.has(key)) {
					const s = value as SavedString;
					if (s.lost) lostNames.push(key);
					profile[key] = s.text;
				} else profile[key] = value as Exclude<Value, SavedString> as Json;
			}
			if (o.voice) profile.ubVoiceId = o.voice(profile);
			profile.inv = (values.inv as number[]).map((n, slot) => {
				const item = items.index(n);
				if (item) return item.name;
				warnings.push(`profile.inv[${slot}]: item ${n} is unknown to this game, left empty`);
				return "NOTHING";
			});
			const inventory = readInventory(bytes, record.size, items, warnings);
			return { doc: { version: 1, profile, inventory }, warnings, lostNames };
		},
	};
}
