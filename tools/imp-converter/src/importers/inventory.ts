// The carried objects after an old profile's record: OBJECTTYPE as it lay in
// memory, 36 bytes each, which is also how a saved game stores one. Which
// fields the shared middle part holds depends on the item, as in ExtractObject
// (LoadSaveObjectType.cc); the output is what ObjectToJson in the game's
// IMPProfileJson.cc writes for the same object.

import { type ItemTable, NOTHING } from "../gamedata/items.ts";
import { type Attachment, type InventoryObject, KEY_STATUSES, MAX_ATTACHMENTS, MAX_OBJECTS_PER_SLOT, SLOTS } from "../model/profile.ts";

export const SAVED_OBJECT_SIZE = 36;

export function readInventory(bytes: Uint8Array, at: number, items: ItemTable, warnings: string[]): InventoryObject[] {
	const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
	const out: InventoryObject[] = [];
	SLOTS.forEach((slot, i) => {
		const o = at + i * SAVED_OBJECT_SIZE;
		const index = view.getUint16(o, true);
		if (index === NOTHING) return;
		const item = items.index(index);
		const count = view.getUint8(o + 2);
		if (!item) {
			warnings.push(`${slot}: item ${index} is unknown to this game, left out`);
			return;
		}
		if (item.mapOnly || count === 0) {
			warnings.push(`${slot}: ${item.name} cannot be carried, left out`);
			return;
		}
		const u8s = (from: number, n: number) => Array.from({ length: n }, (_, k) => view.getUint8(o + from + k));
		const i8s = (from: number, n: number) => Array.from({ length: n }, (_, k) => view.getInt8(o + from + k));
		const obj: InventoryObject = { slot, item: item.name, count, flags: 0, mission: 0, trap: 0, imprintId: 0, weight: 0, used: 0 };
		const name = (n: number, what: string) => {
			const found = items.index(n);
			if (found) return found.name;
			warnings.push(`${slot}: ${what} ${n} is unknown to this game, left empty`);
			return "NOTHING";
		};
		switch (item.kind) {
			case "ammo":
				obj.shotsLeft = u8s(4, Math.min(count, MAX_OBJECTS_PER_SLOT));
				break;
			case "gun":
				obj.status = view.getInt8(o + 4);
				obj.ammoType = view.getUint8(o + 5);
				obj.shotsLeft = view.getUint8(o + 6);
				obj.ammoItem = name(view.getUint16(o + 8, true), "ammunition");
				obj.ammoStatus = view.getInt8(o + 10);
				break;
			case "key":
				obj.status = i8s(4, Math.min(count, KEY_STATUSES));
				obj.keyId = view.getUint8(o + 10);
				break;
			case "money":
				obj.status = view.getInt8(o + 4);
				obj.amount = view.getUint32(o + 8, true);
				break;
			case "plain":
				obj.status = i8s(4, Math.min(count, MAX_OBJECTS_PER_SLOT));
				break;
		}
		const attachments: Attachment[] = [];
		for (let a = 0; a < MAX_ATTACHMENTS; a++) {
			const n = view.getUint16(o + 16 + a * 2, true);
			if (n === NOTHING) continue;
			const found = items.index(n);
			if (!found) {
				warnings.push(`${slot}: attachment ${n} is unknown to this game, left out`);
				continue;
			}
			attachments.push({ item: found.name, status: view.getInt8(o + 24 + a) });
		}
		if (attachments.length) obj.attachments = attachments;
		obj.flags = view.getInt8(o + 28);
		obj.mission = view.getUint8(o + 29);
		obj.trap = view.getInt8(o + 30);
		obj.imprintId = view.getUint8(o + 31);
		obj.weight = view.getUint8(o + 32);
		obj.used = view.getUint8(o + 33);
		out.push(obj);
	});
	return out;
}
