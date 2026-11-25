// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the visualizer's controls, modelled on tunetap js/ui-viz.js (mode ◀ ▶, preset, the active plugin's
// schema controls, palette, the two "tour" buttons) without its zones / full screen / status line.
//   mountViz({ viz, stage, modeRow, options })
//     viz: createNanoViz() result; stage: the box the canvases go into; modeRow: container for ◀ name ▶ (on the card);
//     options: container for the detailed controls (the Visual tab).
//   Text comes from the page's string table (globalThis.NSN_I18N, see i18n.js); a language change rebuilds the controls.
import { I18N } from '../core/i18n.js';
const el = (tag, props = {}, ...kids) => {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(props)) { if (k === 'on') for (const [ev, fn] of Object.entries(v)) e.addEventListener(ev, fn); else if (k in e && k !== 'list') e[k] = v; else e.setAttribute(k, v); }
  e.append(...kids);
  return e;
};
const setText = (e, v) => { if (e.textContent !== v) e.textContent = v; };

export function mountViz({ viz, stage, modeRow, options }) {
  const { host, t } = viz;

  // ---- on the card: ◀ name ▶ ----
  const prev = el('button', { type: 'button', className: 'viz-btn', 'aria-label': t('viz.prev'), 'data-i18n-aria': 'viz.prev', on: { click: () => host.prevMode() } }, '◀');
  const next = el('button', { type: 'button', className: 'viz-btn', 'aria-label': t('viz.next'), 'data-i18n-aria': 'viz.next', on: { click: () => host.nextMode() } }, '▶');
  const name = el('span', { className: 'viz-mode-name', role: 'status' });
  modeRow.replaceChildren(prev, name, next);

  // ---- the Visual tab ----
  let builtFor = null;           // plugin id the controls were built for
  let controls = [];             // { wrap, el, schema, out }
  let modeSel = null, presetSel = null, themeSel = null;
  const note = el('div', { className: 'mut viz-note', role: 'status' });

  const option = (value, label, selected, disabled = false) => { const o = el('option', { value: String(value), disabled }, label); o.selected = !!selected; return o; };
  const field = (id, label, input, ...extra) => { input.id = id; return el('div', { className: 'viz-field' }, el('label', { htmlFor: id }, label), input, ...extra); };
  const modeLabel = () => { const m = host.modes().find((x) => x.id === host.mode()); return m ? t(m.labelKey) : ''; };

  function build() {
    const p = host.plugin();
    controls = []; presetSel = null;
    const parts = [];
    modeSel = el('select', { className: 'viz-select' }, ...host.modes().map((m) => option(m.id, t(m.labelKey), m.id === host.mode())));
    modeSel.addEventListener('change', () => host.setMode(modeSel.value));
    parts.push(field('vz-mode', t('viz.mode'), modeSel));
    if (!p) {
      parts.push(el('div', { className: 'mut' }, host.status() === 'unavailable' ? t('viz.unavailable') : t('common.loading')));
    } else {
      if (p.presets.length) {
        presetSel = el('select', { className: 'viz-select' }, ...p.presets.map((x) => option(x.id, t(x.labelKey), x.id === p.preset)), option(-1, t('viz.preset.custom'), p.preset === -1, true));
        presetSel.addEventListener('change', () => host.setPreset(Number(presetSel.value)));
        parts.push(field('vz-preset', t('viz.preset'), presetSel));
      }
      for (const s of p.schema) {
        const id = `vz-${s.key}`;
        let input, out = null;
        if (s.type === 'select') {
          input = el('select', { className: 'viz-select' }, ...(s.options ?? []).map((o) => option(o.value, t(o.labelKey), false)));
          input.addEventListener('change', () => host.setCfg({ [s.key]: Number(input.value) }));
        } else if (s.type === 'range') {
          input = el('input', { type: 'range', className: 'viz-range', min: s.min, max: s.max, step: s.step ?? 1 });
          out = el('output', { className: 'viz-range-out', htmlFor: id });
          input.addEventListener('input', () => host.setCfg({ [s.key]: Number(input.value) }));
        } else if (s.type === 'toggle') {
          input = el('input', { type: 'checkbox', className: 'viz-toggle' });
          input.addEventListener('change', () => host.setCfg({ [s.key]: input.checked ? 1 : 0 }));
        } else continue;
        const w = field(id, t(s.labelKey), input, ...(out ? [out] : []));
        controls.push({ wrap: w, el: input, schema: s, out });
        parts.push(w);
      }
      themeSel = el('select', { className: 'viz-select' }, ...host.themes().map((x) => option(x.name, t(x.labelKey), x.name === host.themeName())));
      themeSel.addEventListener('change', () => host.setThemeName(themeSel.value));
      parts.push(field('vz-theme', t('viz.theme'), themeSel));
      const gs = host.groups();
      if (gs.length) {
        parts.push(el('div', { className: 'row', style: 'margin-top:.6rem' }, ...gs.map((g, i) => el('button', {
          type: 'button', on: { click: () => { const r = host.stepGroup(i, t); if (r?.text) setText(note, r.text); } },
        }, '⟳ ' + t(g.labelKey)))));
      }
    }
    parts.push(note);
    options.replaceChildren(...parts);
    builtFor = p ? p.id : null;
    refresh();
  }

  // values and visibility from the host, in place (focus stays)
  function refresh() {
    const p = host.plugin();
    if (modeSel && modeSel.value !== host.mode()) modeSel.value = host.mode();
    if (!p) return;
    if (presetSel) { const v = String(p.preset); if (presetSel.value !== v) presetSel.value = v; }
    if (themeSel && themeSel.value !== host.themeName()) themeSel.value = host.themeName();
    for (const c of controls) {
      const v = p.cfg[c.schema.key];
      c.wrap.hidden = typeof c.schema.showWhen === 'function' ? !c.schema.showWhen(p.cfg) : false;
      if (c.schema.type === 'toggle') c.el.checked = !!v;
      else if (c.el.value !== String(v)) c.el.value = String(v);
      if (c.out) c.out.textContent = c.schema.format === 'hue' ? `${Number(v) * 10}°` : String(v);
    }
  }

  function sync() {
    setText(name, modeLabel());
    const p = host.plugin();
    if ((p ? p.id : null) !== builtFor || !modeSel) build(); else refresh();
  }

  // a language change: the mode name and every label are looked up again (the controls are rebuilt, the host's state is untouched)
  I18N.onChange(() => { note.textContent = ''; modeSel = null; sync(); });
  host.mount(stage);
  host.onChange(sync);
  host.setEnabled(true);
  sync();
  // the stage's size follows the layout (phone / two columns / rotation)
  try { new ResizeObserver(() => host.resize()).observe(stage); } catch { window.addEventListener('resize', () => host.resize()); }
  return { sync };
}
