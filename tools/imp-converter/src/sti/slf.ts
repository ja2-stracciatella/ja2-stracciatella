// Reads SLF archives ("Sir-tech Library File"), the containers in the game's
// Data folder. See rust/stracciatella/src/file_formats/slf.rs for the layout.

const HEADER_BYTES = 532;
const ENTRY_BYTES = 280;
const STATE_OK = 0;

export interface SlfEntry {
	/** Full path inside Data, lower case, with "/" separators. */
	path: string;
	offset: number;
	length: number;
}

function fixedString(bytes: Uint8Array, start: number, length: number): string {
	let end = start;
	while (end < start + length && bytes[end] !== 0) end++;
	return new TextDecoder("latin1").decode(bytes.subarray(start, end));
}

function normalize(path: string): string {
	return path.replaceAll("\\", "/").toLowerCase();
}

export class SlfArchive {
	readonly entries = new Map<string, SlfEntry>();
	readonly bytes: Uint8Array;

	constructor(bytes: Uint8Array) {
		this.bytes = bytes;
		const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
		if (bytes.length < HEADER_BYTES) throw new Error("SLF: file too short for a header");
		const libraryPath = normalize(fixedString(bytes, 256, 256));
		const count = view.getInt32(512, true);
		const first = bytes.length - count * ENTRY_BYTES;
		if (count < 0 || first < HEADER_BYTES) throw new Error(`SLF: bad entry count ${count}`);
		for (let i = 0; i < count; i++) {
			const at = first + i * ENTRY_BYTES;
			if (bytes[at + 264] !== STATE_OK) continue;
			const path = libraryPath + normalize(fixedString(bytes, at, 256));
			this.entries.set(path, {
				path,
				offset: view.getUint32(at + 256, true),
				length: view.getUint32(at + 260, true),
			});
		}
	}

	/** The file at a path inside Data, matched without regard to case. */
	read(path: string): Uint8Array | undefined {
		const entry = this.entries.get(normalize(path));
		return entry && this.bytes.subarray(entry.offset, entry.offset + entry.length);
	}
}
