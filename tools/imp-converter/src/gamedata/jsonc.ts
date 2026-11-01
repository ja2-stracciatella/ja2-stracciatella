// The game's externalized JSON may carry // comments, which JSON.parse refuses.

/** JSON with // comments, as the game's externalized files are written. */
export function readJsonc(text: string): unknown {
	let result = "";
	for (let i = 0, inString = false; i < text.length; i++) {
		const c = text[i]!;
		if (inString) {
			result += c;
			if (c === "\\") result += text[++i] ?? "";
			else if (c === '"') inString = false;
		} else if (c === "/" && text[i + 1] === "/") {
			while (i < text.length && text[i] !== "\n") i++;
			result += "\n";
		} else {
			if (c === '"') inString = true;
			result += c;
		}
	}
	return JSON.parse(result);
}
