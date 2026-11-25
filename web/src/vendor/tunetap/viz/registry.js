// SPDX-License-Identifier: LGPL-2.1-or-later
// The visual modes (plugins) the host can show, registered at startup by js/viz/index.js and
// loaded lazily on first use (design amendment A). A plugin is an in-repo ES module (CSP stays
// script-src 'self'); its definition is { id, labelKey, needsAudio, stillWithoutAudio, load: () =>
// import(...) }, the module exports create(args) -> instance (see host.js for the instance contract).
// needsAudio: the plugin reacts to the analyser (the host passes audio: null when there is none).
// stillWithoutAudio: with no audio there is nothing to animate (the spectrum): the host draws one
// resting frame instead of running the loop. Without it the loop runs on with audio: null (the
// drive's scene animates calmly).
// words: how the status line speaks of the data: 'spectrum' (the spectrum: "This station doesn't
// provide spectrum data", "Live spectrum") or 'audio' (the default, every other visual: "No audio
// data – not reacting to the music", "Reacting to the music") or 'none' (a visual that does not hear
// the music at all, the cassette: "This picture doesn't follow the music", whatever the station
// gives; register it with needsAudio: false, and the host reads nothing of the tap for it).
// chipchill has no registry (its pages are a fixed enum, pages/mod.rs); this is tunetap's.

const ID = /^[a-z][a-z0-9-]*$/;

export function createRegistry() {
  const defs = new Map();
  const given = new Map(); // id -> the definition object it was registered with
  const loads = new Map();
  return {
    register(def) {
      if (!def || typeof def.id !== 'string' || !ID.test(def.id)) throw new TypeError('a plugin needs an id of lower-case letters, digits and dashes');
      if (typeof def.load !== 'function') throw new TypeError(`plugin ${def.id}: load must be a function`);
      if (given.get(def.id) === def) return defs.get(def.id);
      if (defs.has(def.id)) throw new Error(`plugin ${def.id} is already registered`);
      const d = Object.freeze({ id: def.id, labelKey: def.labelKey ?? `viz.mode.${def.id}`, needsAudio: def.needsAudio !== false, stillWithoutAudio: !!def.stillWithoutAudio, words: def.words === 'spectrum' || def.words === 'none' ? def.words : 'audio', load: def.load });
      defs.set(def.id, d);
      given.set(def.id, def);
      return d;
    },
    get: (id) => defs.get(id) ?? null,
    list: () => [...defs.values()],
    // The plugin's module, imported once (a failed import may be tried again).
    load(id) {
      const d = defs.get(id);
      if (!d) return Promise.reject(new Error(`no plugin ${id}`));
      if (!loads.has(id)) {
        const p = Promise.resolve().then(() => d.load());
        loads.set(id, p);
        p.catch(() => { if (loads.get(id) === p) loads.delete(id); });
      }
      return loads.get(id);
    },
  };
}

// The app's registry.
export const registry = createRegistry();
export const registerPlugin = (def) => registry.register(def);
