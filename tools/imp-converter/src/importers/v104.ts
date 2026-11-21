// Profiles written by master at save version 104, the last layout copied from
// memory before profiles became JSON (672 bytes). The voice has a field of its
// own, so clones and twins keep a voice apart from their profile slot.

import { PROFILE_FIELDS } from "../model/fields.ts";
import { binaryImporter } from "./binary.ts";

const spec = { id: "104", label: "save v104 binary", fields: PROFILE_FIELDS };
export const v104 = binaryImporter(spec);
export const v104x32 = binaryImporter({ ...spec, bits: 32 });
