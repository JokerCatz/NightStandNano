// SPDX-License-Identifier: LGPL-2.1-or-later
// The status poll: `st` once a second (started by link.js after a connect) into app.state; every page that shows something
// from it subscribes with on("status", fn). The poll pauses while a Wi-Fi scan runs (the box answers nothing else then).
import { app, emit } from "./app.js";
import { send } from "./link.js";

export async function refreshStatus() {
  if (app.scanBusy) return;
  const r = await send("st");
  try { app.state = JSON.parse(r.find(l => l.startsWith("{"))); } catch { return; }
  emit("status", app.state);
}
