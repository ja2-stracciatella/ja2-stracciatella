// The items the game knows, from assets/externalized/*.json, reduced to what
// the converter needs: the number an old file stores, the internal name the
// JSON format uses, and which of the object layouts the item is saved in.

/** How the fields an object shares with other objects are to be read, as
 * ExtractObject (LoadSaveObjectType.cc) decides it. */
export type ItemKind = "plain" | "gun" | "ammo" | "key" | "money";

export interface ItemInfo {
	index: number;
	name: string;
	kind: ItemKind;
	/** Rounds a magazine holds, or a gun's magazine size; 0 otherwise. */
	capacity: number;
	/** Placed on the map only; the game refuses one in a profile. */
	mapOnly: boolean;
}

/** Item 0: an empty slot. */
export const NOTHING = 0;

const IC_KEY = 0x10000;
const IC_MONEY = 0x20000000;
const MAP_ONLY = new Set(["SWITCH", "ACTION_ITEM", "OWNERSHIP"]);
// Weapon types WeaponModels.cc builds with IC_GUN.
const GUN_TYPES = new Set(["PISTOL", "M_PISTOL", "SMG", "SN_RIFLE", "RIFLE", "ASRIFLE", "SHOTGUN", "LMG", "LAW", "CANNON", "MONSTSPIT"]);

interface Entry { itemIndex: number; internalName: string }
export interface ExternalizedItems {
	items: (Entry & { usItemClass: number })[];
	weapons: (Entry & { internalType: string; ubMagSize?: number })[];
	magazines: (Entry & { capacity: number })[];
	armours: Entry[];
	explosives: Entry[];
}

export function itemsFromExternalized(x: ExternalizedItems): ItemInfo[] {
	const info = (e: Entry, kind: ItemKind, capacity = 0): ItemInfo =>
		({ index: e.itemIndex, name: e.internalName, kind, capacity, mapOnly: MAP_ONLY.has(e.internalName) });
	const nothing: ItemInfo = { index: NOTHING, name: "NOTHING", kind: "plain", capacity: 0, mapOnly: false };
	return [
		nothing,
		...x.items.map((e) => info(e, e.usItemClass === IC_KEY ? "key" : e.usItemClass === IC_MONEY ? "money" : "plain")),
		...x.weapons.map((e) => GUN_TYPES.has(e.internalType) ? info(e, "gun", e.ubMagSize ?? 0) : info(e, "plain")),
		...x.magazines.map((e) => info(e, "ammo", e.capacity)),
		...x.armours.map((e) => info(e, "plain")),
		...x.explosives.map((e) => info(e, "plain")),
	].sort((a, b) => a.index - b.index);
}

export class ItemTable {
	private readonly byIndex = new Map<number, ItemInfo>();
	private readonly byName = new Map<string, ItemInfo>();

	constructor(items: ItemInfo[]) {
		for (const i of items) {
			this.byIndex.set(i.index, i);
			this.byName.set(i.name, i);
		}
	}

	index(n: number): ItemInfo | undefined { return this.byIndex.get(n); }
	name(s: string): ItemInfo | undefined { return this.byName.get(s); }
}
