// Decodes STCI images (.sti), the game's own image format. See src/sgp/ImgFmt.h
// and rust/stracciatella/src/file_formats/stci/ for the layout.
//
// Indexed images hold a 256 colour palette and one or more ETRLE compressed
// sub images; palette index 0 is transparent. RGB images hold one RGB565 image.

const HEADER_BYTES = 64;
const PALETTE_BYTES = 256 * 3;
const SUBIMAGE_HEADER_BYTES = 16;

const STCI_ETRLE_COMPRESSED = 0x20;
const STCI_INDEXED = 0x08;
const STCI_RGB = 0x04;

export interface StiImage {
	width: number;
	height: number;
	/** Where the game draws this sub image relative to the object's origin. */
	offsetX: number;
	offsetY: number;
	/** RGBA, 4 bytes per pixel, row by row. */
	rgba: Uint8ClampedArray;
}

function etrleDecompress(src: Uint8Array, out: Uint8Array): void {
	let i = 0;
	let o = 0;
	while (i < src.length && o < out.length) {
		const control = src[i++]!;
		const length = control & 0x7f;
		if (control & 0x80) {
			o += length; // a run of transparent pixels, already 0
		} else {
			out.set(src.subarray(i, i + length), o);
			i += length;
			o += length;
		}
	}
}

export function decodeSti(bytes: Uint8Array): StiImage[] {
	const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
	if (new TextDecoder().decode(bytes.subarray(0, 4)) !== "STCI") throw new Error("STI: missing STCI signature");
	const flags = view.getUint32(16, true);
	const height = view.getUint16(20, true);
	const width = view.getUint16(22, true);

	if (flags & STCI_RGB) {
		const rgba = new Uint8ClampedArray(width * height * 4);
		for (let p = 0; p < width * height; p++) {
			const c = view.getUint16(HEADER_BYTES + p * 2, true);
			const r = c >> 11, g = (c >> 5) & 0x3f, b = c & 0x1f;
			rgba[p * 4] = (r << 3) | (r >> 2);
			rgba[p * 4 + 1] = (g << 2) | (g >> 4);
			rgba[p * 4 + 2] = (b << 3) | (b >> 2);
			rgba[p * 4 + 3] = 255;
		}
		return [{ width, height, offsetX: 0, offsetY: 0, rgba }];
	}

	if (!(flags & STCI_INDEXED)) throw new Error(`STI: unsupported flags 0x${flags.toString(16)}`);
	const count = view.getUint16(28, true);
	const palette = bytes.subarray(HEADER_BYTES, HEADER_BYTES + PALETTE_BYTES);
	const headers = HEADER_BYTES + PALETTE_BYTES;
	const data = headers + count * SUBIMAGE_HEADER_BYTES;
	const images: StiImage[] = [];
	for (let s = 0; s < count; s++) {
		const h = headers + s * SUBIMAGE_HEADER_BYTES;
		const offset = view.getUint32(h, true);
		const length = view.getUint32(h + 4, true);
		const w = view.getUint16(h + 14, true);
		const ht = view.getUint16(h + 12, true);
		const indices = new Uint8Array(w * ht);
		const src = bytes.subarray(data + offset, data + offset + length);
		if (flags & STCI_ETRLE_COMPRESSED) etrleDecompress(src, indices);
		else indices.set(src.subarray(0, indices.length));
		const rgba = new Uint8ClampedArray(w * ht * 4);
		for (let p = 0; p < indices.length; p++) {
			const idx = indices[p]!;
			if (idx === 0) continue;
			rgba[p * 4] = palette[idx * 3]!;
			rgba[p * 4 + 1] = palette[idx * 3 + 1]!;
			rgba[p * 4 + 2] = palette[idx * 3 + 2]!;
			rgba[p * 4 + 3] = 255;
		}
		images.push({ width: w, height: ht, offsetX: view.getInt16(h + 8, true), offsetY: view.getInt16(h + 10, true), rgba });
	}
	return images;
}
