import type { ItemTable } from "../gamedata/items.ts";
import type { ProfileDoc } from "../model/profile.ts";

export interface Import {
	doc: ProfileDoc;
	/** Things the converter had to leave out or change, for the user to see. */
	warnings: string[];
	/** Text fields the old file could not hold (16 characters or more), now
	 * empty; the user has to type them again. */
	lostNames: string[];
}

export interface Importer {
	id: string;
	label: string;
	/** The file size this layout makes; old files carry nothing else that says
	 * which layout they are in. */
	size: number;
	recordSize: number;
	read(bytes: Uint8Array, items: ItemTable): Import;
}
