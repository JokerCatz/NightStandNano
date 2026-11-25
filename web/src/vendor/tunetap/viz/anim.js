// SPDX-License-Identifier: LGPL-2.1-or-later
// How far an animation moves in one frame. Everything that moves on a page is driven by elapsed
// time, never by a count of frames: what rgxx wrote as one step per frame keeps an accumulator
// and asks here how many whole steps this frame covers (one every 1/30 s), so the rate a page is
// drawn at never changes how fast it runs. Ported from chipchill crates/chipchill-viz/src/anim.rs.
// The spectrum's own ballistics are dt-based (bands.js) and need no steps; later pages (scope,
// goniometer, waterfall, drive) use these. The accumulator is an object { v } (chipchill: &mut f32).

const F = Math.fround;
export const STEP_S = F(1 / 30); // what "once a frame" used to mean
export const STEP_CAP = 30; // a second of steps: a slower frame has nothing more to show

function take(acc, dt, stepS, cap, keep) {
  if (stepS <= 0 || cap <= 0) return 0;
  if (dt > 0) acc.v = F(acc.v + dt);
  if (acc.v < stepS) return 0;
  let n = Math.max(Math.trunc(F(acc.v / stepS)), 1);
  if (n > cap || !keep) {
    n = Math.min(n, cap);
    acc.v = 0;
  } else {
    acc.v = F(acc.v - F(n * stepS)); // exact at dt = stepS: back to 0
  }
  return n;
}

// dt into acc; out come the whole steps it now holds, at most cap. The remainder is carried; what
// the cap left over is dropped.
export function steps(acc, dt, stepS, cap) { return take(acc, dt, F(stepS), cap, true); }

// The same, dropping the remainder (rgxx's waterfall tick).
export function ticks(acc, dt, stepS, cap) { return take(acc, dt, F(stepS), cap, false); }

// A fall that was one multiplication a frame, over n steps of them.
export function decay(perStep, n) {
  if (n <= 0) return 1;
  if (n === 1) return F(perStep);
  return F(Math.pow(F(perStep), n));
}
