// SPDX-License-Identifier: LGPL-2.1-or-later
// Tiny DOM helpers: no innerHTML anywhere (text only), so nothing a board prints can become markup.
export const $ = (sel, root = document) => root.querySelector(sel);
export function el(tag, props = {}, ...kids) {
  const n = document.createElement(tag);
  for (const [k, v] of Object.entries(props || {})) {
    if (v === false || v === null || v === undefined) continue;
    if (k === "class") n.className = v;
    else if (k === "text") n.textContent = v;
    else if (k.startsWith("on")) n.addEventListener(k.slice(2), v);
    else n.setAttribute(k, v === true ? "" : String(v));
  }
  for (const c of kids.flat()) if (c !== null && c !== undefined && c !== false) n.append(c.nodeType ? c : document.createTextNode(String(c)));
  return n;
}
export const show = (n, on) => { n.hidden = !on; };
