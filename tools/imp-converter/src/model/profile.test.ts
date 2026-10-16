import { readFileSync } from "node:fs";
import { join } from "node:path";
import { describe, expect, it } from "vitest";
import items from "virtual:items";
import { layout } from "../binary/layout.ts";
import { ItemTable } from "../gamedata/items.ts";
import { readJson } from "../importers/json.ts";
import { v022 } from "../importers/v022.ts";
import { type ProfileDoc, validate } from "./profile.ts";

const table = new ItemTable(items);
const john = () => readJson(readFileSync(join(import.meta.dirname, "../../merc-profiles/json/mercprofile.John.json"), "utf8")).doc;
const errorsAfter = (change: (d: ProfileDoc) => void) => {
	const d = john();
	change(d);
	return validate(d, table);
};

describe("layout", () => {
	it("aligns each field to its size and pads the struct to its widest member", () => {
		const l = layout([["a", "u8"], ["b", "u32"], ["c", "u8"], ["d", "i16", [3]]]);
		expect(l.fields.map((f) => f.offset)).toEqual([0, 4, 8, 10]);
		expect(l.size).toBe(16);
	});
	it("aligns strings to 8 and sectors to 2", () => {
		const l = layout([["a", "u8"], ["s", "string"], ["b", "u8"], ["t", "sector"]]);
		expect(l.fields.map((f) => f.offset)).toEqual([0, 8, 40, 42]);
		expect(l.size).toBe(48);
	});
});

describe("validate", () => {
	it("accepts the game's own file", () => expect(validate(john(), table)).toEqual([]));
	it("refuses values outside what a player could make", () => {
		expect(errorsAfter((d) => { d.profile.bAgility = 101; })).toEqual(["profile.bAgility: 101 is not in the range 1..100"]);
		expect(errorsAfter((d) => { d.profile.bLife = 93; })).toEqual(["profile.bLife: 93 is not in the range 1..92"]);
		expect(errorsAfter((d) => { d.profile.bSkillTrait = 16; })).toEqual(["profile.bSkillTrait: 16 is not in the range 0..15"]);
	});
	it("refuses values outside the field's C type", () => {
		expect(errorsAfter((d) => { d.profile.usKills = 70000; })).toEqual(["profile.usKills: 70000 is not in the range 0..65535"]);
	});
	it("refuses names longer than the savegame has room for", () => {
		expect(errorsAfter((d) => { d.profile.zNickname = "Johnathan!"; })).toEqual(["profile.zNickname: longer than 9 characters"]);
		expect(errorsAfter((d) => { d.profile.zNickname = ""; })).toEqual(["profile.zNickname: must not be empty"]);
	});
	it("refuses unknown and uncarriable items", () => {
		expect(errorsAfter((d) => { d.inventory[0]!.item = "NO_SUCH_THING"; })).toEqual(["inventory[0].item: there is no item 'NO_SUCH_THING'"]);
		expect(errorsAfter((d) => { d.inventory[0]!.item = "ACTION_ITEM"; })).toEqual(["inventory[0].item: 'ACTION_ITEM' cannot be carried"]);
	});
	it("refuses more ammunition than a magazine holds", () => {
		const errors = errorsAfter((d) => { d.inventory.find((o) => o.slot === "SMALLPOCK1POS")!.shotsLeft = [21, 20]; });
		expect(errors).toEqual([expect.stringMatching(/shotsLeft\[0\]: 21 is not in the range 0\.\.20/)]);
	});
	it("refuses a slot taken twice", () => {
		expect(errorsAfter((d) => { d.inventory[1]!.slot = d.inventory[0]!.slot; })).toEqual(["inventory[1].slot: HELMETPOS is taken twice"]);
	});
});

describe("names an old file lost", () => {
	it("are reported and left empty", () => {
		const bytes = new Uint8Array(readFileSync(join(import.meta.dirname, "../../merc-profiles/binary/mercprofile.John")));
		// zName: a string of 20 characters, whose text was on the heap.
		new DataView(bytes.buffer).setBigUint64(8, 20n, true);
		const { doc, lostNames } = v022.read(bytes, table);
		expect(lostNames).toEqual(["zName"]);
		expect(doc.profile.zName).toBe("");
	});
});
