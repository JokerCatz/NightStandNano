// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import { EQ } from '../src/js/lib/eq.js';

test('eq: a 0 dB band is flat, a +6 dB peak peaks near its centre', () => {
  const flat = EQ.response([{ type: 0, fc: 1000, gain: 0, q: 1 }], 0, [100, 1000, 10000]);
  for (const d of flat) assert.ok(Math.abs(d) < 1e-9);
  const pk = EQ.response([{ type: 0, fc: 1000, gain: 6, q: 1 }], 0, [100, 1000, 10000]);
  assert.ok(Math.abs(pk[1] - 6) < 0.05); assert.ok(Math.abs(pk[0]) < 1.5); assert.ok(Math.abs(pk[2]) < 1.5);
});
test('eq: shelves and preamp', () => {
  const ls = EQ.response([{ type: 1, fc: 200, gain: 6, q: 0.7 }], 0, [20, 20000]);
  assert.ok(ls[0] > 5.5 && Math.abs(ls[1]) < 0.3);
  const hs = EQ.response([{ type: 2, fc: 5000, gain: -6, q: 0.7 }], -3, [100, 20000]);
  assert.ok(Math.abs(hs[0] + 3) < 0.3 && hs[1] < -8);
});
test('eq: AutoEq text', () => {
  const r = EQ.parseAutoEq('Preamp: -6.1 dB\nFilter 1: ON PK Fc 105 Hz Gain -3.2 dB Q 0.70\nFilter 2: OFF PK Fc 200 Hz Gain 1 dB Q 1\nFilter 3: ON LSC Fc 80 Hz Gain 2 dB Q 0.7\nFilter 4: ON LP Fc 9000 Hz\nFilter 5: ON HS Fc 9000 Hz Gain -2,5 dB Q 0.7');
  assert.equal(r.preamp, -6.1); assert.equal(r.bands.length, 3); assert.equal(r.skipped, 2);
  assert.deepEqual(r.bands.map(b => b.type), [0, 1, 2]); assert.equal(r.bands[2].gain, -2.5);
});
test('eq: AutoEq limits (10 bands, clamping)', () => {
  const text = Array.from({ length: 12 }, (_, i) => `Filter ${i + 1}: ON PK Fc ${30000 + i} Hz Gain 40 dB Q 50`).join('\n');
  const r = EQ.parseAutoEq(text);
  assert.equal(r.bands.length, EQ.MAX_BANDS); assert.equal(r.overflow, 2);
  assert.equal(r.bands[0].fc, 20000); assert.equal(r.bands[0].gain, 24); assert.equal(r.bands[0].q, 10);
});
