// SPDX-License-Identifier: LGPL-2.1-or-later
// What 夜行's weather hears of the music (天氣隨音樂): rain flares on the kick and thickens with
// the loudness, snow is blown sideways by a gust and its big flakes glint on a cymbal, the fog
// breathes. Every one of these needs a quantity that remembers (a slow average, a decay, an onset),
// and all of them live here, in one object advanced once a frame. Ported from chipchill
// crates/chipchill-viz/src/react.rs (rgxx drive_react.c), operation for operation in f32
// (Math.fround), so its numbers are chipchill's (tests/viz-drive-react.test.mjs checks a trace).
//
// Time, never frames: the frame's dt goes through anim.js, so one step is 1/30 s at every rate and
// a frame that covers a whole second does a second's worth and no more.
//
// Both onsets are the same shape: a rise above the band's OWN recent behaviour, never a fixed
// level and never a fixed fraction of it. A fixed level cannot work for the bass (an ordinary track
// keeps it above any threshold worth having); a fixed fraction of the average is a cliff on a loud,
// compressed master (the floor climbs towards the ceiling and the kick's headroom shrinks). So the
// margin is the signal's own: a hit is a rise that is large against how far the bass usually rises
// above its average BETWEEN hits.
import { steps, decay, STEP_S, STEP_CAP } from './anim.js';

const F = Math.fround;
// Per step (1/30 s), chosen by eye on rgxx's demo (chipchill's names).
const SLOW = F(0.983); // a two-second average: a chorus takes seconds to bring the rain up
const HI_SLOW = F(0.95); // two thirds of a second: the horizon a cymbal has to clear
const KICK_SLOW = F(0.97); // about a second: the same horizon for the kick
const GUST = F(0.92); // a whole second for a gust to blow itself out
const GLINT = F(0.62); // two frames: a flash on the flake, not a state it sits in
const KICK_DEV = F(0.97); // how long the kick's own deviation is averaged over
const KICK_A = F(0.03); // a rise is never less than this fraction of the floor…
const KICK_B = F(12); // …and must be this many of the kick's usual deviations…
const KICK_MARGIN_MIN = F(0.02); // …and never less than this
const KICK_ARM = F(0.5); // it fires again only once it has fallen back inside this part of the margin
const KICK_FLOOR = F(0.12); // never in what is nearly silence
const GUST_GAP = F(0.25); // seconds: two gusts are never closer than this
const GUST_FULL = F(0.3); // a rise of this much of the 0..1 scale blows the gust all the way
const HI_RISE = F(1.22); // the top bands rising this far above their average is a cymbal…
const HI_FLOOR = F(0.05); // …if they are louder than this at all

const sane = (v, fallback) => (Number.isFinite(v) ? F(v) : fallback);

export class React {
  constructor() {
    this.slow = 0; // a slow average of the overall loudness: how much rain, how thick the fog
    this.hiSlow = 0; // the same for the top bands: the horizon a cymbal has to rise above
    this.kickSlow = 0; // and for the bass: the horizon a hit has to rise above
    this.kickDev = 0; // how far the kick usually rises above that
    this.devPeak = 0; // the largest rise since the last step
    this.gust = 0; // the snow's sideways push: 1 at a kick, back to nothing over about a second
    this.glint = 0; // how much the big flakes catch the light just now: 1 at a cymbal
    this.since = 0; // seconds since the last gust
    this.acc = { v: 0 }; // what is left of the last frames' dt
    this.armed = false; // the gust can fire
    this.fires = 0; // how many gusts have been blown
    this.heard = false; // the first frame seeds the averages
  }

  // How far above its average the kick has to come to count as a hit, just now.
  kickMargin() {
    let m = F(KICK_A * this.kickSlow);
    const b = F(KICK_B * this.kickDev);
    if (b > m) m = b;
    return m > KICK_MARGIN_MIN ? m : KICK_MARGIN_MIN;
  }

  // One frame of dt seconds, with this frame's bass (0..1: the band loudness itself, not the drive's
  // peak-held kick), the overall loudness and the loudness of the top bands. Not moving (paused)
  // freezes all of it. A NaN or an infinity in any of them is ignored.
  step(dt, moving, bass, loud, high) {
    dt = sane(dt, 0);
    bass = sane(bass, this.kickSlow);
    loud = sane(loud, this.slow);
    high = sane(high, this.hiSlow);
    if (!this.heard) { // the first frame hears the song where it is
      this.slow = loud;
      this.hiSlow = high;
      this.kickSlow = bass;
      this.armed = true;
      this.since = GUST_GAP;
      this.heard = true;
    }
    if (!moving) return;
    this.since = F(this.since + dt);
    // How far the kick is above its average, read on EVERY frame and kept as a peak until a step
    // takes it. Only the rises count.
    const off = F(bass - this.kickSlow);
    const n = steps(this.acc, dt, STEP_S, STEP_CAP);
    if (n > 0) {
      let k = decay(SLOW, n);
      this.slow = F(this.slow + F(F(loud - this.slow) * F(1 - k)));
      k = decay(HI_SLOW, n);
      this.hiSlow = F(this.hiSlow + F(F(high - this.hiSlow) * F(1 - k)));
      k = decay(KICK_SLOW, n);
      this.kickSlow = F(this.kickSlow + F(off * F(1 - k)));
      k = decay(KICK_DEV, n);
      this.kickDev = F(this.kickDev + F(F(this.devPeak - this.kickDev) * F(1 - k)));
      this.devPeak = 0;
      this.gust = F(this.gust * decay(GUST, n));
      this.glint = F(this.glint * decay(GLINT, n));
    }
    // The onsets are read every frame, not every step.
    const margin = this.kickMargin();
    const rise = F(bass - this.kickSlow);
    if (rise <= F(margin * KICK_ARM)) this.armed = true;
    if (this.armed && bass > KICK_FLOOR && rise > margin && this.since >= GUST_GAP) {
      // a whisper of a kick is a puff, a hit a third of the whole scale a full gust
      const strength = F(F(0.5) + F(F(0.5) * (rise > GUST_FULL ? 1 : F(rise / GUST_FULL))));
      if (strength > this.gust) this.gust = strength;
      this.armed = false;
      this.since = 0;
      this.fires++;
    }
    // The deviation takes this frame's rise only if the frame was not itself a hit.
    if (this.armed && off > this.devPeak) this.devPeak = off;
    if (high > HI_FLOOR && high > F(this.hiSlow * HI_RISE)) this.glint = 1;
  }
}
