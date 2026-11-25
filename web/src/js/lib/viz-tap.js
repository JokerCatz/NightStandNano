// SPDX-License-Identifier: LGPL-2.1-or-later
// NightStand Nano: the audio tap the tunetap visualizer host reads, synthesized from the box's 32 band bytes.
//
// tunetap's tap (js/viz-audio.js createVizTap) gives the host  tap.read(needs) -> { rate, bins, freq: { l, r } }
// with freq = Float32Array of dB per FFT bin (an AnalyserNode, fftSize 2048, so 1024 bins, getFloatFrequencyData:
// a full-scale sine reads about -13.56 dB, silence -Infinity). viz/bands.js turns that back into levels with
//   level = (db + DB_CAL + 60) / 60  (clamped 0..1),   DB_CAL = 20 log10(2 / 0.42)
// i.e. level 0 at -60 dBFS and 1 at 0 dBFS (a full-scale sine, chipchill's scale).
//
// The box (app/main/spectrum.c) sends byte = (dBFS + 70) * 255 / 70 per log-spaced band, 40 Hz .. 16 kHz, with the
// same dBFS convention (a full-scale sine = 0 dB). So  dBFS = byte * 70 / 255 - 70,  and the analyser value that
// bands.js maps back to the same dBFS is  dBFS - DB_CAL.  Bytes 0 .. ~36 (-70 .. -60 dBFS) land below the
// floor and read as level 0; 255 reads as level 1.
//
// The 32 band values are spread over the 1024 bins by linear interpolation (in the byte domain, which is dB) between
// band centres on the log axis, both channels the same array (mono). Bins below 40 Hz and above 16 kHz repeat the
// first / last band. bands.js then takes the loudest bin of each of its 16 / 64 bands, so a flat input stays flat.
import { DB_CAL } from '../../vendor/tunetap/viz/bands.js';

export const NANO_BANDS = 32;
const RATE = 48000, FFT = 2048, BINS = FFT / 2;
const LO = 40, HI = 16000;

export function createBandTap(getBytes) {
  // per bin: the two bands it lies between and the weight of the second
  const i0 = new Uint8Array(BINS), i1 = new Uint8Array(BINS), w = new Float32Array(BINS);
  const span = Math.log(HI / LO);
  for (let k = 0; k < BINS; k++) {
    const f = (k * RATE) / FFT;
    // position in band-centre units: 0 = centre of band 0, 31 = centre of band 31
    let p = f > 0 ? (Math.log(f / LO) / span) * NANO_BANDS - 0.5 : 0;
    p = Math.min(Math.max(p, 0), NANO_BANDS - 1);
    const a = Math.min(Math.floor(p), NANO_BANDS - 2);
    i0[k] = a; i1[k] = a + 1; w[k] = p - a;
  }
  const db = new Float32Array(BINS);
  const result = { rate: RATE, bins: BINS, freq: { l: db, r: db } };
  const toDb = (b) => (b > 0 ? (b * 70) / 255 - 70 - DB_CAL : -Infinity);
  return {
    ready: true,
    needs: { freq: 'mono', time: 'none' },
    setNeeds() {},
    read() {
      const v = getBytes();
      for (let k = 0; k < BINS; k++) db[k] = toDb(v[i0[k]] * (1 - w[k]) + v[i1[k]] * w[k]);
      return result;
    },
    close() {},
  };
}
