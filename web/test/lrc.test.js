// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import { parseLrc, findLine, parseReply, decodeLrc } from '../src/js/lib/lrc.js';

test('lrc: tags, offset, word times, ordering', () => {
  const r = parseLrc("[ti:x]\r\n[offset:+500]\n[00:10.5][01:00]hi <00:10.50>there\n[00:05]\nnotime\n[00:20:50]a\n[1:02.123]b\n[00:03.00]c");
  assert.equal(r.lines[0].ms, 2500); assert.equal(r.lines[0].text, 'c');
  assert.equal(r.lines[1].text, ''); assert.equal(r.lines[2].text, 'hi there'); assert.equal(r.lines[2].ms, 10000);
  assert.equal(findLine(r.lines, 0), -1); assert.equal(findLine(r.lines, 2500), 0); assert.equal(findLine(r.lines, 99999), r.lines.length - 1);
});
test('lrc: Big5 and BOM bytes, reply framing', () => {
  const big5 = new Uint8Array([0x5b, 0x30, 0x30, 0x3a, 0x30, 0x31, 0x5d, 0xa7, 0x41, 0xa6, 0x6e]);   // [00:01]你好
  assert.equal(decodeLrc(big5), '[00:01]你好');
  const b64 = Buffer.from('﻿[00:01]歌詞').toString('base64');
  const rr = parseReply(['lrc begin 14', 'lrc+ ' + b64, 'lrc end']); assert.equal(rr.lines[0].text, '歌詞');
  assert.equal(parseReply(['lrc none']), null);
});
