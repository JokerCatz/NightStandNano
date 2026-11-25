// SPDX-License-Identifier: LGPL-2.1-or-later
#include "pc/pc_ts.h"
#include <string.h>

void pc_ts_init(pc_ts_t *t)
{
    memset(t, 0, sizeof *t);
    t->pmt_pid = t->audio_pid = t->stream_type = -1;
}

static void psi(pc_ts_t *t, int pid, const uint8_t *p, int n)
{
    if (n < 1 || 1 + p[0] >= n) return;
    const uint8_t *s = p + 1 + p[0];                      /* after the pointer field */
    int left = n - 1 - p[0];
    if (left < 8) return;
    int len = ((s[1] & 0x0F) << 8) | s[2], end = 3 + len - 4;   /* before the CRC */
    if (end > left) end = left;
    if (pid == 0 && s[0] == 0x00) {                       /* PAT: the first real program's PMT */
        for (int i = 8; i + 4 <= end; i += 4) {
            int prog = (s[i] << 8) | s[i + 1];
            if (prog) { t->pmt_pid = ((s[i + 2] & 0x1F) << 8) | s[i + 3]; return; }
        }
    } else if (pid == t->pmt_pid && s[0] == 0x02 && end >= 12) {   /* PMT: the audio stream */
        int pil = ((s[10] & 0x0F) << 8) | s[11];
        int best = -1, best_type = -1;
        for (int i = 12 + pil; i + 5 <= end;) {
            int type = s[i], es = ((s[i + 1] & 0x1F) << 8) | s[i + 2], eil = ((s[i + 3] & 0x0F) << 8) | s[i + 4];
            if ((type == 0x0F && best_type != 0x0F) || ((type == 0x03 || type == 0x04) && best < 0)) { best = es; best_type = type; }
            i += 5 + eil;
        }
        if (best >= 0) { t->audio_pid = best; t->stream_type = best_type; }
    }
}

static size_t packet(pc_ts_t *t, const uint8_t *k, uint8_t *out, size_t cap)
{
    int pid = ((k[1] & 0x1F) << 8) | k[2], pusi = k[1] & 0x40, afc = (k[3] >> 4) & 3;
    if (!(afc & 1)) return 0;                             /* no payload */
    int off = 4 + ((afc & 2) ? 1 + k[4] : 0);
    if (off >= 188) return 0;
    const uint8_t *p = k + off;
    int n = 188 - off;
    if (pid == 0 || (pid == t->pmt_pid && t->pmt_pid >= 0)) { if (pusi) psi(t, pid, p, n); return 0; }
    if (pid != t->audio_pid) return 0;
    if (pusi) {                                           /* a PES starts: skip its header */
        if (n < 9 || p[0] || p[1] || p[2] != 1) return 0;
        int h = 9 + p[8];
        if (h >= n) return 0;
        p += h;
        n -= h;
    }
    size_t w = (size_t)n < cap ? (size_t)n : cap;
    memcpy(out, p, w);
    return w;
}

size_t pc_ts_feed(pc_ts_t *t, const uint8_t *in, size_t n, uint8_t *out, size_t cap)
{
    size_t w = 0;
    for (size_t i = 0; i < n;) {
        if (t->pkt_n == 0 && in[i] != 0x47) { i++; continue; }   /* find the sync byte */
        size_t k = 188 - (size_t)t->pkt_n < n - i ? 188 - (size_t)t->pkt_n : n - i;
        memcpy(t->pkt + t->pkt_n, in + i, k);
        t->pkt_n += (int)k;
        i += k;
        if (t->pkt_n == 188) { w += packet(t, t->pkt, out + w, cap - w); t->pkt_n = 0; }
    }
    return w;
}
