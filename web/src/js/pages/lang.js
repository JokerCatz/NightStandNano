// SPDX-License-Identifier: LGPL-2.1-or-later
// #/settings/lang: the language select (a change re-renders in place: no reload, the connection and every list stay).
// The explanations switch of the same page is core/help.js.
import { $, emit } from "../core/app.js";
import { I18N } from "../core/i18n.js";

export function initLang() {
  const sel = $("langSel");
  sel.value = I18N.pref();
  sel.onchange = () => I18N.setLang(sel.value);
  I18N.onChange(() => { sel.value = I18N.pref(); emit("lang"); });
}
