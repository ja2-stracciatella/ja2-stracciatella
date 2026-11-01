// Generates the converter's UI art through OpenRouter, in the style of the
// reference crops in art/refs/. Raw model output goes to art/raw/, the
// finished assets (downscaled, palette-reduced, keyed) to public/ui/.
//
//   node art/generate.mjs              generate every asset missing from art/raw/
//   node art/generate.mjs marble-grey  (re)generate only the named assets
//
// Reads the API key from openrouter-key.txt, which is git-ignored.

import { execFileSync } from "node:child_process";
import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const MODEL = "google/gemini-3-pro-image";

const STYLE =
	"Match the art style of the reference image exactly: a 1999 PC strategy game " +
	"(Jagged Alliance 2), low-resolution pre-rendered 3D/painted look, muted colours, " +
	"slightly noisy, no modern gloss, no text, no logos, no watermark.";
const KEY_BG =
	"Place the subject alone on a perfectly flat, uniform pure green (#00FF00) background " +
	"with no shadow on the background, no gradient and no floor.";

// post: how the raw image becomes the asset.
//   tile  – seamless texture, reduced to an indexed palette like the game's STI files
//   key   – cut out from the green background, then trimmed
const ASSETS = [
	{
		name: "marble-grey",
		refs: ["marble.png"],
		aspect: "1:1",
		size: "512x512",
		post: "tile",
		prompt:
			"Create a seamless, tileable square texture of the dark grey mottled 'marble / " +
			"camouflage' pattern shown in the reference: blotchy medium-grey and dark-grey " +
			"patches with pale speckled edges. Same scale of blotches and same greys. The " +
			"texture must tile seamlessly on all four edges.",
	},
	{
		name: "icon-open",
		refs: ["sidebar.png"],
		aspect: "1:1",
		size: "64x64",
		post: "key",
		prompt:
			"A single desktop icon in the style of the 'Dateien' icon of the reference sirOS " +
			"sidebar: a yellow file folder with a magnifying glass, late-90s OS icon look. " + KEY_BG,
	},
	{
		name: "icon-save",
		refs: ["sidebar.png"],
		aspect: "1:1",
		size: "64x64",
		post: "key",
		prompt:
			"A single desktop icon in the style of the icons of the reference sirOS sidebar: a " +
			"flat blue 3.5 inch floppy diskette seen straight from the front (metal shutter at the top, white paper label below) with a small red arrow pointing down onto it, late-90s OS icon " +
			"look. " + KEY_BG,
	},
];

function magick(...args) {
	execFileSync("magick", args, { stdio: "inherit" });
}

async function generate(asset, key) {
	const content = [{ type: "text", text: `${asset.prompt} ${STYLE}` }];
	for (const ref of asset.refs) {
		const b64 = readFileSync(join(root, "art/refs", ref)).toString("base64");
		content.push({ type: "image_url", image_url: { url: `data:image/png;base64,${b64}` } });
	}
	const res = await fetch("https://openrouter.ai/api/v1/chat/completions", {
		method: "POST",
		headers: { Authorization: `Bearer ${key}`, "Content-Type": "application/json" },
		body: JSON.stringify({
			model: MODEL,
			modalities: ["image", "text"],
			image_config: { aspect_ratio: asset.aspect },
			messages: [{ role: "user", content }],
		}),
	});
	if (!res.ok) throw new Error(`${asset.name}: HTTP ${res.status} ${await res.text()}`);
	const json = await res.json();
	const url = json.choices?.[0]?.message?.images?.[0]?.image_url?.url;
	if (!url) throw new Error(`${asset.name}: no image in response: ${JSON.stringify(json).slice(0, 500)}`);
	const raw = join(root, "art/raw", `${asset.name}.png`);
	writeFileSync(raw, Buffer.from(url.slice(url.indexOf(",") + 1), "base64"));
	console.log(`${asset.name}: generated (cost ${json.usage?.cost ?? "?"})`);
}

function finish(asset) {
	const raw = join(root, "art/raw", `${asset.name}.png`);
	const out = join(root, "public/ui", `${asset.name}.png`);
	if (asset.post === "tile") {
		// The model rarely tiles cleanly. Blend the image with a copy rolled by half
		// its size: towards the edges the rolled copy takes over, and its edges are
		// the original's middle, which continue into each other.
		const [w, h] = asset.size.split("x").map(Number);
		const tmp = join(root, "art/raw", `${asset.name}.tmp.png`);
		// Flattening the lighting first keeps the blend from showing as a grid.
		magick(raw, "-gravity", "center", "-crop", "84%x84%+0+0", "+repage", "-resize", `${w}x${h}!`,
			"(", "-clone", "0", "-blur", "0x40", ")", "(", "-clone", "0", "-scale", "1x1!", "-scale", `${w}x${h}!`, ")",
			"-fx", "u[0]/(u[1]+0.001)*u[2]", "-delete", "1,2", tmp);
		magick(tmp, "(", "+clone", "-roll", `+${w / 2}+${h / 2}`, ")", "+swap",
			"(", "-size", `${w}x${h}`, "xc:black", "-fx", "min(1, 4*min(min(i,w-1-i)/w, min(j,h-1-j)/h))", ")",
			"-composite", "-dither", "FloydSteinberg", "-colors", "64", out);
	} else {
		// Key out every green pixel, not just those connected to a corner, then pull
		// the green spill out of the edges so the cut-out sits on any background.
		magick(raw, "-alpha", "set", "-fuzz", "30%", "-transparent", "#00FF00",
			"-channel", "A", "-morphology", "Erode", "Disk:1", "+channel", "-trim", "+repage",
			"-resize", asset.size, "-channel", "G", "-fx", "min(g, max(r, b))", "+channel",
			"-dither", "FloydSteinberg", "-colors", "128", out);
	}
	console.log(`${asset.name}: → public/ui/${asset.name}.png`);
}

const key = readFileSync(join(root, "openrouter-key.txt"), "utf8").trim();
const only = process.argv.slice(2);
for (const dir of ["art/raw", "public/ui"]) mkdirSync(join(root, dir), { recursive: true });

const todo = ASSETS.filter((a) => (only.length ? only.includes(a.name) : !existsSync(join(root, "art/raw", `${a.name}.png`))));
const results = await Promise.allSettled(todo.map((a) => generate(a, key)));
results.forEach((r, i) => r.status === "rejected" && console.error(String(r.reason)));
for (const a of ASSETS) if (existsSync(join(root, "art/raw", `${a.name}.png`))) finish(a);
