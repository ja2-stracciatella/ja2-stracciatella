// The item table vite.config.ts builds from assets/externalized; the shape is
// ItemInfo in src/gamedata/items.ts.
declare module "virtual:items" {
	const items: {
		index: number;
		name: string;
		kind: "plain" | "gun" | "ammo" | "key" | "money";
		capacity: number;
		mapOnly: boolean;
	}[];
	export default items;
}

// The I.M.P. site's choices from assets/externalized/imp.json.
declare module "virtual:imp" {
	const imp: {
		portraits: { id: number; male: boolean; skin: string; hair: string; eyes: number[]; mouth: number[] }[];
		voices: { id: number; male: boolean }[];
	};
	export default imp;
}
