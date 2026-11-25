// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import * as W from '../src/js/lib/wifi.js';

test('wifi: six states', () => {
  for (const s of ['connected', 'connecting', 'wrong password', 'not found', 'off', 'no credentials']) {
    const p = W.parseWifi([`wifi ${s} ip=192.0.2.5 rssi=-60 time=2026-11-25 10:00:00 src=nvs ssid="Net"`]);
    assert.strictEqual(p.state, s); assert.strictEqual(p.ip, '192.0.2.5'); assert.strictEqual(p.rssi, -60); assert.strictEqual(p.src, 'nvs'); assert.strictEqual(p.ssid, 'Net'); assert.strictEqual(p.synced, true);
  }
  assert.strictEqual(W.stateKey('wrong password'), 'wifi.state.wrong_password'); assert.strictEqual(W.stateKey('no credentials'), 'wifi.state.no_credentials');
});

test('wifi: unsynced time, card, escapes', () => {
  const p = W.parseWifi(['wifi connected ip=10.0.0.2 rssi=-71 time=(unsynced) 1970-01-01 00:00:12 src=card ssid="My \\"Quoted\\" \\\\ Net"']);
  assert.strictEqual(p.synced, false); assert.strictEqual(p.time, '1970-01-01 00:00:12'); assert.strictEqual(p.src, 'card'); assert.strictEqual(p.ssid, 'My "Quoted" \\ Net');
});

test('wifi: ssid that looks like fields', () => {
  const p = W.parseWifi(['wifi connected ip=1.2.3.4 rssi=-50 time=2026-01-01 00:00:00 src=none ssid="a src=nvs ssid=\\"x\\""']);
  assert.strictEqual(p.src, 'none'); assert.strictEqual(p.ssid, 'a src=nvs ssid="x"');
});

test('wifi: empty ssid, junk lines', () => {
  assert.strictEqual(W.parseWifi(['wifi no credentials ip=0.0.0.0 rssi=0 time=(unsynced) 1970-01-01 00:00:01 src=none ssid=""']).ssid, '');
  assert.strictEqual(W.parseWifi(['wifi set: ok, connecting', 'junk']), null);
});

test('scan parser', () => {
  const r = W.parseScan(['wifi ap -48 secured "Home"', 'wifi ap -61 open "Cafe \\"Blue\\" Guest"', 'wifi ap -80 secured "訪客"', 'wifi scan: 3 found']);
  assert.strictEqual(r.status, 'found'); assert.strictEqual(r.n, 3); assert.strictEqual(r.aps.length, 3);
  assert.deepStrictEqual(r.aps[1], { rssi: -61, secured: false, ssid: 'Cafe "Blue" Guest' }); assert.strictEqual(r.aps[0].secured, true);
  assert.strictEqual(W.parseScan(['wifi scan: busy']).status, 'busy'); assert.strictEqual(W.parseScan(['wifi scan: failed']).status, 'failed');
  assert.strictEqual(W.parseScan([]).status, null);
});

test('utf8 byte length', () => {
  assert.strictEqual(W.utf8Len('abc'), 3); assert.strictEqual(W.utf8Len('é'), 2); assert.strictEqual(W.utf8Len('中'), 3); assert.strictEqual(W.utf8Len('😀'), 4); assert.strictEqual(W.utf8Len(''), 0);
  assert.strictEqual(W.utf8Len('中'.repeat(10)), 30);
});

test('validation', () => {
  assert.strictEqual(W.validate('a'.repeat(32), ''), null);
  assert.deepStrictEqual(W.validate('a'.repeat(33), ''), { key: 'wifi.err.ssidLong', params: { n: 33 } });
  assert.strictEqual(W.validate('中'.repeat(10), ''), null);                   // 30 bytes: fine
  assert.strictEqual(W.validate('中'.repeat(11), '').params.n, 33);           // 11 chars but 33 bytes
  assert.strictEqual(W.validate('', '').key, 'wifi.err.ssidEmpty');
  assert.strictEqual(W.validate('a|b', '').key, 'wifi.err.bar');
  assert.deepStrictEqual(W.validate('ab', '1234567'), { key: 'wifi.err.pwLen', params: { n: 7 } });
  assert.strictEqual(W.validate('ab', '12345678'), null); assert.strictEqual(W.validate('ab', 'x'.repeat(63)), null);
  assert.strictEqual(W.validate('ab', 'x'.repeat(64)).params.n, 64);
  assert.strictEqual(W.validate('ab', '密碼'.repeat(2)), null);                 // 12 bytes
  assert.strictEqual(W.validate('ab', '密'.repeat(2)).params.n, 6);              // 2 chars, 6 bytes: too short
  assert.strictEqual(W.validate('a\nb', '').key, 'wifi.err.ctrl'); assert.strictEqual(W.validate('ab', 'x\ty12345').key, 'wifi.err.ctrl');
});

test('whyKey', () => {
  assert.strictEqual(W.whyKey('ssid must be 1-32 bytes'), 'wifi.why.ssid'); assert.strictEqual(W.whyKey('password must be empty or 8-63 bytes'), 'wifi.why.pw');
  assert.strictEqual(W.whyKey('usage: wifi set <ssid>|<password>'), 'wifi.why.usage'); assert.strictEqual(W.whyKey('control characters'), 'wifi.why.ctrl');
  assert.strictEqual(W.whyKey('cannot store'), 'wifi.why.store'); assert.strictEqual(W.whyKey('weird'), null);
});

test('log redaction', () => {
  assert.strictEqual(W.redact('wifi set MyNet|secretpw1'), 'wifi set MyNet|••••');
  assert.strictEqual(W.redact('wifi set My Net|pass|word'), 'wifi set My Net|••••');
  assert.strictEqual(W.redact('wifi set OpenNet|'), 'wifi set OpenNet|');
  assert.strictEqual(W.redact('wifi'), 'wifi'); assert.strictEqual(W.redact('wifi scan'), 'wifi scan'); assert.strictEqual(W.redact('vol 5|x'), 'vol 5|x');
  assert.ok(!W.redact('wifi set a|hunter2hunter2').includes('hunter'));
});

