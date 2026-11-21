import { readFileSync } from "node:fs";
import { join } from "node:path";
import { describe, expect, it } from "vitest";
import imp from "virtual:imp";
import items from "virtual:items";
import { ItemTable } from "../gamedata/items.ts";
import { PROFILE_KEYS, validate } from "../model/profile.ts";
import { importerFor } from "./registry.ts";
import { v021 } from "./v021.ts";

const table = new ItemTable(items);

describe("0.21", () => {
	it("is a 648 byte record, file 1332", () => {
		// The size #2523 asserts for ProfileV21.
		expect(v021.recordSize).toBe(648);
		expect(v021.size).toBe(1332);
		expect(importerFor(1332)).toBe(v021);
	});

	// Needle, the 0.21 profile #2523 was tested with.
	const { doc, warnings, lostNames } = v021.read(new Uint8Array(readFileSync(join(import.meta.dirname, "../../merc-profiles/binary/mercprofile.Needle"))), table);
	const p = doc.profile;
	it("reads without warnings or lost names", () => {
		expect(warnings).toEqual([]);
		expect(lostNames).toEqual([]);
	});
	it("has exactly the keys of the format", () => expect(Object.keys(p).sort()).toEqual([...PROFILE_KEYS].sort()));
	it("passes the game's checks", () => expect(validate(doc, table)).toEqual([]));
	it("is Ned Lee", () => {
		expect([p.zName, p.zNickname, p.bExpLevel, p.usKills]).toEqual(["Ned Lee", "Needle", 8, 311]);
	});
	it("has the portrait's own offsets and palettes, as the I.M.P. site set them", () => {
		const portrait = imp.portraits.find((o) => o.id === p.ubFaceIndex)!;
		expect([p.usEyesX, p.usEyesY]).toEqual(portrait.eyes);
		expect([p.usMouthX, p.usMouthY]).toEqual(portrait.mouth);
		expect([p.SKIN, p.HAIR]).toEqual([portrait.skin, portrait.hair]);
	});
	it("takes the voice from where 0.21 kept it", () => expect(p.ubVoiceId).toBe(51 + (p.ubSuspiciousDeath as number)));
});
