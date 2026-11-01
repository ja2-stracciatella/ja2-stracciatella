// Picks the importer for an old binary profile. Nothing in such a file says
// which layout it is in except its length, and no two layouts, 32 or 64-bit,
// have the same one (a test makes sure).

import type { Importer } from "./types.ts";
import { v021, v021x32 } from "./v021.ts";
import { v022, v022x32 } from "./v022.ts";
import { v104, v104x32 } from "./v104.ts";

export const IMPORTERS: Importer[] = [v021, v022, v104, v021x32, v022x32, v104x32];

/** Layouts known to exist but not read yet, so the page can name them. */
export const KNOWN_SIZES: Record<number, string> = {};

export function importerFor(size: number): Importer | undefined {
	return IMPORTERS.find((i) => i.size === size);
}
