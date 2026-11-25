// SPDX-License-Identifier: LGPL-2.1-or-later
// node test/helpers/make-fixtures.mjs <outdir>: dummy release files (not firmware) for the screenshots.
import { mkdirSync, writeFileSync } from "node:fs";
import { releaseSet, mergedImage } from "./images.js";
const out = process.argv[2] || "fixtures";
mkdirSync(out, { recursive: true });
const s = releaseSet({ version: "1.2.3" });
writeFileSync(out + "/NightStandNano-1.2.3-full.bin", mergedImage(s));
writeFileSync(out + "/NightStandNano-1.2.3-bootloader.bin", s.bootloader);
writeFileSync(out + "/NightStandNano-1.2.3-partition-table.bin", s["partition-table"]);
writeFileSync(out + "/NightStandNano-1.2.3-app.bin", s.app);
writeFileSync(out + "/not-firmware.bin", new Uint8Array(5000).fill(7));
console.log("fixtures in " + out);
