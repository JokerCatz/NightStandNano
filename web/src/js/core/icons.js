// SPDX-License-Identifier: LGPL-2.1-or-later
// The icon set: simple outline icons on a 24 x 24 grid, stroke = currentColor (drawn here, no icon font, no external files).
// Markup carries <svg class="ic" data-ic="name"></svg>; hydrateIcons() fills it in (CSS in base.css sets stroke, width, linecap).
// The transport glyphs are solid (fill="currentColor" on the shape); everything else is stroke only.
const SOLID = ' fill="currentColor"';
export const ICONS = {
  now: '<circle cx="12" cy="12" r="9"/><path d="M10 8.5l5.5 3.5-5.5 3.5z"/>',
  music: '<path d="M9 18V5l11-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="17" cy="16" r="3"/>',
  radio: '<rect x="3" y="8" width="18" height="12" rx="2"/><path d="M7 8l10-5"/><circle cx="8.5" cy="14" r="2.5"/><path d="M15 13h3M15 16h3"/>',
  ambient: '<path d="M3 7c3-3 6 3 9 0s6 3 9 0"/><path d="M3 12c3-3 6 3 9 0s6 3 9 0"/><path d="M3 17c3-3 6 3 9 0s6 3 9 0"/>',
  settings: '<path d="M4 6h8M18 6h2M4 12h2M12 12h8M4 18h10M20 18h0"/><circle cx="15" cy="6" r="2.5"/><circle cx="9" cy="12" r="2.5"/><circle cx="17" cy="18" r="2.5"/>',
  help: '<circle cx="12" cy="12" r="9"/><path d="M9.5 9.2a2.6 2.6 0 0 1 5 .8c0 1.8-2.5 2.1-2.5 3.7"/><path d="M12 17h.01"/>',
  back: '<path d="M15 5l-7 7 7 7"/>',
  chev: '<path d="M9 5l7 7-7 7"/>',
  up: '<path d="M12 19V6M6 11l6-6 6 6"/>',
  play: '<path d="M7 4.5v15l12-7.5z"' + SOLID + '/>',
  pause: '<rect x="6" y="5" width="4" height="14" rx="1"' + SOLID + '/><rect x="14" y="5" width="4" height="14" rx="1"' + SOLID + '/>',
  prev: '<path d="M6 5v14"/><path d="M19 5.5v13L9 12z"' + SOLID + '/>',
  next: '<path d="M18 5v14"/><path d="M5 5.5v13L15 12z"' + SOLID + '/>',
  stop: '<rect x="6" y="6" width="12" height="12" rx="2"' + SOLID + '/>',
  vol: '<path d="M4 9h4l5-4v14l-5-4H4z"/><path d="M16.5 8.5a5 5 0 0 1 0 7M19 6a8.5 8.5 0 0 1 0 12"/>',
  auto: '<path d="M12 3l2.2 5.8L20 11l-5.8 2.2L12 19l-2.2-5.8L4 11l5.8-2.2z"/>',
  dac: '<path d="M9 3v5M15 3v5M7 8h10v4a5 5 0 0 1-10 0zM12 17v4"/>',
  bt: '<path d="M7 7l10 10-5 4V3l5 4L7 17"/>',
  cast: '<circle cx="12" cy="12" r="1.8"/><path d="M8.3 8.3a5.2 5.2 0 0 0 0 7.4M15.7 8.3a5.2 5.2 0 0 1 0 7.4M5.4 5.4a9.3 9.3 0 0 0 0 13.2M18.6 5.4a9.3 9.3 0 0 1 0 13.2"/>',
  lyrics: '<path d="M4 6h16M4 11h12M4 16h16M4 21h8"/>',
  night: '<path d="M20 14.5A8 8 0 1 1 9.5 4a6.5 6.5 0 0 0 10.5 10.5z"/>',
  output: '<path d="M4 15v-3a8 8 0 0 1 16 0v3"/><rect x="3" y="14" width="4" height="7" rx="1.5"/><rect x="17" y="14" width="4" height="7" rx="1.5"/>',
  fx: '<path d="M4 21v-7M4 10V3M12 21v-9M12 8V3M20 21v-5M20 12V3M1.5 14h5M9.5 8h5M17.5 16h5"/>',
  wifi: '<path d="M2 9a15 15 0 0 1 20 0M5.5 12.5a10 10 0 0 1 13 0M9 16a5 5 0 0 1 6 0M12 19.5h.01"/>',
  clock: '<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3.5 2"/>',
  card: '<path d="M7 3h7l4 4v14H7z"/><path d="M10 7v3M13 7v3"/>',
  keys: '<rect x="7" y="2.5" width="10" height="19" rx="3"/><path d="M12 7v.01M9.5 12h5M9.5 16h5"/>',
  led: '<path d="M9 18h6M10 21h4"/><path d="M12 3a6 6 0 0 0-3.5 10.9c.6.5 1 1.2 1 2.1h5c0-.9.4-1.6 1-2.1A6 6 0 0 0 12 3z"/>',
  viz: '<path d="M3 12h2.5l2-6 3.5 12 3-9 2 5 1.5-2H21"/>',
  lang: '<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18"/>',
  flash: '<path d="M13 2L5 13.5h6L10 22l9-12h-6z"/>',
  download: '<path d="M12 3v12M7 10.5l5 5 5-5M4 20h16"/>',
  code: '<path d="M8.5 7L3.5 12l5 5M15.5 7l5 5-5 5M13.5 4.5l-3 15"/>',
  ext: '<path d="M14 4h6v6M20 4l-9 9M18 14v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h5"/>',
  diag: '<path d="M4 18a8 8 0 1 1 16 0"/><path d="M12 18l4-6"/><path d="M12 18h.01"/>',
};
export function setIcon(svg, name) {
  svg.setAttribute("data-ic", name);
  svg.setAttribute("viewBox", "0 0 24 24");
  svg.setAttribute("aria-hidden", "true");
  svg.innerHTML = ICONS[name] || "";
}
export function hydrateIcons(root = document) { for (const s of root.querySelectorAll("svg[data-ic]")) setIcon(s, s.getAttribute("data-ic")); }
