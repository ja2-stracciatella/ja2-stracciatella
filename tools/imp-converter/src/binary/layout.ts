// Describes a C struct as it lay in memory in a build of the game, and reads
// one from bytes. Offsets are not written down but computed as a C
// compiler lays a struct out: each field aligned to its own size, the struct
// padded to its widest member. A layout is then as short as the struct it
// copies, and a test compares its size against what the game wrote.

export type Scalar = "i8" | "u8" | "bool" | "i16" | "u16" | "i32" | "u32";
/** An ST::string as the build held it; see readSavedString. */
export type Str = "string";
/** SGPSector: INT16 x, INT16 y, INT8 z. */
export type Sector = "sector";
export type FieldType = Scalar | Str | Sector;

/** [name, type] or [name, type, dimensions], e.g. ["ubApproachMod", "u8", [3, 4]]. */
export type FieldDef = readonly [string, FieldType] | readonly [string, FieldType, readonly number[]];

/** The build that wrote a file. Only a string differs between the two: it
 * holds a pointer and a size_t, 8 bytes each in a 64-bit build and 4 in a
 * 32-bit one (st_charbuffer.h in string_theory). */
export type Bits = 32 | 64;

const SIZE: Record<Exclude<FieldType, "string">, number> = { i8: 1, u8: 1, bool: 1, i16: 2, u16: 2, i32: 4, u32: 4, sector: 6 };
const ALIGN: Record<Exclude<FieldType, "string">, number> = { i8: 1, u8: 1, bool: 1, i16: 2, u16: 2, i32: 4, u32: 4, sector: 2 };
const size = (t: FieldType, bits: Bits) => (t === "string" ? 2 * (bits / 8) + 16 : SIZE[t]);
const alignment = (t: FieldType, bits: Bits) => (t === "string" ? bits / 8 : ALIGN[t]);

export interface Field {
	name: string;
	type: FieldType;
	dims: readonly number[];
	offset: number;
}

export interface Layout {
	fields: Field[];
	size: number;
	bits: Bits;
}

const align = (n: number, to: number) => Math.ceil(n / to) * to;

export function layout(defs: readonly FieldDef[], bits: Bits = 64): Layout {
	let offset = 0;
	let widest = 1;
	const fields = defs.map(([name, type, dims = []]) => {
		offset = align(offset, alignment(type, bits));
		widest = Math.max(widest, alignment(type, bits));
		const field = { name, type, dims, offset };
		offset += size(type, bits) * dims.reduce((a, b) => a * b, 1);
		return field;
	});
	return { fields, size: align(offset, widest), bits };
}

export interface SavedString {
	text: string;
	/** The text did not fit inside the string and was never written. */
	lost: boolean;
}

/** An ST::string: a pointer into the heap of the game that wrote the file, the
 * length, and a 16 byte buffer the text lives in when it is short enough. A
 * longer text was on the heap, which the file never held. */
function readSavedString(view: DataView, at: number, bits: Bits): SavedString {
	const word = bits / 8;
	const length = bits === 64 ? Number(view.getBigUint64(at + word, true)) : view.getUint32(at + word, true);
	if (length >= 16) return { text: "", lost: true };
	const bytes = new Uint8Array(view.buffer, view.byteOffset + at + 2 * word, length);
	return { text: new TextDecoder("utf-8").decode(bytes), lost: false };
}

function readScalar(view: DataView, type: Scalar, at: number): number {
	switch (type) {
		case "i8": return view.getInt8(at);
		case "u8": case "bool": return view.getUint8(at);
		case "i16": return view.getInt16(at, true);
		case "u16": return view.getUint16(at, true);
		case "i32": return view.getInt32(at, true);
		case "u32": return view.getUint32(at, true);
	}
}

export type Value = number | SavedString | { x: number; y: number; z: number } | Value[];

function readOne(view: DataView, type: FieldType, at: number, bits: Bits): Value {
	if (type === "string") return readSavedString(view, at, bits);
	if (type === "sector") return { x: view.getInt16(at, true), y: view.getInt16(at + 2, true), z: view.getInt8(at + 4) };
	return readScalar(view, type, at);
}

function readArray(view: DataView, type: FieldType, at: number, dims: readonly number[], bits: Bits): Value {
	if (dims.length === 0) return readOne(view, type, at, bits);
	const [n, ...rest] = dims as [number, ...number[]];
	const stride = size(type, bits) * rest.reduce((a, b) => a * b, 1);
	return Array.from({ length: n }, (_, i) => readArray(view, type, at + i * stride, rest, bits));
}

export function read(l: Layout, bytes: Uint8Array, at = 0): Record<string, Value> {
	if (bytes.length < at + l.size) throw new Error(`need ${l.size} bytes at ${at}, have ${bytes.length - at}`);
	const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
	const out: Record<string, Value> = {};
	for (const f of l.fields) out[f.name] = readArray(view, f.type, at + f.offset, f.dims, l.bits);
	return out;
}
