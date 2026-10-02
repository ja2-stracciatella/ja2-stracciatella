// Profiles already in the JSON format. They are taken as they are, keys this
// converter does not know included, so that saving one changes nothing but
// what was edited.

import type { ProfileDoc } from "../model/profile.ts";
import type { Import } from "./types.ts";

export function readJson(text: string): Import {
	let doc: unknown;
	try {
		doc = JSON.parse(text);
	} catch (e) {
		throw new Error(`not JSON: ${(e as Error).message}`);
	}
	const d = doc as Partial<ProfileDoc> | null;
	if (!d || typeof d !== "object" || typeof d.profile !== "object" || !Array.isArray(d.inventory)) {
		throw new Error("not an I.M.P. profile: it needs \"version\", \"profile\" and \"inventory\"");
	}
	return { doc: d as ProfileDoc, warnings: [], lostNames: [] };
}
