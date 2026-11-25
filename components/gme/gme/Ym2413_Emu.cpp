// YM2413 FM sound chip emulator interface: emu2413 (the same core Nes_Vrc7_Apu uses)
// JC 0007: the vendored file was the "chip disabled" stub (set_rate returned 2: VGM files with a YM2413 were refused). Upstream
// libgme 0.6.5 wraps emu2413 like this.

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/

#include "Ym2413_Emu.h"

#include <string.h>

extern "C" {
#include "emu2413.h"
}

Ym2413_Emu::Ym2413_Emu() { opll = 0; quiet = 0; }

Ym2413_Emu::~Ym2413_Emu()
{
	if ( opll )
		OPLL_delete( opll );
}

int Ym2413_Emu::set_rate( double sample_rate, double clock_rate )
{
	if ( opll )
	{
		OPLL_delete( opll );
		opll = 0;
	}

	opll = OPLL_new( (uint32_t) clock_rate, (uint32_t) sample_rate );
	if ( !opll )
		return 1;

	OPLL_setChipType( opll, 0 );
	OPLL_resetPatch( opll, 0 );
	reset();
	return 0;
}

void Ym2413_Emu::reset()
{
	OPLL_reset( opll );
	quiet = 0;
}

void Ym2413_Emu::write( int addr, int data )
{
	OPLL_writeIO( opll, 0, (uint8_t) addr );
	OPLL_writeIO( opll, 1, (uint8_t) data );
}

void Ym2413_Emu::mute_voices( int mask )
{
	OPLL_setMask( opll, (uint32_t) mask );
}

// JC 0011 (INV): idle skip. An OPLL whose carriers are all muted costs as much per sample as a playing one (about 760 host instructions
// per 49.7 kHz tick: 18 slot phase updates and the branches), and gme runs a silent emulator at 3x real time (Music_Emu silence_lookahead)
// until it has seen 6 s of silence, so the silent tail of a short KSS track was most of the board's 91 %. When the chip cannot change by
// itself and its output is zero, the samples are not computed until the next register write:
//   - no slot has a write waiting to be committed (update_requests), no mute mask, no test bits;
//   - every slot is in a fixed envelope state (emu2413's eg_idle: sustain/release at a fixed level or rate 0);
//   - every carrier is muted (eg_out above EG_MAX = EG_MUTE - 4 = 123: to_linear returns 0); in rhythm mode so are the hi-hat and tom slots
//     (even slots 14 and 16), which sound on their own;
//   - the converter history is flushed: the last 32 calculated samples were zero (`quiet`).
// Modulators need not be muted (a muted carrier ignores its input), only idle: a keyed-off modulator freezes at its level, so most
// instruments leave one unmuted for good. Skipped time still advances exactly (skip_ticks: counters, LFO, slot phases, noise register,
// sample clock). What is not advanced is the feedback history (the last two outputs) of an unmuted modulator with FB > 0; the first
// notes after a silence start from its old history instead of an evolved one. Measured against calculating every sample: bit-identical
// when no such modulator exists (puyo-puyo.kss tracks 0-9, test_gme_voices.c), otherwise 35-100 dB SNR over minutes of random notes
// and silences (about 100 samples out of 5 million differ, by up to 1500 of 32767, in the first milliseconds of a note).
static bool opll_silent( const OPLL* o )
{
	if ( o->mask || o->test_flag )
		return false;
	for ( int i = 0; i < 18; i++ )
	{
		const OPLL_SLOT& s = o->slot [i];
		if ( s.update_requests || !s.eg_idle ) // a write is not committed yet (also keeps the phase increments current for skip_ticks)
			return false;
		const bool muted = s.eg_out > 123;
		const bool sounds = (i & 1) || (o->rhythm_mode && (i == 14 || i == 16));
		if ( sounds && !muted )
			return false;
	}
	return true;
}

// emu2413's noise_steps: k (1..9) steps of `if (n & 1) n ^= 0x800200; n >>= 1` at once (closed form, same expression).
static inline uint32_t noise_steps( uint32_t n, int k )
{
	const uint32_t l = n & ((1u << k) - 1);
	return (n >> k) ^ (l << (23 - k)) ^ (l << (9 - k));
}

// The chip ticks that `n` output samples would have run (the loop of OPLL_calcStereo in closed form), applied without the per-slot work:
// eg_counter and the LFO phases advance by the tick count, each slot phase by its phase increment per LFO step (exact: the step changes
// every 1024 ticks, counted from the incremented value), the noise register tick by tick, and the sample clock by the same loop. Nothing else changes while the chip is silent (opll_silent).
static void skip_ticks( OPLL* o, int n )
{
	const uint64_t need = (uint64_t) n * o->out_step;
	uint64_t ticks = need > o->out_time ? (need - o->out_time + o->inp_step - 1) / o->inp_step : 0;
	o->out_time = (uint32_t) ((uint64_t) o->out_time + ticks * o->inp_step - need);
	while ( ticks )
	{
		const uint32_t q = o->pm_phase + 1;   // a tick increments pm_phase first, then takes the step from the new value
		uint32_t len = 1024 - (q & 1023);
		if ( len > ticks )
			len = (uint32_t) ticks;
		const uint32_t step = (q >> 10) & 7;
		for ( int i = 0; i < 18; i++ )
		{
			OPLL_SLOT& s = o->slot [i];
			s.pg_phase = (s.pg_phase + s.pg_inc [step] * len) & ((1u << 19) - 1);   // DP_BITS 19
			s.pg_out = s.pg_phase >> 9;                                             // DP_BASE_BITS = 19 - PG_BITS
		}
		for ( uint32_t t = 0; t < len; t++ )
		{
			o->noise = noise_steps( noise_steps( o->noise, 7 ), 7 );   // 14 + 2 + 2 steps of the noise register per tick, as update_output
			o->noise = noise_steps( noise_steps( o->noise, 2 ), 2 );
		}
		o->eg_counter += len;
		o->pm_phase += len;
		o->am_phase = (int32_t) ((uint32_t) o->am_phase + len);
		ticks -= len;
	}
}

void Ym2413_Emu::run( int pair_count, sample_t* out )
{
	while ( pair_count )
	{
		if ( quiet >= 32 && opll_silent( opll ) )
		{
			memset( out, 0, (size_t) pair_count * 2 * sizeof *out );
			skip_ticks( opll, pair_count );
			return;
		}
		int32_t buffer [2];
		OPLL_calcStereo( opll, buffer );
		*out++ = (sample_t) buffer [0];
		*out++ = (sample_t) buffer [1];
		pair_count--;
		if ( (buffer [0] | buffer [1]) == 0 && opll_silent( opll ) )
			quiet++;
		else
			quiet = 0;
	}
}
