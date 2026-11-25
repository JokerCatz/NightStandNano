// SPDX-License-Identifier: LGPL-2.1-or-later
import test from 'node:test';
import assert from 'node:assert';
import { STRINGS } from '../src/js/core/strings.js';
import { I18N } from '../src/js/core/i18n.js';

test('i18n: language tags', () => {
  const n = I18N.normalize;
  assert.equal(n('en-GB'), 'en'); assert.equal(n('zh-hk'), 'zh-TW'); assert.equal(n('zh-Hans-SG'), 'zh-CN'); assert.equal(n('zh'), 'zh-CN'); assert.equal(n('fr'), null);
  assert.equal(I18N.detect(['fr', 'zh-TW', 'en']), 'zh-TW'); assert.equal(I18N.detect(['fr']), 'en');
});
test('i18n: t() fills {params} and nested keys, falls back to the key', () => {
  assert.equal(I18N.t('timer.left', { m: 5 }).includes('5'), true);
  assert.equal(I18N.t('no.such.key'), 'no.such.key');
  assert.ok(I18N.t('ent.night', { s: { t: 'night.on' }, r: '22:00–06:00' }).includes('22:00'));
});
test('i18n: every row has all three languages, placeholders match', () => {
  const ph = s => (s.match(/\{\w+\}/g) || []).sort().join(',');
  for (const [k, row] of Object.entries(STRINGS)) {
    for (const l of ['en', 'zh-TW', 'zh-CN']) assert.ok(typeof row[l] === 'string' && row[l], k + ' lacks ' + l);
    assert.equal(new Set(['en', 'zh-TW', 'zh-CN'].map(l => ph(row[l]))).size, 1, 'placeholders differ in ' + k);
  }
});
