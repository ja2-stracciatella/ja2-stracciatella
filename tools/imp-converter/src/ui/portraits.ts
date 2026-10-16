// Portraits, read from the player's own Data/FACES.SLF. The file is read in the
// browser and never leaves it; the page ships no game art. Portraits decoded
// once are kept in this browser's storage, so the file is picked only once.

import { SlfArchive } from "../sti/slf.ts";
import { decodeSti, type StiImage } from "../sti/sti.ts";

const STORAGE_KEY = "imp-converter.portraits.v1";

let archive: SlfArchive | undefined;
let cache: Record<string, string> = load();
const listeners: (() => void)[] = [];

const key = (id: number, big: boolean) => `${big ? "big" : "small"}/${id}`;
const path = (id: number, big: boolean) => `faces/${big ? "bigfaces/" : ""}${id}.sti`;

function load(): Record<string, string> {
	try {
		return JSON.parse(localStorage.getItem(STORAGE_KEY) ?? "{}") as Record<string, string>;
	} catch {
		return {};
	}
}

function store(): void {
	try {
		localStorage.setItem(STORAGE_KEY, JSON.stringify(cache));
	} catch {
		// Storage full or blocked: the portraits then last until the page closes.
	}
}

function toDataUrl(image: StiImage): string {
	const canvas = document.createElement("canvas");
	canvas.width = image.width;
	canvas.height = image.height;
	canvas.getContext("2d")!.putImageData(new ImageData(new Uint8ClampedArray(image.rgba), image.width, image.height), 0, 0);
	return canvas.toDataURL("image/png");
}

/** The portrait as an image URL, or nothing when FACES.SLF was not given yet
 * or has no such face. */
export function portrait(id: number, big: boolean): string | undefined {
	const k = key(id, big);
	if (cache[k]) return cache[k];
	const bytes = archive?.read(path(id, big));
	if (!bytes) return undefined;
	try {
		cache[k] = toDataUrl(decodeSti(bytes)[0]!);
	} catch {
		return undefined;
	}
	store();
	return cache[k];
}

/** Whether any portrait can be shown: the file was given now or before. */
export function havePortraits(): boolean {
	return archive !== undefined || Object.keys(cache).length > 0;
}

/** Reads a FACES.SLF the player picked. Throws if it is some other file. */
export function usePortraitFile(bytes: Uint8Array, ids: number[]): void {
	const a = new SlfArchive(bytes);
	if (![...a.entries.keys()].some((p) => p.startsWith("faces/bigfaces/"))) {
		throw new Error("this archive holds no portraits; pick Data/FACES.SLF from your JA2 folder");
	}
	archive = a;
	cache = {};
	// The I.M.P. portraits are decoded at once, so they are kept for next time.
	for (const id of ids) {
		portrait(id, true);
		portrait(id, false);
	}
	listeners.forEach((l) => l());
}

export function onPortraitsChanged(l: () => void): void {
	listeners.push(l);
}
