// Konami VRC6 sound chip emulator

// Nes_Snd_Emu 0.1.8
#ifndef NES_VRC6_APU_H
#define NES_VRC6_APU_H

#include "blargg_common.h"
#include "Blip_Buffer.h"

struct vrc6_apu_state_t;

class Nes_Vrc6_Apu {
public:
	// See Nes_Apu.h for reference
	void reset();
	void volume( double );
	void treble_eq( blip_eq_t const& );
	void output( Blip_Buffer* );
	enum { osc_count = 3 };
	void osc_output( int index, Blip_Buffer* );
	void end_frame( blip_time_t );
	void save_state( vrc6_apu_state_t* ) const;
	void load_state( vrc6_apu_state_t const& );

	// Oscillator 0 write-only registers are at $9000-$9002
	// Oscillator 1 write-only registers are at $A000-$A002
	// Oscillator 2 write-only registers are at $B000-$B002
	enum { reg_count = 3 };
	enum { base_addr = 0x9000 };
	enum { addr_step = 0x1000 };
	void write_osc( blip_time_t, int osc, int reg, int data );

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only peek at oscillator i's last computed amplitude
	// (Vrc6_Osc::last_amp), mirroring Nes_Apu::osc_last_amp/
	// Gb_Apu::osc_last_amp above -- same bounds-assert convention, same
	// "control-path peek only" contract. Oscillator 0/1 are square waves
	// (run_square(): `volume = regs[0] & 15`, last_amp unipolar 0-15,
	// same shape as Nes_Apu's squares); oscillator 2 is the saw wave
	// (run_saw(): last_amp is `amp >> 3` where amp is an 8-bit
	// accumulator, unipolar 0-31 -- see Nes_Vrc6_Apu.cpp). Task 39 (NSF
	// expansion-chip Channels page adapters).
	int osc_last_amp( int i ) const { assert( (unsigned) i < osc_count ); return oscs [i].last_amp; }
	// Visualiser pitch (JC 2026-11-17): the period incl. the +1 (osc 0/1 squares, 2 saw); Hz = clock / (16 period) squares, clock / (14 period) saw.
	int osc_period( int i ) const { assert( (unsigned) i < osc_count ); return oscs [i].period(); }
	// -- end voice-peek addition --

public:
	Nes_Vrc6_Apu();
	BLARGG_DISABLE_NOTHROW
private:
	// noncopyable
	Nes_Vrc6_Apu( const Nes_Vrc6_Apu& );
	Nes_Vrc6_Apu& operator = ( const Nes_Vrc6_Apu& );

	struct Vrc6_Osc
	{
		uint8_t regs [3];
		Blip_Buffer* output;
		int delay;
		int last_amp;
		int phase;
		int amp; // only used by saw

		int period() const
		{
			return (regs [2] & 0x0F) * 0x100L + regs [1] + 1;
		}
	};

	Vrc6_Osc oscs [osc_count];
	blip_time_t last_time;

	Blip_Synth<blip_med_quality,1> saw_synth;
	Blip_Synth<blip_good_quality,1> square_synth;

	void run_until( blip_time_t );
	void run_square( Vrc6_Osc& osc, blip_time_t );
	void run_saw( blip_time_t );
};

struct vrc6_apu_state_t
{
	uint8_t regs [3] [3];
	uint8_t saw_amp;
	uint16_t delays [3];
	uint8_t phases [3];
	uint8_t unused;
};

inline void Nes_Vrc6_Apu::osc_output( int i, Blip_Buffer* buf )
{
	assert( (unsigned) i < osc_count );
	oscs [i].output = buf;
}

inline void Nes_Vrc6_Apu::volume( double v )
{
	double const factor = 0.0967 * 2;
	saw_synth.volume( factor / 31 * v );
	square_synth.volume( factor * 0.5 / 15 * v );
}

inline void Nes_Vrc6_Apu::treble_eq( blip_eq_t const& eq )
{
	saw_synth.treble_eq( eq );
	square_synth.treble_eq( eq );
}

#endif
