// ===== PRACTICE TRACKER B: do not modify =====
// Set TRACKER_GLITCHES to 1 or 0 *above* this block, never inside it.
static uint32_t trackerSeed = 4242u;
static uint32_t trackerRand() {
  trackerSeed = trackerSeed * 1664525u + 1013904223u;
  return trackerSeed;
}
float nextAltitude() {
  // This flight's secrets. Every tracker uses different ones.
  const int64_t H0 =  150000;   // altitude at t = 0, in cm
  const int64_t V0 =    4200;   // upward speed at t = 0, in cm/s
  const int64_t G  =     371;   // gravity, in cm/s^2
  const int64_t TD =   24000;   // parachute opens at this time, in ms
  const int64_t VD =    -850;   // speed under the parachute, in cm/s
  static int64_t t = 0;      // time of this reading, in ms
  static int quiet = 0;      // readings until a glitch is allowed again
  int64_t tc = (t < TD) ? t : TD;
  int64_t h = H0 + V0 * tc / 1000 - G * tc * tc / 2000000;
  if (t > TD) h += VD * (t - TD) / 1000;
  uint32_t r = trackerRand();
  if (quiet > 0) quiet--;
  if (TRACKER_GLITCHES && t >= 1000 && quiet == 0 && (r >> 24) < 2) {
    quiet = 10;                                     // glitch: 50 to 300 m off
    int64_t off = 5000 + (int64_t)((r >> 4) % 25001);
    h += ((r >> 3) & 1) ? off : -off;
    if (h < 0) h += 2 * off;
  } else {
    h += (int64_t)((r >> 8) % 401) - 200;           // noise: -2.00 to +2.00 m
  }
  t++;
  return (float)h / 100.0f;
}
// =========================================
