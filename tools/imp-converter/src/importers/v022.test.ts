import { readFileSync } from "node:fs";
import { join } from "node:path";
import { describe, expect, it } from "vitest";
import items from "virtual:items";
import { layout } from "../binary/layout.ts";
import { ItemTable } from "../gamedata/items.ts";
import { PROFILE_FIELDS } from "../model/fields.ts";
import { PROFILE_KEYS, type ProfileDoc, serialize, validate } from "../model/profile.ts";
import { importerFor } from "./registry.ts";
import { v022 } from "./v022.ts";

const table = new ItemTable(items);
const fixture = (path: string) => new Uint8Array(readFileSync(join(import.meta.dirname, "../../merc-profiles", path)));
const game = (name: string) => JSON.parse(new TextDecoder().decode(fixture(`json/mercprofile.${name}.json`))) as ProfileDoc;

describe("layouts", () => {
	// The sizes the C++ in #2523 asserts for the same structs.
	it("save version 104 record is 672 bytes", () => expect(layout(PROFILE_FIELDS).size).toBe(672));
	it("0.22 record is 664 bytes, file 1348", () => {
		expect(v022.recordSize).toBe(664);
		expect(v022.size).toBe(1348);
		expect(importerFor(1348)).toBe(v022);
	});
});

// The binary files are from about a game day later than the JSON the game
// wrote for the same mercs: kills, experience, stat progress and some carried
// objects moved on in between. So only what play does not change is compared;
// a byte-exact comparison needs both files written from one save.
const UNCHANGED_BY_PLAY = [
	"zName", "zNickname", "bSex", "ubBodyType", "uiBodyTypeSubFlags", "ubFaceIndex", "usEyesX", "usEyesY", "usMouthX", "usMouthY",
	"uiBlinkFrequency", "uiExpressionFrequency", "ubVoiceId", "PANTS", "VEST", "SKIN", "HAIR", "bEvolution",
	"bPersonalityTrait", "bSkillTrait", "bSkillTrait2", "bAttitude", "bSexist", "ubSuspiciousDeath",
	"bBuddy", "bHated", "bMercOpinion", "bRace", "bRacist", "bNationality", "bAppearance", "bRefinement", "bHatedNationality",
];
// Objects still carried as they were, checked field by field.
const UNCHANGED_SLOTS: Record<string, string[]> = {
	John: ["HELMETPOS", "LEGPOS", "HEAD1POS", "HEAD2POS", "BIGPOCK1POS", "BIGPOCK2POS", "SMALLPOCK1POS", "SMALLPOCK2POS", "SMALLPOCK3POS", "SMALLPOCK4POS", "SMALLPOCK5POS", "SMALLPOCK8POS"],
	Anna: ["HELMETPOS", "VESTPOS", "LEGPOS", "HEAD1POS", "HEAD2POS", "BIGPOCK1POS", "SMALLPOCK2POS", "SMALLPOCK4POS", "SMALLPOCK6POS", "SMALLPOCK7POS", "SMALLPOCK8POS"],
};

describe.each(["John", "Anna"])("0.22 %s", (name) => {
	const { doc, warnings, lostNames } = v022.read(fixture(`binary/mercprofile.${name}`), table);
	const expected = game(name);
	const slot = (d: ProfileDoc, s: string) => d.inventory.find((o) => o.slot === s);

	it("reads without warnings or lost names", () => {
		expect(warnings).toEqual([]);
		expect(lostNames).toEqual([]);
	});
	it("has exactly the keys of the format", () => {
		expect(Object.keys(doc.profile).sort()).toEqual([...PROFILE_KEYS].sort());
		expect(Object.keys(expected.profile).sort()).toEqual([...PROFILE_KEYS].sort());
	});
	it("passes the game's checks", () => expect(validate(doc, table)).toEqual([]));
	it("takes the voice from where 0.22 kept it", () => expect(doc.profile.ubVoiceId).toBe(expected.profile.ubVoiceId));
	it("matches the game on what play does not change", () => {
		const pick = (d: ProfileDoc) => Object.fromEntries(UNCHANGED_BY_PLAY.map((k) => [k, d.profile[k]]));
		expect(pick(doc)).toEqual(pick(expected));
	});
	it("writes unchanged objects as the game does", () => {
		for (const s of UNCHANGED_SLOTS[name]!) expect(slot(doc, s), s).toEqual(slot(expected, s));
	});
	it("writes the game's formatting", () => {
		const text = new TextDecoder().decode(fixture(`json/mercprofile.${name}.json`));
		expect(serialize(expected)).toBe(text.endsWith("\n") ? text : text + "\n");
	});
});
