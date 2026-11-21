import { readFileSync } from "node:fs";
import { join } from "node:path";
import { describe, expect, it } from "vitest";
import items from "virtual:items";
import { ItemTable } from "../gamedata/items.ts";
import { PROFILE_KEYS, validate } from "../model/profile.ts";
import { importerFor } from "./registry.ts";
import { v104 } from "./v104.ts";

const table = new ItemTable(items);
const fixture = (name: string) => new Uint8Array(readFileSync(join(import.meta.dirname, "../../merc-profiles/binary-v104", `mercprofile.${name}`)));

describe("save version 104", () => {
	it("is a 672 byte record, file 1356", () => {
		expect(v104.recordSize).toBe(672);
		expect(v104.size).toBe(1356);
		expect(importerFor(1356)).toBe(v104);
	});

	describe.each(["Adrian", "Clone", "Twin", "Dbg1", "Dbg18"])("%s", (name) => {
		const { doc, warnings, lostNames } = v104.read(fixture(name), table);
		it("reads without warnings or lost names", () => {
			expect(warnings).toEqual([]);
			expect(lostNames).toEqual([]);
		});
		it("has exactly the keys of the format", () => expect(Object.keys(doc.profile).sort()).toEqual([...PROFILE_KEYS].sort()));
		it("passes the game's checks", () => expect(validate(doc, table)).toEqual([]));
		it("is the merc the file is named after", () => expect(doc.profile.zNickname).toBe(name));
		it("keeps the voice from its own field", () => expect(doc.profile.ubVoiceId).toBeGreaterThanOrEqual(51));
	});
});
