// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "pc/pc_hls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

bool pc_hls_is_hls(const char *b)
{
    while (*b == ' ' || *b == '\r' || *b == '\n' || (unsigned char)*b == 0xEF || (unsigned char)*b == 0xBB || (unsigned char)*b == 0xBF) b++;
    return !strncmp(b, "#EXTM3U", 7) && strstr(b, "#EXT-X-") != NULL;
}

static size_t host_end(const char *u)            /* index just after "scheme://host[:port]" */
{
    const char *p = strstr(u, "://");
    p = p ? p + 3 : u;
    while (*p && *p != '/' && *p != '?' && *p != '#') p++;
    return (size_t)(p - u);
}

int pc_url_resolve(const char *base, const char *ref, char *out, size_t cap)
{
    int n;
    if (!strncasecmp(ref, "http://", 7) || !strncasecmp(ref, "https://", 8)) n = snprintf(out, cap, "%s", ref);
    else if (!strncmp(ref, "//", 2)) {
        const char *c = strstr(base, "://");
        n = snprintf(out, cap, "%.*s:%s", c ? (int)(c - base) : 4, c ? base : "http", ref);
    } else if (ref[0] == '/') n = snprintf(out, cap, "%.*s%s", (int)host_end(base), base, ref);
    else {                                            /* relative to the playlist's directory */
        size_t he = host_end(base), q = he + strcspn(base + he, "?#"), dir = he;
        for (size_t i = he; i < q; i++) if (base[i] == '/') dir = i + 1;
        char *path = malloc(2 * 1280), *norm = path ? path + 1280 : NULL;
        if (!path) return -1;
        const size_t pcap = 1280;
        snprintf(path, pcap, "%s%.*s%s", dir > he ? "" : "/", (int)(dir - he), base + he, ref);
        size_t o = 0, qp = strcspn(path, "?#");
        for (size_t i = 0; i < qp;) {                     /* drop "/." and fold "dir/.." */
            if (!strncmp(path + i, "/./", 3)) { i += 2; continue; }
            if (!strncmp(path + i, "/../", 4)) { while (o > 0 && norm[--o] != '/') {} i += 3; continue; }
            if (o < pcap - 1) norm[o++] = path[i];
            i++;
        }
        norm[o] = 0;
        n = snprintf(out, cap, "%.*s%s%s", (int)he, base, norm, path + qp);
        free(path);
    }
    return n < 0 || (size_t)n >= cap ? -1 : 0;
}

static bool has_ci(const char *hay, const char *needle) { return strcasestr(hay, needle) != NULL; }

/* the value of KEY= in an attribute list (quoted or not), "" when absent */
static void attr(const char *line, const char *key, char *out, size_t cap)
{
    out[0] = 0;
    size_t kl = strlen(key);
    for (const char *p = line; (p = strstr(p, key)) != NULL; p += kl) {
        if (p != line && p[-1] != ',' && p[-1] != ':') continue;
        if (p[kl] != '=') continue;
        const char *v = p + kl + 1;
        size_t n;
        if (*v == '"') { v++; n = strcspn(v, "\""); } else n = strcspn(v, ",");
        snprintf(out, cap, "%.*s", (int)n, v);
        return;
    }
}

/* codecs the box cannot decode, and video codecs (a variant with video is the last resort) */
static bool codec_bad(const char *c) { return has_ci(c, "ec-3") || has_ci(c, "ac-3") || has_ci(c, "ac-4") || has_ci(c, "flac") || has_ci(c, "opus") || has_ci(c, "alac"); }
static bool codec_video(const char *c) { return has_ci(c, "avc") || has_ci(c, "hvc") || has_ci(c, "hev") || has_ci(c, "vp0") || has_ci(c, "av01") || has_ci(c, "dvh"); }

static void title_of(const char *line, char *out)
{
    const char *c = strchr(line, ',');
    out[0] = 0;
    if (!c) return;
    c++;
    while (*c == ' ' || *c == '\t') c++;
    size_t n = strlen(c);
    while (n && (c[n - 1] == ' ' || c[n - 1] == '\t')) n--;
    if (!n || c[0] == '<' || (n == 7 && !strncasecmp(c, "no desc", 7))) return;      /* a placeholder some encoders write; an XML blob some stations put there is no title */
    snprintf(out, PC_HLS_TITLE, "%.*s", (int)n, c);
}

int pc_hls_parse_from(const char *body, const char *base, uint32_t max_bps, uint64_t from_seq, pc_hls_t *h)
{
    memset(h, 0, sizeof *h);
    if (!pc_hls_is_hls(body)) return -1;
    /* variant candidates: per tier (0 audio-only, 1 audio rendition, 2 other) the best one under max_bps and the lowest */
    long bw = -1;
    int tier = 2;
    char (*bestu)[PC_HLS_URL] = h->cand_best, (*lowu)[PC_HLS_URL] = h->cand_low;
    long bestb[3] = { -1, -1, -1 }, lowb[3] = { -1, -1, -1 };
    bool any_variant = false, any_ok = false;
    char *media_uri = h->media;
    uint64_t idx = 0;
    char ttl[PC_HLS_TITLE] = "";
    for (const char *p = body; *p;) {
        const char *e = strpbrk(p, "\r\n");
        size_t len = e ? (size_t)(e - p) : strlen(p);
        char *line = h->scratch_line;
        snprintf(line, sizeof h->scratch_line, "%.*s", (int)(len < sizeof h->scratch_line ? len : sizeof h->scratch_line - 1), p);
        p += len;
        while (*p == '\r' || *p == '\n') p++;
        if (!line[0]) continue;
        if (!strncmp(line, "#EXT-X-STREAM-INF:", 18)) {
            h->master = true;
            char v[160], codecs[160];
            attr(line, "BANDWIDTH", v, sizeof v);
            bw = v[0] ? strtol(v, NULL, 10) : 0;
            attr(line, "CODECS", codecs, sizeof codecs);
            attr(line, "RESOLUTION", v, sizeof v);
            any_variant = true;
            if (codec_bad(codecs)) bw = -2;                       /* skipped below */
            else { any_ok = true; tier = (v[0] || codec_video(codecs)) ? 2 : 0; }
        } else if (!strncmp(line, "#EXT-X-MEDIA:", 13)) {
            char t[32];
            char *u = h->scratch_abs;
            attr(line, "TYPE", t, sizeof t);
            attr(line, "URI", u, PC_HLS_URL);
            if (!strcasecmp(t, "AUDIO") && u[0] && !media_uri[0]) {
                snprintf(media_uri, PC_HLS_URL, "%s", u);
                if (pc_url_resolve(base, media_uri, h->scratch_abs, PC_HLS_URL)) media_uri[0] = 0; else snprintf(media_uri, PC_HLS_URL, "%s", h->scratch_abs);
            }
        } else if (!strncmp(line, "#EXT-X-MEDIA-SEQUENCE:", 22)) h->first_seq = strtoull(line + 22, NULL, 10);
        else if (!strncmp(line, "#EXT-X-TARGETDURATION:", 22)) h->target_s = atoi(line + 22);
        else if (!strncmp(line, "#EXT-X-ENDLIST", 14)) h->ended = true;
        else if (!strncmp(line, "#EXT-X-KEY:", 11)) { char m[32]; attr(line, "METHOD", m, sizeof m); if (m[0] && strcasecmp(m, "NONE")) h->encrypted = true; }
        else if (!strncmp(line, "#EXT-X-MAP:", 11)) h->fmp4 = true;
        else if (!strncmp(line, "#EXTINF:", 8)) title_of(line, ttl);
        else if (line[0] != '#') {
            char *abs = h->scratch_abs;
            if (pc_url_resolve(base, line, abs, PC_HLS_URL)) { ttl[0] = 0; bw = -1; continue; }
            if (bw >= 0 || bw == -2) {                            /* a variant */
                if (bw >= 0) {
                    if (bw <= (long)max_bps && bw > bestb[tier]) { bestb[tier] = bw; snprintf(bestu[tier], sizeof bestu[0], "%s", abs); }
                    if (lowb[tier] < 0 || bw < lowb[tier]) { lowb[tier] = bw; snprintf(lowu[tier], sizeof lowu[0], "%s", abs); }
                }
                bw = -1;
            } else {                                              /* a segment */
                uint64_t seq = h->first_seq + idx++;
                if (from_seq == UINT64_MAX) {                     /* keep the newest PC_HLS_MAX_SEG */
                    if (h->nseg == PC_HLS_MAX_SEG) {
                        memmove(h->seg[0], h->seg[1], sizeof h->seg[0] * (PC_HLS_MAX_SEG - 1));
                        memmove(&h->seg_seq[0], &h->seg_seq[1], sizeof h->seg_seq[0] * (PC_HLS_MAX_SEG - 1));
                        memmove(h->title[0], h->title[1], sizeof h->title[0] * (PC_HLS_MAX_SEG - 1));
                        h->nseg--;
                    }
                } else if (seq < from_seq) { ttl[0] = 0; continue; }
                else if (h->nseg == PC_HLS_MAX_SEG) { h->more = true; ttl[0] = 0; continue; }
                snprintf(h->seg[h->nseg], sizeof h->seg[0], "%s", abs);
                snprintf(h->title[h->nseg], PC_HLS_TITLE, "%s", ttl);
                h->seg_seq[h->nseg++] = seq;
                ttl[0] = 0;
            }
        }
    }
    if (h->master) {
        const char *pick = NULL;
        long pb = 0;
        if (bestu[0][0]) { pick = bestu[0]; pb = bestb[0]; }
        else if (lowu[0][0]) { pick = lowu[0]; pb = lowb[0]; }
        else if (media_uri[0]) pick = media_uri;
        else if (bestu[2][0]) { pick = bestu[2]; pb = bestb[2]; }
        else if (lowu[2][0]) { pick = lowu[2]; pb = lowb[2]; }
        if (pick) { snprintf(h->variant, sizeof h->variant, "%s", pick); h->variant_bps = (uint32_t)pb; }
        h->no_audio = any_variant && !any_ok && !media_uri[0];
        return h->variant[0] ? 0 : -1;
    }
    (void)any_variant;
    if (h->target_s <= 0) h->target_s = 6;
    return h->nseg ? 0 : -1;
}

int pc_hls_parse(const char *body, const char *base, uint32_t max_bps, pc_hls_t *h)
{
    return pc_hls_parse_from(body, base, max_bps, UINT64_MAX, h);
}

int pc_hls_next(const pc_hls_t *h, uint64_t last)
{
    if (!h->nseg) return -1;
    if (last == UINT64_MAX || h->seg_seq[h->nseg - 1] < last)       /* first time, or the numbering restarted */
        return h->nseg > 3 ? h->nseg - 3 : 0;                         /* a little behind live: room to buffer */
    for (int i = 0; i < h->nseg; i++) if (h->seg_seq[i] > last) return i;
    return -1;
}
