# Vendored from tunetap — do not edit

`viz/` and `radio/` are **unmodified copies** of files of the owner's other project, tunetap (`https://github.com/JokerCatz/tunetap`, directory `js/`):

| here | there |
|---|---|
| `viz/` (`anim.js bands.js cfg.js colour.js drive-env.js drive-react.js host.js labels.js page.js pixel.js registry.js tables.js themes.js`, `plugins/{cassette,drive,spectrum,waterfall}.js`) | `js/viz/` — the same files; tunetap's `index.js meter.js samples.js plugins/scope.js plugins/xy.js` are **not** copied (the box sends no raw samples) |
| `radio/{find-model,rb,regions,route}.js` | `js/{find-model,rb,regions,route}.js` — the four files side by side in tunetap's `js/` |

They must stay byte-identical to tunetap. Everything Nano-specific lives next to them in `src/js/lib/` (`viz.js`, `viz-ui.js`, `viz-tap.js`, `radio.js`), which import these files.

Re-sync after tunetap changes (from `web/`):

```sh
T=/path/to/tunetap/js   # a checkout of https://github.com/JokerCatz/tunetap
for f in $(cd src/vendor/tunetap/viz && find . -name '*.js'); do cp "$T/viz/$f" "src/vendor/tunetap/viz/$f"; done
for f in find-model rb regions route; do cp "$T/$f.js" "src/vendor/tunetap/radio/$f.js"; done
npm run build && npm run check
```

Verify they are still identical: `diff -r src/vendor/tunetap/viz $T/viz` shows only the five "Only in" lines for the files that are deliberately not copied, and `for f in src/vendor/tunetap/radio/*.js; do cmp $f $T/$(basename $f); done` prints nothing.
