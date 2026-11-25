// Turbo Grafx 16 (PC Engine) PSG sound chip emulator

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef HES_APU_H
#define HES_APU_H

#include "blargg_common.h"
#include "Blip_Buffer.h"

struct Hes_Osc
{
	unsigned char wave [32];
	short volume [2];
	int last_amp [2];
	int delay;
	int period;
	unsigned char noise;
	unsigned char phase;
	unsigned char balance;
	unsigned char dac;
	blip_time_t last_time;

	Blip_Buffer* outputs [2];
	Blip_Buffer* chans [3];
	unsigned noise_lfsr;
	unsigned char control;

	static const unsigned int amp_range = 0x8000;
	typedef Blip_Synth<blip_med_quality,1> synth_t;

	void run_until( synth_t& synth, blip_time_t );
};

class Hes_Apu {
public:
	void treble_eq( blip_eq_t const& );
	void volume( double );

	static const int osc_count = 6;
	void osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right );

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only peek at oscillator i's last-computed output amplitude.
	// Hes_Osc::last_amp is per-channel (last_amp[0]=center/left,
	// last_amp[1]=right, see Hes_Osc::run_until in Hes_Apu.cpp: both are
	// `dac * volume[ch]`, dac a 5-bit unsigned wave/noise sample and
	// volume[ch] a non-negative log-table entry -- always >=0, unipolar,
	// range 0..amp_range (0x8000)). A hard-panned voice can leave one
	// channel silent while the other carries the audible signal, so this
	// takes the max of the two rather than picking one -- the largest
	// magnitude is "how loud", same "how loud, not which side" spirit as
	// the signed accessors on other chips. Updated by run_until();
	// reflects the level as of the last time the chip was run, not
	// necessarily "right now". Control-path peek only; do not call from
	// the audio render thread.
	int osc_last_amp( int i ) const
	{
		assert( (unsigned) i < osc_count );
		int a = oscs [i].last_amp [0];
		int b = oscs [i].last_amp [1];
		if ( a < 0 ) a = -a;
		if ( b < 0 ) b = -b;
		return a > b ? a : b;
	}
	// Task 9 meter: channel i's control register (bit 7 on, bit 6 direct-D/A mode).
	int osc_control( int i ) const { assert( (unsigned) i < osc_count ); return oscs [i].control; }
	// Visualiser pitch (JC 2026-11-17): the 12 bit period; Hz = clock / (64 p), clock = the PC Engine CPU clock.
	int osc_period( int i ) const { assert( (unsigned) i < osc_count ); return oscs [i].period; }
	// -- end voice-peek addition --

	void reset();

	static const unsigned int start_addr = 0x0800;
	static const unsigned int end_addr   = 0x0809;
	void write_data( blip_time_t, int addr, int data );

	void end_frame( blip_time_t );

public:
	Hes_Apu();
private:
	Hes_Osc oscs [osc_count];
	int latch;
	int balance;
	Hes_Osc::synth_t synth;

	void balance_changed( Hes_Osc& );
	void recalc_chans();
};

inline void Hes_Apu::volume( double v ) { synth.volume( 1.8 / osc_count / Hes_Osc::amp_range * v ); }

inline void Hes_Apu::treble_eq( blip_eq_t const& eq ) { synth.treble_eq( eq ); }

#endif
