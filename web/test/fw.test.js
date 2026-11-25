// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import * as F from '../src/js/lib/fw.js';
import { STRINGS } from '../src/js/core/strings.js';

test('ver: both version shapes, extra fields, junk', () => {
  assert.deepStrictEqual(F.parseVer(['ver 0.1.0-ee14d98-dirty idf=v6.1 chip=esp32s31 built=874f52d83']), { version: '0.1.0-ee14d98-dirty', idf: 'v6.1', chip: 'esp32s31', built: '874f52d83' });
  assert.strictEqual(F.parseVer(['ver v0.1.0 idf=v6.1 chip=esp32s31 built=abc']).version, 'v0.1.0');
  assert.deepStrictEqual(F.parseVer(['noise', 'ver 1.2.3']), { version: '1.2.3', idf: '', chip: '', built: '' });
  assert.strictEqual(F.parseVer(['unknown command']), null); assert.strictEqual(F.parseVer([]), null);
});

const SD = 'hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=mounted mode=sdmmc';
test('hw: SDMMC form', () => {
  const h = F.parseHw([SD, 'hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any', 'hw led gpio=60 order=grb on=1 src=default', 'hw reserved 26,27,28', 'hw gpios 0-61', 'hw pending 0']);
  assert.strictEqual(h.sd.mode, 'sdmmc'); assert.strictEqual(h.sd.clk, 24); assert.strictEqual(h.sd.d3, 23); assert.strictEqual(h.sd.width, 4); assert.strictEqual(h.sd.src, 'default'); assert.strictEqual(h.sd.state, 'mounted');
  assert.strictEqual(h.sdmmc.routing, 'iomux'); assert.deepStrictEqual(h.led, { gpio: 60, order: 'grb', on: 1, src: 'default' });
  assert.strictEqual(h.reserved, '26,27,28'); assert.strictEqual(h.gpios, '0-61'); assert.strictEqual(h.pending, 0);
});
test('hw: SPI form, LED off, pending', () => {
  const h = F.parseHw(['hw sd clk=18 mosi=19 miso=20 cs=21 src=custom state=none mode=spi', 'hw led gpio=8 order=rgb on=0 src=custom', 'hw pending 1']);
  assert.strictEqual(h.sd.mode, 'spi'); assert.strictEqual(h.sd.mosi, 19); assert.strictEqual(h.sd.cs, 21); assert.strictEqual(h.sd.src, 'custom'); assert.strictEqual(h.sdmmc, null);
  assert.strictEqual(h.led.on, 0); assert.strictEqual(h.pending, 1);
  assert.strictEqual(F.parseHw(['hw sd clk=1 mosi=2 miso=3 cs=4 src=custom state=mounted']).sd.mode, 'spi');   // no mode= field: told apart by its pins
});
test('hw: nothing recognisable', () => { assert.strictEqual(F.parseHw(['unknown command', '']), null); assert.strictEqual(F.parseHw([]), null); });

test('ratetest status: running, done, the table', () => {
  const r = F.parseRatetest(['ratetest state=running dac=06cb:1594 now=192000',
    'ratetest rate=96000 set=ok clock=96000 xfer_ok=750 xfer_bad=0 fps=95999 verdict=ok',
    'ratetest rate=192000 set=ok clock=192000 xfer_ok=600 xfer_bad=150 fps=191000 verdict=bad',
    'ratetest rate=384000 set=na clock=0 xfer_ok=0 xfer_bad=0 fps=0 verdict=untested']);
  assert.strictEqual(r.state, 'running'); assert.strictEqual(r.dac, '06cb:1594'); assert.strictEqual(r.now, 192000); assert.strictEqual(r.rates.length, 3); assert.strictEqual(r.max, null);
  assert.deepStrictEqual(r.rates[0], { rate: 96000, set: 'ok', clock: 96000, xferOk: 750, xferBad: 0, fps: 95999, verdict: 'ok' });
  assert.strictEqual(r.rates[1].verdict, 'bad'); assert.strictEqual(r.rates[1].xferBad, 150); assert.strictEqual(r.rates[2].set, 'na');
  const d = F.parseRatetest(['ratetest state=done dac=06cb:1594', 'ratetest rate=48000 set=ok clock=48000 xfer_ok=1 xfer_bad=0 fps=47999 verdict=ok', 'ratetest max=48000']);
  assert.strictEqual(d.state, 'done'); assert.strictEqual(d.max, 48000); assert.strictEqual(d.now, 0);
  assert.strictEqual(F.parseRatetest(['ratetest state=failed dac=0000:0000 reason="no sound"']).reason, 'no sound');
  assert.strictEqual(F.parseRatetest(['ratetest state=idle dac=06cb:1594']).rates.length, 0);
  assert.strictEqual(F.parseRatetest(['ratetest rate=96000 set=ok']), null);   // rows without a state line
  assert.strictEqual(F.parseRatetest(['unknown command']), null);
});
test('ratetest replies', () => {
  assert.deepStrictEqual(F.parseRatetestReply(['ratetest: started']), { ok: true });
  assert.deepStrictEqual(F.parseRatetestReply(['ratetest: cancelled']), { ok: true });
  assert.deepStrictEqual(F.parseRatetestReply(['ratetest: refused no dac']), { refused: 'no dac' });
  assert.deepStrictEqual(F.parseRatetestReply(['ratetest: refused card test running']), { refused: 'card test running' });
  assert.deepStrictEqual(F.parseRatetestReply(['ratetest: refused not running']), { refused: 'not running' });
  assert.strictEqual(F.parseRatetestReply(['ratetest state=running dac=1:2']).running, true);
  assert.strictEqual(F.parseRatetestReply([]), null);
});

test('play: error mapping', () => {
  assert.strictEqual(F.playError(['play: ESP_OK']), null);
  assert.strictEqual(F.playError([]), null);
  assert.deepStrictEqual(F.playError(['play: ESP_ERR_INVALID_SIZE']), { code: 'ESP_ERR_INVALID_SIZE', key: 'play.err.size' });
  assert.strictEqual(F.playError(['play: ESP_ERR_NOT_FOUND']).key, 'play.err.notfound');
  assert.strictEqual(F.playError(['play: ESP_ERR_TIMEOUT']).key, 'play.err.timeout');
  assert.strictEqual(F.playError(['play: ESP_ERR_INVALID_STATE']).key, 'play.err.busy');
  assert.strictEqual(F.playError(['play: ESP_ERR_NO_MEM']).key, 'play.err.nomem');
  assert.deepStrictEqual(F.playError(['play: ESP_FAIL']), { code: 'ESP_FAIL', key: 'play.err.other' });
  for (const k of ['size', 'notfound', 'timeout', 'busy', 'nomem', 'notsup', 'other']) assert.ok(STRINGS['play.err.' + k], k);
});
test('zip helpers, kHz', () => {
  assert.ok(F.isZipEntry('nes.zip/')); assert.ok(F.isZipEntry('X.ZIP/')); assert.ok(!F.isZipEntry('nes.zip')); assert.ok(!F.isZipEntry('Albums/'));
  assert.ok(F.inZip('/sdcard/packs/nes.zip')); assert.ok(F.inZip('/sdcard/packs/nes.zip/sub')); assert.ok(!F.inZip('/sdcard/packs')); assert.ok(!F.inZip('/sdcard/my.zipper'));
  assert.strictEqual(F.khz(96000), '96'); assert.strictEqual(F.khz(44100), '44.1'); assert.strictEqual(F.khz(384000), '384');
});

test('play: error detail (chips, too heavy, unknown) in three languages', async () => {
  const { I18N } = await import('../src/js/core/i18n.js');
  const say = (lang, e) => { I18N.setLang(lang); const v = F.playErrView(e, I18N.t('list.sep')); return I18N.t(v.key, v.p); };
  let e = F.playError(['play: ESP_ERR_NOT_SUPPORTED unsupported chip YM2151, OKIM6295']);
  assert.deepStrictEqual([e.key, e.chips], ['play.err.chip', ['YM2151', 'OKIM6295']]);
  assert.strictEqual(say('zh-TW', e), '這個檔案需要裝置還不支援的音源晶片：YM2151、OKIM6295');
  assert.ok(say('en', e).endsWith('YM2151, OKIM6295')); assert.ok(say('zh-CN', e).includes('音源芯片：YM2151、OKIM6295'));
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED unsupported chip SN76489 + YM2151 (mixed)']);
  assert.deepStrictEqual([e.key, e.chips], ['play.err.chipMixed', ['SN76489 + YM2151']]);
  assert.strictEqual(say('zh-TW', e), '這個檔案需要裝置還不支援的音源晶片：SN76489 + YM2151（混用）');
  assert.ok(say('en', e).endsWith('SN76489 + YM2151 (mixed)'));
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED too heavy for cast']);
  assert.deepStrictEqual([e.key, e.out], ['play.err.heavyTry', 'cast']);
  assert.ok(say('zh-TW', e).includes('廣播（Auracast）') && say('zh-TW', e).includes('小尾巴')); assert.ok(say('zh-CN', e).includes('负载过高'));
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED too heavy for dac']); assert.strictEqual(e.key, 'play.err.heavy');
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED too heavy for i2s']); assert.ok(say('en', e).includes('too heavy for i2s'));
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED weird new reason']);
  assert.deepStrictEqual([e.key, e.detail], ['play.err.notsupD', 'weird new reason']); assert.ok(say('zh-TW', e).includes('weird new reason'));
  e = F.playError(['play: ESP_FAIL oops']); assert.strictEqual(e.key, 'play.err.otherD');
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED']); assert.deepStrictEqual(e, { code: 'ESP_ERR_NOT_SUPPORTED', key: 'play.err.notsup' });   // no detail: generic
  e = F.playError(['play: ESP_ERR_NOT_SUPPORTED unsupported chip ']); assert.strictEqual(e.detail, 'unsupported chip');   // no chip names: shown as received
  I18N.setLang('en');
});
