// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import { parseEvents, describeEvent, newEventState, eventsCmd, applyEvents, MAX_EVENTS } from '../src/js/lib/events.js';
import { boxUrl, toRow } from '../src/js/lib/radio.js';
import { STRINGS } from '../src/js/core/strings.js';

test('events: parse lines, junk ignored', () => {
  const r = parseEvents(['ev 5 12 wired c2 next', 'noise', 'ev 6 3 bt 0x44 play', 'ev end 7']);
  assert.deepStrictEqual(r, { entries: [{ seq: 5, age: 12, text: 'wired c2 next' }, { seq: 6, age: 3, text: 'bt 0x44 play' }], next: 7 });
  assert.deepStrictEqual(parseEvents(['ev end 1']), { entries: [], next: 1 });
  assert.strictEqual(parseEvents(['events: usage']).next, null);
});

test('events: incremental fetch, newest first, clock from age', () => {
  const st = newEventState();
  assert.strictEqual(eventsCmd(st), 'events');
  let r = applyEvents(st, ['ev 1 30 wired c1 play', 'ev 2 10 bt 0x4B next', 'ev end 3'], 100000);
  assert.deepStrictEqual([r.added, r.restart], [2, false]);
  assert.deepStrictEqual(st.list.map((e) => [e.text, e.at]), [['bt 0x4B next', 90000], ['wired c1 play', 70000]]);
  assert.strictEqual(eventsCmd(st), 'events 3');
  r = applyEvents(st, ['ev end 3'], 103000);
  assert.strictEqual(r.added, 0); assert.strictEqual(st.list.length, 2);
  r = applyEvents(st, ['ev 3 1 wired p1 vol+', 'ev end 4'], 105000);
  assert.strictEqual(st.list[0].text, 'wired p1 vol+'); assert.strictEqual(eventsCmd(st), 'events 4');
  assert.strictEqual(applyEvents(st, [], 1).added, 0); assert.strictEqual(st.next, 4);   // an empty reply changes nothing
});

test('events: sequence restart after a reboot', () => {
  const st = newEventState();
  applyEvents(st, ['ev 40 1 wired c1 play', 'ev end 41'], 1000);
  let r = applyEvents(st, ['ev end 2'], 4000);   // asked `events 41`, the box is at 2 now
  assert.strictEqual(r.restart, true); assert.strictEqual(eventsCmd(st), 'events');
  r = applyEvents(st, ['ev 1 2 bt 0x44 play', 'ev end 2'], 5000);   // the plain `events` reply: everything is new
  assert.strictEqual(r.added, 1); assert.strictEqual(st.list.length, 2); assert.strictEqual(st.list[0].text, 'bt 0x44 play'); assert.strictEqual(eventsCmd(st), 'events 2');
});

test('events: at most 100 kept', () => {
  const st = newEventState(), lines = [];
  for (let i = 1; i <= 150; i++) lines.push(`ev ${i} 0 wired c1 play`);
  applyEvents(st, [...lines, 'ev end 151'], 0);
  assert.strictEqual(st.list.length, MAX_EVENTS);
});

test('events: texts, AVRCP names, unknown as received', () => {
  assert.deepStrictEqual(describeEvent('wired c2 next'), { kind: 'wired', b: 'c', g: '2', a: 'next' });
  assert.deepStrictEqual(describeEvent('wired mh vol-'), { kind: 'wired', b: 'm', g: 'h', a: 'vol-' });
  const names = { 0x41: 'volup', 0x42: 'voldn', 0x43: 'mute', 0x44: 'play', 0x45: 'stop', 0x46: 'pause', 0x48: 'rew', 0x49: 'ff', 0x4b: 'fwd', 0x4c: 'bwd' };
  for (const [c, n] of Object.entries(names)) {
    const d = describeEvent('bt 0x' + (+c).toString(16) + ' ignored');
    assert.strictEqual(d.name, n); assert.ok(STRINGS['ev.k.' + n], n);
  }
  assert.deepStrictEqual(describeEvent('bt 0x5a ignored'), { kind: 'bt', code: '0x5A', name: null, a: 'ignored' });
  assert.deepStrictEqual(describeEvent('something new 7'), { kind: 'raw', text: 'something new 7' });
  assert.strictEqual(describeEvent('wired x9 play').kind, 'raw');
});

test('radio: url choice prefers a direct stream, HLS is acceptable now', () => {
  assert.strictEqual(boxUrl(['https://a.example/x.m3u8', 'https://a.example/x.mp3']), 'https://a.example/x.mp3');
  assert.strictEqual(boxUrl(['https://a.example/x.mp3', 'http://a.example/x.aac']), 'http://a.example/x.aac');
  assert.strictEqual(boxUrl(['https://a.example/live/master.m3u8?t=1']), 'https://a.example/live/master.m3u8?t=1');
  assert.strictEqual(boxUrl(['https://a.example/x.m3u8', 'http://a.example/y.M3U8']), 'http://a.example/y.M3U8');
  assert.strictEqual(boxUrl(['ftp://x/y', 'javascript:1']), null); assert.strictEqual(boxUrl([]), null);
});

test('radio: rows, HLS tag, soft warning stays for other codecs', () => {
  const st = (o) => ({ stationuuid: '8a1f0001-0000-4000-8000-000000000001', name: 'S', url: 'https://a.example/x.mp3', codec: 'MP3', bitrate: 128, ...o });
  let r = toRow(st({ url: 'https://a.example/x.m3u8', codec: 'AAC' }));
  assert.strictEqual(r.hls, true); assert.strictEqual(r.maybe, false); assert.ok(r.url.endsWith('.m3u8'));
  r = toRow(st({ codec: 'FLAC' })); assert.strictEqual(r.hls, false); assert.strictEqual(r.maybe, true);
  r = toRow(st({ url: 'https://a.example/x.m3u8', url_resolved: 'https://a.example/x.mp3' })); assert.strictEqual(r.hls, false);
  assert.strictEqual(toRow(st({ stationuuid: 'nope' })), null);
});
