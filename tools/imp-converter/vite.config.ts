import { readFileSync } from "node:fs";
import { join } from "node:path";
import { defineConfig, type Plugin } from "vite";
import { viteSingleFile } from "vite-plugin-singlefile";
import { itemsFromExternalized } from "./src/gamedata/items.ts";
import { readJsonc } from "./src/gamedata/jsonc.ts";

const externalized = join(import.meta.dirname, "../../assets/externalized");

const file = (name: string) => readJsonc(readFileSync(join(externalized, `${name}.json`), "utf8")) as never;

/** A module made at build time from this repo's assets/externalized. */
function generated(id: string, make: () => unknown): Plugin {
	return {
		name: id,
		resolveId: (source) => (source === id ? "\0" + id : undefined),
		load: (resolved) => (resolved === "\0" + id ? `export default ${JSON.stringify(make())};` : undefined),
	};
}

/** `virtual:items`: the game's items, reduced to what the converter needs. */
const items = () => generated("virtual:items", () => itemsFromExternalized({
	items: file("items"), weapons: file("weapons"), magazines: file("magazines"),
	armours: file("armours"), explosives: file("explosives"),
}));

/** `virtual:imp`: the portraits and voices the I.M.P. site offers. */
const imp = () => generated("virtual:imp", () => {
	const { portraits, voices } = file("imp") as {
		portraits: { gender: string; face: number; skin: string; hair: string; eyes: number[]; mouth: number[] }[];
		voices: { gender: string; profile: number }[];
	};
	return {
		portraits: portraits.map((p) => ({ id: p.face, male: p.gender === "MALE", skin: p.skin, hair: p.hair, eyes: p.eyes, mouth: p.mouth })),
		voices: voices.map((v) => ({ id: v.profile, male: v.gender === "MALE" })),
	};
});

export default defineConfig({
	plugins: [items(), imp(), viteSingleFile()],
	// public/ holds sources the page imports (and so embeds), not files to copy.
	publicDir: false,
	build: { outDir: "dist" },
});
