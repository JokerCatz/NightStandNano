# NightStand Nano control page

The only user interface of the box: a web page that talks to it over Bluetooth LE (Web Bluetooth). Phone first, works on a desktop up to 1024 px of content.
No framework, no TypeScript: plain modern JavaScript (ES modules) and CSS in `src/`, bundled by `esbuild` into `dist/`, which is **committed** and what GitHub Pages serves.

```
web/
  src/index.html        markup (+ the screen map and the URL parameters in its header comment)
  src/css/              base.css (tokens, controls) · layout.css (shell, navigation, control bar, breakpoints) · components.css
  src/js/main.js        entry: starts every module in the right order
  src/js/core/          app (shared state, events), link (Bluetooth + command queue + reconnect), status (the 1 s poll), router, help,
                        session (what is read after a connect / when a page opens), bar (control bar), icons, i18n, strings
  src/js/pages/         one module per destination / settings page: now, lyrics, music, radio, ambient, settings (the grid), fx, night, keys,
                        device (Wi-Fi / time / card), diag, lang, set-output (volume limit, highest sample rate, DAC rate test), set-led,
                        fw (`ver` / `hw` read once per connection, the hardware block, the flasher entry)
  src/js/lib/           section libraries: wifi, time, card, eq, lrc, radio, viz (+ viz-ui, viz-tap), fw (parsers: ver, hw, ratetest, play errors)
  src/js/demo/box.js    the fake box for ?demo (bundled, only started with ?demo)
  src/vendor/tunetap/   unmodified copies from tunetap, see its README.md
  test/                 node --test unit tests (parsers, EQ, i18n)
  build.mjs  check.mjs  package.json
  dist/                 the build: index.html, app.js, app.css
```

## Commands

```sh
cd web
npm install            # once: installs esbuild 0.28.2 (the only dependency, pinned)
npm run build          # src/ -> dist/ (deterministic: rebuilding unchanged sources changes nothing)
npm test               # node --test test/
npm run check          # string table, node --check of every source, dist/ is up to date and uses relative URLs only
npm run dev            # builds into .dev/ with source maps, rebuilds on every change, serves http://localhost:8000/
```

Run the page without a box: open `dist/index.html` (also from `file://`) or `http://localhost:8000/?demo`. `?demo` plays against a fake box; the parameters
(`?demo&card=bad`, `&wifi=none`, `&clock=none`, `&drop=5`, `&cast=bad`, `&tags=long`, `&viz=cassette`, `?lang=en`, ...) are listed at the top of `src/index.html`.
Real Bluetooth needs `https://` or `http://localhost`.

## The rule for `dist/`

**`dist/` is committed and must be rebuilt in the same commit as every change under `src/`.** `npm run check` fails when it is stale. The build stamp shown in
Settings › Diagnostics is a hash of everything in `src/`, so it tells which sources a deployed page was built from.
`dist/app.js` is one classic script (an IIFE, not a module), `dist/index.html` references `app.css?v=<stamp>` and `app.js?v=<stamp>` relatively: it works from a
sub-path (GitHub Pages project site) and from `file://`.

## Structure of the UI

Routes are the URL hash, so Back and deep links work and a native app can mirror the same screen map: `#/now`, `#/now/lyrics`, `#/music`, `#/radio`, `#/ambient`,
`#/settings` and `#/settings/{output,fx,night,viz,wifi,time,card,keys,led,lang,diag}`. The control bar (title, transport, volume, output) is on every screen.
Explanations are elements with class `help`; warnings that matter are never given that class.

## Links to the other pages, firmware facts

The page, the flasher and the README point to each other. The flasher link is **relative** (`flash/`), so it works from the published site
(`https://jokercatz.github.io/NightStandNano/flash/`), from a fork and from a local copy of the whole site. Served on its own from `web/dist` (`npm run dev`,
`file://`) `flash/` does not exist and the link 404s: that is expected. The releases and repository links (`github.com/JokerCatz/NightStandNano`) open in a new
tab with `rel="noopener"`. They are in Settings > About, in Diagnostics, and (flasher only) as a compact entry on Now / Settings while not connected.

Read once per (re)connection: `ver` (shown in Diagnostics as firmware version next to the page version) and `hw` (read-only pins of the card and LED; they can only be
changed in the flasher page over USB). A LED disabled in the hardware settings replaces the brightness slider with a note. `ratetest status` is polled (1 s) only while a
test runs and Settings > Output & volume is the current route. A failed `play` (`play: ESP_ERR_...`) shows a message under the file list (Music); `.zip` files list as
folders with an archive icon. `?demo&hw=spi|ledoff|pending`, `&shed=1`, `&dac=none` fake the matching box states (see `src/index.html`).
