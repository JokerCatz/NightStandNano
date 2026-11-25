// SPDX-License-Identifier: LGPL-2.1-or-later
// The pages' bands: the spectrum per channel as levels 0..1, log-spaced 40 Hz to 16 kHz (16 bands,
// and 64 fine ones), up at once, down 1.5 of full scale a second, with caps that hold 0.35 s and
// then fall 0.6 a second. Ported from chipchill crates/chipchill-viz/src/analysis.rs (rgxx
// viz_spectrum) and view.rs (Bands); time-based (dt seconds), f32 as chipchill.
//
// Difference: chipchill runs its own Hann FFT on the samples; here the input is what an
// AnalyserNode gives (getFloatFrequencyData, fftSize 2048, smoothing 0): dB per bin, Blackman
// window, scaled 1/N. chipchill's scale puts a full-scale sine at 0 dB (Hann gain 0.5, one-sided
// x2: norm 4/N); the analyser puts it at 20*log10(0.42 / 2) = -13.56 dB (Blackman coherent gain
// 0.42, one-sided magnitude A/2). DB_CAL adds that back so the floor (-60 dB) and the full scale
// mean what they mean in chipchill. The loudest bin of a band is its loudest dB (log is monotonic).

const F = Math.fround;
export const BANDS = 16;
export const FINE = 64;
export const FFT = 2048;
export const FLOOR_DB = -60;
export const FALL_PER_S = 1.5;
export const PEAK_HOLD_S = 0.35;
export const PEAK_FALL_PER_S = 0.6;
export const DB_CAL = 20 * Math.log10(2 / 0.42);

// { level, peak, hold, fine, finePeak, fineHold }: each [L, R] Float32Arrays.
export function createBands() {
  const two = (n) => [new Float32Array(n), new Float32Array(n)];
  return { level: two(BANDS), peak: two(BANDS), hold: two(BANDS), fine: two(FINE), finePeak: two(FINE), fineHold: two(FINE) };
}

export function resetBands(b) {
  for (const k of ['level', 'peak', 'hold', 'fine', 'finePeak', 'fineHold']) { b[k][0].fill(0); b[k][1].fill(0); }
}

// n log-spaced bands from 40 Hz to 16 kHz as FFT bin edges (n + 1 of them); low down a band is
// one bin wide.
export function bandEdges(n, rate, fft = FFT) {
  const e = new Int32Array(n + 1);
  for (let b = 0; b <= n; b++) {
    const f = 40 * Math.pow(16000 / 40, b / n);
    e[b] = Math.trunc((f * fft) / rate + 0.5);
    if (b > 0 && e[b] <= e[b - 1]) e[b] = e[b - 1] + 1;
  }
  return e;
}

// An analyser dB value as a level 0..1 (chipchill follow's first two lines, calibrated).
export function dbToLevel(db) {
  const lv = F((db + DB_CAL - FLOOR_DB) / -FLOOR_DB);
  return lv > 0 ? (lv < 1 ? lv : 1) : 0; // NaN and -Infinity (digital silence) are 0
}

// A band follows what the analyser found: up at once, down smoothly, with a cap that holds and
// then falls (analysis.rs follow, operation for operation).
function follow(lv, dt, level, peak, hold, i) {
  const cur = F(level[i] - F(FALL_PER_S * dt));
  level[i] = lv > cur ? lv : cur < 0 ? 0 : cur;
  if (level[i] >= peak[i]) {
    peak[i] = level[i];
    hold[i] = PEAK_HOLD_S;
  } else {
    hold[i] = F(hold[i] - dt);
    if (hold[i] < 0) {
      peak[i] = F(peak[i] - F(PEAK_FALL_PER_S * dt));
      if (peak[i] < 0) peak[i] = 0;
    }
  }
}

export class BandAnalyser {
  constructor() {
    this.key = '';
    this.edges = null;
    this.fineEdges = null;
  }

  // freq: { l, r } Float32Array dB per bin (r may be the same array as l: a mono stream);
  // rate: the context's sample rate; fft: the analyser's fftSize; dt: seconds since the last run.
  run(b, freq, rate, fft, dt) {
    const k = rate + ':' + fft;
    if (k !== this.key) {
      this.edges = bandEdges(BANDS, rate, fft);
      this.fineEdges = bandEdges(FINE, rate, fft);
      this.key = k;
    }
    dt = F(dt > 0 ? dt : 0);
    const chans = [freq?.l, freq?.r ?? freq?.l];
    for (let ch = 0; ch < 2; ch++) {
      const db = chans[ch];
      for (let band = 0; band < BANDS; band++) {
        follow(dbToLevel(loudest(db, this.edges[band], this.edges[band + 1], fft)), dt, b.level[ch], b.peak[ch], b.hold[ch], band);
      }
      for (let band = 0; band < FINE; band++) {
        follow(dbToLevel(loudest(db, this.fineEdges[band], this.fineEdges[band + 1], fft)), dt, b.fine[ch], b.finePeak[ch], b.fineHold[ch], band);
      }
    }
  }
}

function loudest(db, from, to, fft) {
  let m = -Infinity;
  if (!db) return m;
  const top = Math.min(db.length, fft / 2);
  for (let i = Math.max(from, 1); i < to && i < top; i++) if (db[i] > m) m = db[i]; // bin 0 has no mirror image
  return m;
}
