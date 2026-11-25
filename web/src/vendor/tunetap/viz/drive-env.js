// SPDX-License-Identifier: LGPL-2.1-or-later
// What 夜行 takes from the world when its hour is set to follow it, and how an hour tints the sky.
// Ported from chipchill crates/chipchill-viz/src/env.rs (rgxx drive_env.c), less the forecast
// reader: tunetap fetches no forecast and asks for no location, so the weather is always the one
// the owner chose (or the station's own, 隨電台); there is no "follow the forecast" value and no
// WMO code table. The hour can follow the local clock (HOUR.AUTO, chipchill's 跟著時鐘), read
// through an injected Date.

export const HOUR = Object.freeze({ NIGHT: 0, DUSK: 1, DAWN: 2, DAY: 3, AUTO: 4 });
export const WEATHER = Object.freeze({ CLEAR: 0, RAIN: 1, SNOW: 2, FOG: 3 });

// The hour of the sky at a minute of the day: dawn 05:00-06:59, day 07:00-16:59, dusk
// 17:00-18:59, night the rest.
export function hourAt(m) {
  if (m >= 5 * 60 && m < 7 * 60) return HOUR.DAWN;
  if (m >= 7 * 60 && m < 17 * 60) return HOUR.DAY;
  if (m >= 17 * 60 && m < 19 * 60) return HOUR.DUSK;
  return HOUR.NIGHT;
}

// The sky of an hour: the colours its rows are pulled towards, at the top and at the horizon, how
// far (0..255; night 0: the routes' own skies), and how many of the stars stay (percent).
const TINTS = Object.freeze([
  Object.freeze({ top: [0, 0, 0], horizon: [0, 0, 0], amount: 0, stars: 100 }),
  Object.freeze({ top: [40, 10, 70], horizon: [255, 120, 60], amount: 140, stars: 40 }),
  Object.freeze({ top: [30, 40, 90], horizon: [255, 190, 120], amount: 140, stars: 40 }),
  Object.freeze({ top: [70, 140, 230], horizon: [200, 230, 250], amount: 191, stars: 0 }),
]);

// The tint of an hour; anything that is not one of the four is night.
export function tint(h) {
  return TINTS[Number.isInteger(h) && h >= 0 && h < HOUR.AUTO ? h : HOUR.NIGHT];
}

// Minutes since local midnight of a Date; null for no clock (chipchill's World.minute_of_day None).
export function minuteOfDay(d) {
  if (!d || typeof d.getHours !== 'function') return null;
  const m = d.getHours() * 60 + d.getMinutes();
  return Number.isFinite(m) ? m : null;
}
