// The I.M.P. profile JSON format, version 1, as the game reads and writes it
// (Tactical/IMPProfileJson.cc): the profile under its struct field names, and
// the carried items by slot, named by their internal names.

import type { FieldDef, FieldType } from "../binary/layout.ts";
import { type ItemTable, NOTHING } from "../gamedata/items.ts";
import { NOT_IN_JSON, PROFILE_FIELDS } from "./fields.ts";

export const JSON_VERSION = 1;

export const SLOTS = [
	"HELMETPOS", "VESTPOS", "LEGPOS", "HEAD1POS", "HEAD2POS", "HANDPOS", "SECONDHANDPOS",
	"BIGPOCK1POS", "BIGPOCK2POS", "BIGPOCK3POS", "BIGPOCK4POS",
	"SMALLPOCK1POS", "SMALLPOCK2POS", "SMALLPOCK3POS", "SMALLPOCK4POS",
	"SMALLPOCK5POS", "SMALLPOCK6POS", "SMALLPOCK7POS", "SMALLPOCK8POS",
] as const;

export const MAX_OBJECTS_PER_SLOT = 8;
export const MAX_ATTACHMENTS = 4;
export const KEY_STATUSES = 6;
const NUM_PROFILES = 200;
const NAME_LENGTH = 30, NICKNAME_LENGTH = 10, PALETTE_LENGTH = 30;

export type Json = null | boolean | number | string | Json[] | { [key: string]: Json };
export type Profile = Record<string, Json>;

export interface Attachment { item: string; status: number }
export interface InventoryObject {
	slot: string;
	item: string;
	count: number;
	/** Per object for plain items and keys; one value for guns and money. */
	status?: number | number[];
	shotsLeft?: number | number[];
	ammoType?: number;
	ammoItem?: string;
	ammoStatus?: number;
	keyId?: number;
	amount?: number;
	attachments?: Attachment[];
	flags: number;
	mission: number;
	trap: number;
	imprintId: number;
	weight: number;
	used: number;
	[key: string]: unknown;
}

export interface ProfileDoc {
	version: number;
	profile: Profile;
	inventory: InventoryObject[];
	/** Keys this converter does not know, kept as they were. */
	[key: string]: unknown;
}

/** The JSON keys of the profile, in record order. */
export const PROFILE_KEYS = PROFILE_FIELDS.map(([name]) => name).filter((n) => !NOT_IN_JSON.has(n));

const RANGE: Record<Exclude<FieldType, "string" | "sector">, [number, number]> = {
	i8: [-128, 127], u8: [0, 255], bool: [0, 255], i16: [-32768, 32767], u16: [0, 65535],
	i32: [-2147483648, 2147483647], u32: [0, 4294967295],
};

// The ranges ValidateProfile checks beyond each field's C type.
const RULES: Record<string, [number, number]> = {
	bSex: [0, 1], ubBodyType: [0, 3],
	bLifeMax: [1, 100], bAgility: [1, 100], bDexterity: [1, 100], bStrength: [1, 100], bLeadership: [1, 100], bWisdom: [1, 100],
	bMarksmanship: [0, 100], bExplosive: [0, 100], bMechanical: [0, 100], bMedical: [0, 100], bExpLevel: [1, 10],
	bEvolution: [0, 2], bPersonalityTrait: [0, 7], bSkillTrait: [0, 15], bSkillTrait2: [0, 15], bAttitude: [0, 9], bSexist: [0, 3],
	ubVoiceId: [0, NUM_PROFILES - 1], bLearnToLike: [-1, NUM_PROFILES - 1], bLearnToHate: [-1, NUM_PROFILES - 1],
};
const PROFILE_ID_ARRAYS = new Set(["bBuddy", "bHated"]);

const utf16Length = (s: string) => s.length;
const utf8Length = (s: string) => new TextEncoder().encode(s).length;

/** What the game would refuse in this document, as "path: reason" lines, the
 * way its own reader words them. Empty when the game loads it. */
export function validate(doc: ProfileDoc, items: ItemTable): string[] {
	const errors: string[] = [];
	const fail = (path: string, what: string) => errors.push(`${path}: ${what}`);
	const inRange = (path: string, v: Json | undefined, min: number, max: number) => {
		if (typeof v !== "number" || !Number.isInteger(v)) fail(path, "expected an integer");
		else if (v < min || v > max) fail(path, `${v} is not in the range ${min}..${max}`);
	};

	if (doc.version !== JSON_VERSION) fail("version", `${doc.version} is not a version this game reads (1..${JSON_VERSION})`);
	const p = doc.profile;
	const check = (path: string, v: Json | undefined, def: FieldDef, dims: readonly number[]): void => {
		const type = def[1];
		if (dims.length) {
			if (!Array.isArray(v) || v.length !== dims[0]) {
				fail(path, `expected ${dims[0]} elements`);
				return;
			}
			v.forEach((e, i) => check(`${path}[${i}]`, e, def, dims.slice(1)));
			return;
		}
		if (type === "string") {
			if (typeof v !== "string") fail(path, "expected a string");
		} else if (type === "sector") {
			const s = v as Record<string, Json> | null;
			if (typeof s !== "object" || s === null || Array.isArray(s)) fail(path, "expected an object");
			else {
				inRange(`${path}.x`, s.x, ...RANGE.i16);
				inRange(`${path}.y`, s.y, ...RANGE.i16);
				inRange(`${path}.z`, s.z, ...RANGE.i8);
			}
		} else if (def[0] === "inv") {
			if (typeof v !== "string" || !items.name(v)) fail(path, `there is no item '${String(v)}'`);
		} else inRange(path, v, ...RANGE[type]);
	};
	for (const def of PROFILE_FIELDS) {
		const [name, , dims = []] = def;
		if (NOT_IN_JSON.has(name)) continue;
		if (!(name in p)) fail(`profile.${name}`, "missing");
		else check(`profile.${name}`, p[name], def, dims);
	}
	if (errors.length) return errors;

	for (const [k, [min, max]] of Object.entries(RULES)) inRange(`profile.${k}`, p[k], min, max);
	inRange("profile.bLife", p.bLife, 1, p.bLifeMax as number);
	for (const k of PROFILE_ID_ARRAYS) (p[k] as number[]).forEach((v, i) => inRange(`profile.${k}[${i}]`, v, -1, NUM_PROFILES - 1));
	const length = (k: string, max: number, measure: (s: string) => number) => {
		if (measure(p[k] as string) > max) fail(`profile.${k}`, `longer than ${max} characters`);
	};
	length("zName", NAME_LENGTH - 1, utf16Length);
	length("zNickname", NICKNAME_LENGTH - 1, utf16Length);
	for (const k of ["PANTS", "VEST", "SKIN", "HAIR"]) length(k, PALETTE_LENGTH - 1, utf8Length);
	if (p.zNickname === "") fail("profile.zNickname", "must not be empty");

	const seen = new Set<string>();
	doc.inventory.forEach((o, n) => {
		const at = (k: string) => `inventory[${n}].${k}`;
		if (!SLOTS.includes(o.slot as (typeof SLOTS)[number])) fail(at("slot"), `'${o.slot}' is not a slot`);
		else if (seen.has(o.slot)) fail(at("slot"), `${o.slot} is taken twice`);
		seen.add(o.slot);
		const item = items.name(o.item);
		if (!item) { fail(at("item"), `there is no item '${o.item}'`); return; }
		if (item.index === NOTHING) fail(at("item"), "an empty slot is left out, not written as NOTHING");
		if (item.mapOnly) fail(at("item"), `'${o.item}' cannot be carried`);
		inRange(at("count"), o.count, 1, MAX_OBJECTS_PER_SLOT);
		const each = (k: string, v: Json | undefined, n: number, min: number, max: number) => {
			if (!Array.isArray(v) || v.length !== n) fail(at(k), `expected ${n} elements`);
			else v.forEach((e, i) => inRange(`${at(k)}[${i}]`, e, min, max));
		};
		switch (item.kind) {
			case "ammo": each("shotsLeft", o.shotsLeft, o.count, 0, item.capacity); break;
			case "gun":
				inRange(at("status"), o.status, 1, 100);
				inRange(at("shotsLeft"), o.shotsLeft ?? 0, 0, item.capacity);
				if (o.ammoItem !== undefined) {
					const ammo = items.name(o.ammoItem);
					if (!ammo) fail(at("ammoItem"), `there is no item '${o.ammoItem}'`);
					else if (ammo.index !== NOTHING && ammo.kind !== "ammo") fail(at("ammoItem"), `'${o.ammoItem}' is not ammunition`);
				}
				break;
			case "key":
				if (o.count > KEY_STATUSES) fail(at("count"), `a stack of keys holds at most ${KEY_STATUSES}`);
				else each("status", o.status, o.count, 1, 100);
				break;
			case "money": inRange(at("status"), o.status, 1, 100); break;
			case "plain": each("status", o.status, o.count, 1, 100); break;
		}
		const attachments = o.attachments ?? [];
		if (attachments.length > MAX_ATTACHMENTS) fail(at("attachments"), `an item takes at most ${MAX_ATTACHMENTS} attachments`);
		attachments.forEach((a, i) => {
			const ai = items.name(a.item);
			if (!ai) fail(`${at("attachments")}[${i}].item`, `there is no item '${a.item}'`);
			else if (ai.index === NOTHING) fail(`${at("attachments")}[${i}].item`, "an attachment must not be NOTHING");
			inRange(`${at("attachments")}[${i}].status`, a.status, 1, 100);
		});
	});
	return errors;
}

function sortKeys(v: Json): Json {
	if (Array.isArray(v)) return v.map(sortKeys);
	if (v && typeof v === "object") {
		return Object.fromEntries(Object.keys(v).sort().map((k) => [k, sortKeys(v[k]!)]));
	}
	return v;
}

/** The document as the game writes one: keys sorted, two space indent. */
export function serialize(doc: ProfileDoc): string {
	return JSON.stringify(sortKeys(doc as unknown as Json), null, 2) + "\n";
}
