// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano flasher page: entry point (one classic script, see build.mjs). ?demo swaps the serial port and esptool-js for pretend ones.
import { I18N } from "./core/i18n.js";
import { createConn } from "./core/conn.js";
import { createMockSerial, createMockEsp } from "./demo/mock.js";
import { setEsptool } from "./core/flasher.js";
import { startApp } from "./ui/app.js";

/* global __BUILD__ */
const q = new URLSearchParams(location.search);
const demo = q.has("demo");
I18N.init();

let conn, esp = null;
if (demo) {
  const num = (k) => (q.has(k) ? Number(q.get(k)) : undefined);
  const serial = createMockSerial({
    sd: q.get("sd") || "mounted", spiCard: q.get("spi") || "none", refuse: q.get("refuse") || "", old: q.has("old"), silent: q.has("silent"), noise: q.get("noise") !== "0",
    split: num("split") || 0, bootMs: num("bootms") ?? 900,
  });
  conn = createConn({ serial, sleep: (ms) => new Promise((r) => setTimeout(r, ms / (q.has("fast") ? 10 : 1))) });
  esp = createMockEsp({ speed: num("speed") ?? 8, stopAt: num("stop"), failAt: num("fail"), chip: q.get("chip") || undefined, flashSize: q.has("flashsize") ? q.get("flashsize") : undefined, corrupt: q.has("corrupt"), maxBaud: num("maxbaud"), writeMaxBaud: num("writemaxbaud") });
  setEsptool(esp);
  globalThis.__nsnDemo = { serial, esp, conn };
} else conn = createConn();

startApp({ conn, esp, I18N, stamp: typeof __BUILD__ === "string" ? __BUILD__ : "dev", demo, tab: q.get("tab") });
