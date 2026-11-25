// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/led: brightness of the indicator light.
import { $ } from "../core/app.js";
import { send } from "../core/link.js";

export function initLed() {
  $("led").oninput = () => { $("ledv").textContent = $("led").value; };
  $("led").onchange = () => send("led " + $("led").value);
}
