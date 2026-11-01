// The converter's page: a list of the profiles dropped on it, and an editor for
// the one selected. Everything stays in the browser; saving downloads the file.

import items from "virtual:items";
import imp from "virtual:imp";
import iconOpen from "../../public/ui/icon-open.png";
import iconSave from "../../public/ui/icon-save.png";
import { ItemTable } from "../gamedata/items.ts";
import { readJson } from "../importers/json.ts";
import { importerFor, KNOWN_SIZES } from "../importers/registry.ts";
import type { Import } from "../importers/types.ts";
import { type Profile, type ProfileDoc, serialize, validate } from "../model/profile.ts";
import { havePortraits, onPortraitsChanged, portrait, usePortraitFile } from "./portraits.ts";

const table = new ItemTable(items);

const SKILLS = ["No skill", "Lock picking", "Hand to hand", "Electronics", "Night ops", "Throwing", "Teaching", "Heavy weapons", "Auto weapons", "Stealthy", "Ambidextrous", "Thief", "Martial arts", "Knifing", "On roof", "Camouflaged"];
const PERSONALITY = ["No flaws", "Heat intolerant", "Nervous", "Claustrophobic", "Non-swimmer", "Fear of insects", "Forgetful", "Psycho"];
const ATTITUDE = ["Normal", "Friendly", "Loner", "Optimist", "Pessimist", "Aggressive", "Arrogant", "Big shot", "Asshole", "Coward"];
const SEXIST = ["None", "Somewhat sexist", "Very sexist", "Gentleman"];
const EVOLUTION = ["Normally", "Doesn't improve", "Gets worse"];
const SEX = ["Male", "Female"];
// Body types (ubBodyType) the I.M.P. site gives (IMP_Compile_Character.cc).
// The game also has a stocky male build, for soldiers it generates; it has no
// paper doll of its own, and no player could make one, so it is not offered.
const REGULAR_BODY = 0, BIG_BODY = 1, FEMALE_BODY = 3;
// A build belongs to one gender: the female build to women, the other two to men.
const bodies = (p: Profile): [number, string][] =>
	p.bSex === 1 ? [[FEMALE_BODY, "Female build"]] : [[REGULAR_BODY, "Regular build"], [BIG_BODY, "Big build"]];
// Only the regular build has the martial arts moves; the I.M.P. site gives the
// others hand to hand instead.
const MARTIAL_ARTS = 12, HAND_TO_HAND = 2;
const canDoMartialArts = (p: Profile) => p.ubBodyType !== BIG_BODY && p.ubBodyType !== FEMALE_BODY;
const NAME_MAX = 29, NICKNAME_MAX = 9; // NAME_LENGTH - 1, NICKNAME_LENGTH - 1
const NAME_LABEL: Record<string, string> = { zName: "full name", zNickname: "nickname" };

// Limits the game's JSON reader enforces, or the field's C type.
const LIMITS: Record<string, [number, number]> = {
	bLifeMax: [1, 100], bAgility: [1, 100], bDexterity: [1, 100], bStrength: [1, 100], bLeadership: [1, 100], bWisdom: [1, 100],
	bMarksmanship: [0, 100], bMechanical: [0, 100], bExplosive: [0, 100], bMedical: [0, 100], bExpLevel: [1, 10],
	usKills: [0, 65535], usAssists: [0, 65535], usBattlesFought: [0, 65535], usTotalDaysServed: [0, 65535],
	uiTotalCostToDate: [0, 4294967295], sSalary: [0, 32767], sMedicalDepositAmount: [0, 65535], iMercMercContractLength: [0, 2147483647],
};

interface Entry {
	file: string;
	source: string;
	doc?: ProfileDoc;
	warnings: string[];
	/** Names the old file lost; the user has to type them again. */
	lostNames: string[];
	edited: boolean;
	/** Why nothing could be read, or that the format is not supported yet. */
	problem?: string;
	unsupported?: boolean;
}

const entries: Entry[] = [];
let selected = -1;

const $ = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
const esc = (s: unknown) => String(s).replace(/[&<>"]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" })[c]!);
const nickFromFile = (name: string) => name.replace(/^mercprofile\./i, "").replace(/\.json$/i, "");
const nick = (e: Entry) => (e.doc?.profile.zNickname as string | undefined) || nickFromFile(e.file);

function missingNames(e: Entry): string[] {
	return e.lostNames.filter((k) => !e.doc?.profile[k]);
}
function errors(e: Entry): string[] {
	if (!e.doc) return [];
	return [
		...missingNames(e).map((k) => `Type the ${NAME_LABEL[k] ?? k} again: the old file could not hold it.`),
		...validate(e.doc, table).filter((m) => !missingNames(e).some((k) => m.startsWith(`profile.${k}:`))),
	];
}
function state(e: Entry): "ok" | "wait" | "bad" {
	if (!e.doc) return e.unsupported ? "wait" : "bad";
	if (errors(e).length) return missingNames(e).length && errors(e).length === missingNames(e).length ? "wait" : "bad";
	return "ok";
}
const LAMP = { ok: "Ready to save", wait: "Needs attention", bad: "Can't be saved as it is" };

function face(p: Profile | undefined, big: boolean): string {
	const src = p && portrait(p.ubFaceIndex as number, big);
	if (!src) {
		if (!big || !p || havePortraits()) return `<span class="none">?</span>`;
		return `<button class="facesHint" type="button">Show portraits<small>Pick <b>Data/FACES.SLF</b> in your JA2 folder. It stays on this computer.</small></button>`;
	}
	return `<img src="${src}" alt="${big ? `Portrait of ${esc(p.zNickname)}` : ""}">`;
}

function renderList(): void {
	$("list").innerHTML = entries.length ? entries.map((e, i) => {
		const s = state(e);
		return `<div class="row" role="option" tabindex="0" aria-selected="${i === selected}" data-i="${i}">
			<div class="thumb">${face(e.doc?.profile, false)}</div>
			<div>
				<div class="nick">${esc(nick(e))}</div>
				<div class="meta">${esc(e.source)}${e.doc ? ` · level ${e.doc.profile.bExpLevel}` : ""}${e.edited ? " · edited" : ""}</div>
			</div>
			<span class="lamp ${s}" title="${LAMP[s]}"></span>
		</div>`;
	}).join("") : `<div class="empty">No profiles yet.<br>Your old files are in the SavedGames folder.</div>`;
	const ready = entries.filter((e) => state(e) === "ok").length;
	$("count").textContent = `${entries.length} profile${entries.length === 1 ? "" : "s"} · ${ready} ready`;
	$<HTMLButtonElement>("saveAll").disabled = ready === 0;
}

// ---- editor fields ----

const limits = (p: Profile, k: string): [number, number] => (k === "bLife" ? [1, p.bLifeMax as number] : LIMITS[k]!);
const fill = (v: number, max: number) => `calc(${Math.max(0, Math.min(1, v / max)) * 100}% - 2px)`;
const num = (p: Profile, k: string, cls = "") => {
	const [min, max] = limits(p, k);
	return `<input class="num ${cls}" type="text" inputmode="numeric" pattern="[0-9]*" autocomplete="off" data-k="${k}" data-min="${min}" data-max="${max}" value="${p[k]}" aria-label="${k}">`;
};
const pick = (p: Profile, k: string, options: string[], label: string, first = 0) => `<select class="pick" data-k="${k}" aria-label="${label}">${
	options.map((o, i) => `<option value="${i + first}"${i + first === p[k] ? " selected" : ""}>${o}</option>`).join("")}</select>`;
const LEVELS = Array.from({ length: 10 }, (_, i) => `Level ${i + 1}`);
/** A choice among the given values; a value from outside them stays as an extra entry. */
const among = (p: Profile, k: string, options: [number, string][], label: string) => `<select class="pick" data-k="${k}" aria-label="${label}">${
	options.some(([v]) => v === p[k]) ? "" : `<option value="${p[k]}" selected>Other · ${p[k]}</option>`}${
	options.map(([v, o]) => `<option value="${v}"${v === p[k] ? " selected" : ""}>${o}</option>`).join("")}</select>`;
const skills = (p: Profile) => SKILLS.map((name, i): [number, string] => [i, name]).filter(([i]) => i !== MARTIAL_ARTS || canDoMartialArts(p));
/** A choice among the I.M.P. site's options, grouped by gender and numbered as
 * the site shows them. A value from outside the list stays as an extra entry. */
function choice(p: Profile, k: string, list: { id: number; male: boolean }[], label: string): string {
	const group = (male: boolean, name: string) => `<optgroup label="${name}">${list.filter((o) => o.male === male).map((o, i) =>
		`<option value="${o.id}"${o.id === p[k] ? " selected" : ""}>${name} ${i + 1} · ${o.id}</option>`).join("")}</optgroup>`;
	const other = list.some((o) => o.id === p[k]) ? "" : `<option value="${p[k]}" selected>Other · ${p[k]}</option>`;
	return `<select class="pick" data-k="${k}" aria-label="${label}">${other}${group(true, "Male")}${group(false, "Female")}</select>`;
}
const meter = (k: string, v: number, max: number) => `<span class="meter"><i data-m="${k}" data-max="${max}" style="width:${fill(v, max)}"></i></span>`;
const stat = (p: Profile, k: string, label: string, max = 100) =>
	`<div class="stat"><span class="k">${label}</span>${num(p, k)}${meter(k, p[k] as number, max)}</div>`;

function issues(e: Entry): string {
	const errs = errors(e);
	return (errs.length ? `<ul class="issues bad">${errs.map((m) => `<li>${esc(m)}</li>`).join("")}</ul>` : "") +
		(e.warnings.length ? `<ul class="issues">${e.warnings.map((m) => `<li>${esc(m)}</li>`).join("")}</ul>` : "");
}

function renderDetail(): void {
	const e = entries[selected];
	if (!e) { $("detail").innerHTML = `<h2>Merc</h2><div class="empty">Select a profile on the left.</div>`; return; }
	if (!e.doc) {
		$("detail").innerHTML = `<h2>Merc</h2><div class="dossier">
			<div class="portrait"><div class="pic">${face(undefined, true)}</div><div class="plate">${esc(nick(e))}</div></div>
			<div class="who"><p class="name">${esc(nick(e))}</p><p class="sub">${esc(e.file)} · ${esc(e.source)}</p>
			<p class="note">${esc(e.problem)}</p></div></div>`;
		return;
	}
	const p = e.doc.profile;
	const row = (k: string, v: string) => `<dt>${k}</dt><dd>${v}</dd>`;
	const cash = (k: string) => `<span class="cash">$${num(p, k, "wide")}</span>`;
	const lost = (k: string) => (missingNames(e).includes(k) ? ` missing" placeholder="Type it again` : "");
	$("detail").innerHTML = `<h2>Merc${e.edited ? ` <span class="edited">· edited</span>` : ""}</h2>
		<div class="dossier">
			<div class="portrait"><div class="pic">${face(p, true)}</div>
				<div class="plate" data-show="zNickname">${esc(p.zNickname)}</div>
				<dl class="looks">
					<dt>Portrait</dt><dd>${choice(p, "ubFaceIndex", imp.portraits, "Portrait")}</dd>
					<dt>Voice</dt><dd>${choice(p, "ubVoiceId", imp.voices, "Voice")}</dd>
				</dl></div>
			<div>
				<div class="who">
					<p class="name" data-show="zName">${esc(p.zName)}</p>
					<p class="sub">${pick(p, "bExpLevel", LEVELS, "Level", 1)}<span aria-hidden="true">·</span>${pick(p, "bSex", SEX, "Gender")}<span aria-hidden="true">·</span>${among(p, "ubBodyType", bodies(p), "Body type")}<span aria-hidden="true">·</span>${esc(e.source)}</p>
					<p class="skills">${among(p, "bSkillTrait", skills(p), "First skill")}<span aria-hidden="true">·</span>${among(p, "bSkillTrait2", skills(p), "Second skill")}</p>
				</div>
				<div class="names">
					<label>Full name <small>up to ${NAME_MAX} characters</small><input class="txt${lost("zName")}" data-k="zName" maxlength="${NAME_MAX}" value="${esc(p.zName)}" spellcheck="false" autocomplete="off"></label>
					<label>Nickname <small>up to ${NICKNAME_MAX}, used in game</small><input class="txt${lost("zNickname")}" data-k="zNickname" maxlength="${NICKNAME_MAX}" value="${esc(p.zNickname)}" spellcheck="false" autocomplete="off"></label>
				</div>
				<div class="facts">
					<section>
						<h3>Psychological profile</h3>
						<dl>
							${row("Personality", pick(p, "bPersonalityTrait", PERSONALITY, "Personality"))}
							${row("Attitude", pick(p, "bAttitude", ATTITUDE, "Attitude"))}
							${row("Sexism", pick(p, "bSexist", SEXIST, "Sexism"))}
							${row("Learns", pick(p, "bEvolution", EVOLUTION, "Learns"))}
						</dl>
					</section>
					<section>
						<h3>Employment</h3>
						<dl>
							${row("Days served", num(p, "usTotalDaysServed", "wide"))}
							${row("Total cost", cash("uiTotalCostToDate"))}
							${row("Daily salary", cash("sSalary"))}
							${row("Medical deposit", cash("sMedicalDepositAmount"))}
							${row("Contract (days)", num(p, "iMercMercContractLength", "wide"))}
						</dl>
					</section>
				</div>
			</div>
			<div class="stats inset">
				<div class="stat health"><span class="k">Health</span><span class="pair">${num(p, "bLife")}/${num(p, "bLifeMax")}</span>${meter("bLife", p.bLife as number, 100)}</div>
				${stat(p, "bAgility", "Agility")}${stat(p, "bMarksmanship", "Marksmanship")}
				${stat(p, "bDexterity", "Dexterity")}${stat(p, "bMechanical", "Mechanical")}
				${stat(p, "bStrength", "Strength")}${stat(p, "bExplosive", "Explosives")}
				${stat(p, "bLeadership", "Leadership")}${stat(p, "bMedical", "Medical")}
				${stat(p, "bWisdom", "Wisdom")}${stat(p, "bExpLevel", "Level", 10)}
			</div>
			<div class="record"><span>${num(p, "usKills", "wide")} kills</span><span>${num(p, "usAssists", "wide")} assists</span><span>${num(p, "usBattlesFought", "wide")} battles</span></div>
			<div class="actions">
				<button class="btn" type="button" id="save"><img class="iconSave" alt="">Save <span data-show="file">mercprofile.${esc(p.zNickname)}.json</span></button>
			</div>
			<div id="issues" style="display:contents">${issues(e)}</div>
		</div>`;
	setIcons();
	$("save").onclick = () => download(e);
	updateSave(e);
}

function updateSave(e: Entry): void {
	const save = document.getElementById("save") as HTMLButtonElement | null;
	if (save) save.disabled = state(e) !== "ok";
}

/** Picking a portrait takes over what the I.M.P. site stores with it: the
 * talking head's offsets and the body's palettes. */
function setPortrait(p: Profile, id: number): void {
	const o = imp.portraits.find((x) => x.id === id);
	if (o) {
		[p.usEyesX, p.usEyesY] = o.eyes as [number, number];
		[p.usMouthX, p.usMouthY] = o.mouth as [number, number];
		p.SKIN = o.skin;
		p.HAIR = o.hair;
	}
	$("detail").querySelector(".portrait .pic")!.innerHTML = face(p, true);
	const thumb = $("list").querySelector(`[data-i="${selected}"] .thumb`);
	if (thumb) thumb.innerHTML = face(p, false);
}

// Edits go straight into the profile. Numbers stay within the game's limits
// while typing; the minimum applies when the field is left, so "50" can start
// with a 5. A name left empty gets its old value back.
function edit(ev: Event, commit: boolean): void {
	const e = entries[selected];
	const f = (ev.target as HTMLElement).closest<HTMLInputElement | HTMLSelectElement>("[data-k]");
	if (!f || !e?.doc) return;
	const p = e.doc.profile;
	const k = f.dataset.k!;
	if (f instanceof HTMLSelectElement) {
		p[k] = +f.value;
		if (k === "ubFaceIndex") setPortrait(p, p[k] as number);
		// A body belongs to one gender; keep the two in step.
		if (k === "bSex" || k === "ubBodyType") {
			// Changing the gender brings the build along; a build is only offered
			// to its own gender.
			if (k === "bSex") {
				if (p.bSex === 1) p.ubBodyType = FEMALE_BODY;
				else if (p.ubBodyType === FEMALE_BODY) p.ubBodyType = REGULAR_BODY;
			}
			if (!canDoMartialArts(p)) {
				if (p.bSkillTrait === MARTIAL_ARTS) p.bSkillTrait = HAND_TO_HAND;
				if (p.bSkillTrait2 === MARTIAL_ARTS) p.bSkillTrait2 = HAND_TO_HAND;
			}
			// Which skills are offered depends on the build: draw the editor anew.
			e.edited = true;
			renderList();
			renderDetail();
			return;
		}
	} else if (f.classList.contains("num")) {
		const digits = f.value.replace(/\D/g, "");
		if (digits !== f.value) f.value = digits;
		if (digits === "" && !commit) return;
		const [min, max] = limits(p, k);
		let v = Math.min(max, parseInt(digits || "0", 10));
		if (commit) v = Math.max(min, v);
		if (String(v) !== f.value) f.value = String(v);
		p[k] = v;
		if (k === "bLifeMax" && commit && (p.bLife as number) > v) {
			p.bLife = v;
			$("detail").querySelector<HTMLInputElement>('[data-k="bLife"]')!.value = String(v);
		}
	} else {
		if (commit) f.value = f.value.trim();
		const lost = e.lostNames.includes(k);
		if (!f.value.trim() && !lost) { if (!commit) return; f.value = p[k] as string; }
		p[k] = f.value.slice(0, k === "zName" ? NAME_MAX : NICKNAME_MAX);
		if (lost) f.classList.toggle("missing", !p[k]);
	}
	e.edited = true;
	const d = $("detail");
	// Other fields showing a value this edit changed, e.g. the level in the stats.
	d.querySelectorAll<HTMLInputElement | HTMLSelectElement>("[data-k]").forEach((o) => {
		if (o !== f && o.dataset.k! in p && o.value !== String(p[o.dataset.k!])) o.value = String(p[o.dataset.k!]);
	});
	d.querySelectorAll<HTMLElement>("[data-m]").forEach((i) => { i.style.width = fill(p[i.dataset.m!] as number, +i.dataset.max!); });
	d.querySelectorAll<HTMLElement>("[data-show]").forEach((s) => {
		s.textContent = s.dataset.show === "file" ? `mercprofile.${p.zNickname}.json` : String(p[s.dataset.show!]);
	});
	const h2 = d.querySelector("h2")!;
	if (!h2.querySelector(".edited")) h2.insertAdjacentHTML("beforeend", ` <span class="edited">· edited</span>`);
	if (commit) {
		$("issues").innerHTML = issues(e);
		updateSave(e);
		renderList();
	}
}

function download(e: Entry): void {
	if (!e.doc || state(e) !== "ok") return;
	const a = document.createElement("a");
	a.href = URL.createObjectURL(new Blob([serialize(e.doc)], { type: "application/json" }));
	a.download = `mercprofile.${e.doc.profile.zNickname}.json`;
	a.click();
	setTimeout(() => URL.revokeObjectURL(a.href), 0);
}

function select(i: number): void {
	selected = i;
	renderList();
	renderDetail();
}

async function read(f: File): Promise<Entry> {
	const bytes = new Uint8Array(await f.arrayBuffer());
	const base = { file: f.name, warnings: [], lostNames: [], edited: false };
	const from = (source: string, r: Import): Entry => ({ ...base, source, doc: r.doc, warnings: r.warnings, lostNames: r.lostNames });
	if (f.name.toLowerCase().endsWith(".json")) {
		try {
			return from("JSON", readJson(new TextDecoder().decode(bytes)));
		} catch (err) {
			return { ...base, source: "JSON", problem: `Can't read this file: ${(err as Error).message}.` };
		}
	}
	const importer = importerFor(bytes.length);
	if (importer) {
		try {
			return from(importer.label, importer.read(bytes, table));
		} catch (err) {
			return { ...base, source: importer.label, problem: `Can't read this file: ${(err as Error).message}.` };
		}
	}
	const known = KNOWN_SIZES[bytes.length];
	if (known) return { ...base, source: known, unsupported: true, problem: `${known} files aren't supported yet.` };
	return { ...base, source: `${bytes.length} bytes`, problem: `No I.M.P. profile format is ${bytes.length} bytes long.` };
}

async function usePortraits(f: File): Promise<void> {
	try {
		usePortraitFile(new Uint8Array(await f.arrayBuffer()), imp.portraits.map((o) => o.id));
	} catch (err) {
		showError(`${f.name}: ${(err as Error).message}`);
	}
}

async function addFiles(files: FileList | File[]): Promise<void> {
	const all = Array.from(files);
	for (const f of all.filter((x) => /\.slf$/i.test(x.name))) await usePortraits(f);
	const profiles = all.filter((x) => !/\.slf$/i.test(x.name));
	if (!profiles.length) return;
	for (const f of profiles) {
		let e: Entry;
		try {
			e = await read(f);
		} catch (err) {
			e = { file: f.name, source: "?", warnings: [], lostNames: [], edited: false, problem: `Can't read this file: ${(err as Error).message}.` };
		}
		const at = entries.findIndex((x) => nick(x) === nick(e));
		if (at >= 0) entries[at] = e; else entries.push(e);
		selected = at >= 0 ? at : entries.length - 1;
	}
	renderList();
	renderDetail();
}

function setIcons(): void {
	document.querySelectorAll<HTMLImageElement>(".iconSave").forEach((i) => { i.src = iconSave; });
}

// ---- wiring ----

$<HTMLImageElement>("iconOpen").src = iconOpen;
setIcons();
$("detail").addEventListener("input", (ev) => edit(ev, false));
$("detail").addEventListener("change", (ev) => edit(ev, true));
$("list").addEventListener("click", (ev) => {
	const r = (ev.target as HTMLElement).closest<HTMLElement>(".row");
	if (r) select(+r.dataset.i!);
});
$("list").addEventListener("keydown", (ev) => {
	if (ev.key !== "ArrowDown" && ev.key !== "ArrowUp") return;
	ev.preventDefault();
	select(Math.max(0, Math.min(entries.length - 1, selected + (ev.key === "ArrowDown" ? 1 : -1))));
	$("list").querySelector<HTMLElement>(`[data-i="${selected}"]`)?.focus();
});
const fileInput = $<HTMLInputElement>("file");
fileInput.addEventListener("change", () => { if (fileInput.files) void addFiles(fileInput.files); fileInput.value = ""; });
const drop = $("drop");
drop.addEventListener("keydown", (ev) => { if (ev.key === "Enter" || ev.key === " ") { ev.preventDefault(); fileInput.click(); } });
for (const t of ["dragenter", "dragover"]) drop.addEventListener(t, (ev) => { ev.preventDefault(); drop.classList.add("over"); });
for (const t of ["dragleave", "drop"]) drop.addEventListener(t, () => drop.classList.remove("over"));
drop.addEventListener("drop", (ev) => { ev.preventDefault(); if (ev.dataTransfer) void addFiles(ev.dataTransfer.files); });
$("saveAll").addEventListener("click", () => entries.filter((e) => state(e) === "ok").forEach(download));
const facesInput = $<HTMLInputElement>("faces");
facesInput.addEventListener("change", () => { const f = facesInput.files?.[0]; if (f) void usePortraits(f); facesInput.value = ""; });
$("detail").addEventListener("click", (ev) => { if ((ev.target as HTMLElement).closest(".facesHint")) facesInput.click(); });
function renderPortraitState(): void {
	$("facesState").textContent = havePortraits() ? "Portraits ✓" : "Portraits…";
}
onPortraitsChanged(() => { renderPortraitState(); renderList(); renderDetail(); });
renderPortraitState();

// An error nothing else handles would otherwise only reach the console: show it.
function showError(what: unknown): void {
	const message = what instanceof Error ? what.message : String(what);
	$("detail").insertAdjacentHTML("afterbegin", `<ul class="issues bad"><li>Something went wrong: ${esc(message)}</li></ul>`);
}
window.addEventListener("error", (ev) => showError(ev.error ?? ev.message));
window.addEventListener("unhandledrejection", (ev) => showError(ev.reason));

// Standalone when a file named "standalone" is served next to the page. Opened
// from disk, that request fails, and the page stays in game mode.
$("mode").textContent = "Game mode";
fetch("standalone", { method: "HEAD", cache: "no-store" })
	.then((r) => { if (r.ok) $("mode").textContent = "Standalone mode"; })
	.catch(() => {});

renderList();
renderDetail();
