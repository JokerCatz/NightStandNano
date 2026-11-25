// SPDX-License-Identifier: LGPL-2.1-or-later
/* Musepack SV7 / SV8 decoder backend (libmpcdec, BSD-3-Clause, components/libmpcdec). The decoder reads the file through a pc_reader_t
 * (no stdio). Float output in [-1, 1]; mono is played on both sides. 32 / 37.8 / 44.1 / 48 kHz, the rate stage downstream takes care of
 * the odd one. Resume token (byte_pos): the PCM frame (sample) index; open() seeks there with libmpcdec's seek table. */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mpc/mpcdec.h>
#include "pc/pc_backend.h"

typedef struct {
    pc_reader_t r;
    uint32_t off;                    /* read position of the libmpcdec reader */
    mpc_reader rd;
    mpc_demux *d;
    mpc_streaminfo si;
    mpc_frame_info fi;
    int avail, used;                 /* samples (frames) in buf, consumed */
    uint64_t pos;                    /* PCM frames handed out */
    uint64_t total;                  /* PCM frames in the file */
    bool ended;
    MPC_SAMPLE_FORMAT buf[MPC_DECODER_BUFFER_LENGTH];
} mpc_t;

static mpc_int32_t rd_read(mpc_reader *p, void *dst, mpc_int32_t n)
{
    mpc_t *m = p->data;
    if (n <= 0 || m->off >= m->r.size) return 0;
    if ((uint32_t)n > m->r.size - m->off) n = (mpc_int32_t)(m->r.size - m->off);
    int got = m->r.read(m->r.u, m->off, dst, (uint32_t)n);
    if (got < 0) return -1;
    m->off += (uint32_t)got;
    return got;
}
static mpc_bool_t rd_seek(mpc_reader *p, mpc_int32_t offset)
{
    mpc_t *m = p->data;
    if (offset < 0 || (uint32_t)offset > m->r.size) return MPC_FALSE;
    m->off = (uint32_t)offset;
    return MPC_TRUE;
}
static mpc_int32_t rd_tell(mpc_reader *p) { return (mpc_int32_t)((mpc_t *)p->data)->off; }
static mpc_int32_t rd_size(mpc_reader *p) { return (mpc_int32_t)((mpc_t *)p->data)->r.size; }
static mpc_bool_t rd_canseek(mpc_reader *p) { (void)p; return MPC_TRUE; }

static void m_close(void *h)
{
    mpc_t *m = h;
    if (!m) return;
    if (m->d) mpc_demux_exit(m->d);
    free(m);
}

static int m_open(void **h, const pc_reader_t *r, uint32_t start_off)
{
    if (!r->read || r->size < 8 || r->size > 0x7fffffffu) return -1;
    mpc_t *m = calloc(1, sizeof *m);
    if (!m) return -1;
    m->r = *r;
    if (m->r.u == r) m->r.u = &m->r;         /* memory readers point at themselves */
    m->rd = (mpc_reader){ .read = rd_read, .seek = rd_seek, .tell = rd_tell, .get_size = rd_size, .canseek = rd_canseek, .data = m };
    m->d = mpc_demux_init(&m->rd);
    if (!m->d) { m_close(m); return -1; }
    mpc_demux_get_info(m->d, &m->si);
    if (m->si.channels < 1 || m->si.channels > 2 || m->si.sample_freq < 8000) { m_close(m); return -1; }
    int64_t t = mpc_streaminfo_get_length_samples(&m->si);
    m->total = t > 0 ? (uint64_t)t : 0;
    m->fi.buffer = m->buf;
    if (start_off && start_off < m->total) {
        if (mpc_demux_seek_sample(m->d, start_off) != MPC_STATUS_OK) { m_close(m); return -1; }
        m->pos = start_off;
    }
    *h = m;
    return 0;
}

static int m_render(void *h, float *out, int frames)
{
    mpc_t *m = h;
    int done = 0;
    while (done < frames) {
        if (m->used >= m->avail) {
            if (m->ended) break;
            m->fi.buffer = m->buf;
            mpc_status st = mpc_demux_decode(m->d, &m->fi);
            if (st != MPC_STATUS_OK || m->fi.bits == -1) { m->ended = true; if (st != MPC_STATUS_OK && !done && m->pos == 0) return -1; break; }
            m->avail = (int)m->fi.samples;
            m->used = 0;
            if (m->avail <= 0) continue;
        }
        int k = m->avail - m->used;
        if (k > frames - done) k = frames - done;
        const float *s = m->buf + (size_t)m->used * m->si.channels;
        for (int i = 0; i < k; i++) {
            float l = s[i * m->si.channels], r = m->si.channels > 1 ? s[i * m->si.channels + 1] : l;
            out[2 * (done + i)] = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
            out[2 * (done + i) + 1] = r > 1.0f ? 1.0f : r < -1.0f ? -1.0f : r;
        }
        m->used += k;
        m->pos += (uint64_t)k;
        done += k;
    }
    return done;
}

static uint32_t m_rate(void *h) { return (uint32_t)((mpc_t *)h)->si.sample_freq; }
static int32_t m_length(void *h) { mpc_t *m = h; return m->total ? (int32_t)(m->total * 1000 / (uint64_t)m->si.sample_freq) : -1; }
static int32_t m_position(void *h) { mpc_t *m = h; return (int32_t)(m->pos * 1000 / (uint64_t)m->si.sample_freq); }
static uint32_t m_byte_pos(void *h) { mpc_t *m = h; return m->ended ? 0 : (uint32_t)m->pos; }

const pc_backend_t pc_backend_mpc = {
    .open = m_open, .render = m_render, .rate = m_rate, .length_ms = m_length, .position_ms = m_position, .close = m_close,
    .byte_pos = m_byte_pos, .name = "MPC",
};
