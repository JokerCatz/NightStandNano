// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import * as T from '../src/js/lib/time.js';

test('TZ builder', () => {
  assert.strictEqual(T.buildTz(480, 'CST'), 'CST-8');   // UTC+8: sign inverted
  assert.strictEqual(T.buildTz(480, 'GMT+8'), 'UTC-8'); // not alphabetic -> UTC
  assert.strictEqual(T.buildTz(330, 'IST'), 'IST-5:30');
  assert.strictEqual(T.buildTz(-210, 'NST'), 'NST3:30');
  assert.strictEqual(T.buildTz(0, ''), 'UTC0'); assert.strictEqual(T.buildTz(0, 'UTC'), 'UTC0');
  assert.strictEqual(T.buildTz(-300, 'EST'), 'EST5'); assert.strictEqual(T.buildTz(-300, 'E'), 'UTC5'); assert.strictEqual(T.buildTz(-300, 'TOOLONG'), 'UTC5');
  assert.strictEqual(T.buildTz(765, undefined), 'UTC-12:45'); assert.strictEqual(T.buildTz(-720, 'ABCDE'), 'ABCDE12');
});

test('TZ from the browser (process TZ)', () => {
  const d = new Date(Date.UTC(2026, 10, 25, 12));   // the page calls tzString(new Date()) with the resolved zone
  const s = T.tzString(d, 'Asia/Taipei'); assert.ok(/^[A-Za-z]{3,5}-?\d{1,2}(:\d\d)?$/.test(s), s);
  console.log('    TZ=' + process.env.TZ + ' ->', s);
});

test('time line parser + local format', () => {
  const p = T.parseTime(['time 1790000000 2026-09-21T22:13:20+08:00 src=ntp tz="CST-8"']);
  assert.strictEqual(p.unix, 1790000000); assert.strictEqual(p.src, 'ntp'); assert.strictEqual(p.tz, 'CST-8'); assert.strictEqual(p.offsetMin, 480);
  assert.strictEqual(T.fmtLocal(p.unix, p.offsetMin), '2026-09-21 22:13:20');
  assert.strictEqual(T.parseTime(['time 5 1970-01-01T03:30:05-03:30 src=none tz="NST3:30"']).offsetMin, -210);
  assert.strictEqual(T.parseTime(['time 5 1970-01-01T00:00:05Z src=client tz="UTC0"']).offsetMin, 0);
  assert.strictEqual(T.parseTime(['time set: ok']), null);
});

