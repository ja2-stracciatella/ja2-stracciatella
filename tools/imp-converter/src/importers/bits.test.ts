import { readFileSync } from "node:fs";
import { join } from "node:path";
import { describe, expect, it } from "vitest";
import items from "virtual:items";
import { type Bits, type FieldDef, layout } from "../binary/layout.ts";
import { ItemTable } from "../gamedata/items.ts";
import { PROFILE_FIELDS } from "../model/fields.ts";
import { IMPORTERS } from "./registry.ts";
import { v022, v022x32 } from "./v022.ts";
import { v104, v104x32 } from "./v104.ts";

const table = new ItemTable(items);
const fixture = (path: string) => new Uint8Array(readFileSync(join(import.meta.dirname, "../../merc-profiles", path)));

/** The same profile as a build of the other width would have written it: every
 * field moved to where that build laid it out, strings with their pointer and
 * size shrunk or grown, text and objects as they were. No real 32-bit file is
 * at hand, so this checks the layouts against each other, not against a game. */
function rewrite(bytes: Uint8Array, fields: readonly FieldDef[], from: Bits, to: Bits): Uint8Array {
	const a = layout(fields, from), b = layout(fields, to);
	const out = new Uint8Array(b.size + (bytes.length - a.size));
	out.set(bytes.subarray(a.size), b.size);
	const src = new DataView(bytes.buffer, bytes.byteOffset), dst = new DataView(out.buffer);
	a.fields.forEach((f, i) => {
		const g = b.fields[i]!;
		if (f.type !== "string") {
			const width = (a.fields[i + 1]?.offset ?? a.size) - f.offset;
			out.set(bytes.subarray(f.offset, f.offset + Math.min(width, (b.fields[i + 1]?.offset ?? b.size) - g.offset)), g.offset);
			return;
		}
		// No layout has an array of strings.
		const fw = from / 8, tw = to / 8;
		const length = from === 64 ? Number(src.getBigUint64(f.offset + fw, true)) : src.getUint32(f.offset + fw, true);
		if (to === 64) dst.setBigUint64(g.offset + tw, BigInt(length), true); else dst.setUint32(g.offset + tw, length, true);
		out.set(bytes.subarray(f.offset + 2 * fw, f.offset + 2 * fw + 16), g.offset + 2 * tw);
	});
	return out;
}

describe("32-bit builds", () => {
	it("give every layout a file size of its own", () => {
		const sizes = IMPORTERS.map((i) => i.size);
		expect(new Set(sizes).size).toBe(sizes.length);
		expect(Object.fromEntries(IMPORTERS.map((i) => [i.label, i.size]))).toMatchInlineSnapshot(`
			{
			  "0.21 binary": 1332,
			  "0.21 binary, 32-bit": 1280,
			  "0.22 binary": 1348,
			  "0.22 binary, 32-bit": 1296,
			  "save v104 binary": 1356,
			  "save v104 binary, 32-bit": 1300,
			}
		`);
	});

	it.each([
		["0.22", v022, v022x32, "binary/mercprofile.John", PROFILE_FIELDS.filter(([n]) => n !== "ubVoiceId" && n !== "impSlotState")],
		["save v104", v104, v104x32, "binary-v104/mercprofile.Clone", PROFILE_FIELDS],
	] as const)("read %s as a 64-bit build would", (_, wide, narrow, file, fields) => {
		const bytes = fixture(file);
		const small = rewrite(bytes, fields, 64, 32);
		expect(small.length).toBe(narrow.size);
		expect(narrow.read(small, table)).toEqual(wide.read(bytes, table));
	});
});
