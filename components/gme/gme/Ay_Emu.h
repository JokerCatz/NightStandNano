// Sinclair Spectrum AY music file emulator

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef AY_EMU_H
#define AY_EMU_H

#include "Classic_Emu.h"
#include "Ay_Apu.h"
#include "Ay_Cpu.h"

class Ay_Emu : private Ay_Cpu, public Classic_Emu {
	typedef Ay_Cpu cpu;
public:
	// AY file header
	enum { header_size = 0x14 };
	struct header_t
	{
		byte tag [8];
		byte vers;
		byte player;
		byte unused [2];
		byte author [2];
		byte comment [2];
		byte max_track;
		byte first_track;
		byte track_info [2];
	};

	static gme_type_t static_type() { return gme_ay_type; }
public:
	Ay_Emu();
	~Ay_Emu();

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only access to the internal Ay_Apu for per-voice peeking
	// (Ay_Apu::osc_last_amp). Mirrors Nsf_Emu::apu_()'s existing
	// pointer-accessor convention (gme/Nsf_Emu.h); apu itself was private
	// with no such accessor before this addition.
	Ay_Apu* apu_() { return &apu; }

	// Read-only peek at the ZX Spectrum 48K beeper's last output
	// level. The beeper (gme's 4th AY voice, name "Beeper" -- see
	// Ay_Emu.cpp's voice_names[]) is NOT part of the AY-3-8910 chip: it's
	// a single I/O port bit (port 0xFE, bit 4) toggled directly by Z80
	// code, handled entirely in ay_cpu_out()/Ay_Emu.cpp, never touching
	// `apu`. `last_beeper` holds that bit's last-written value, masked to
	// `data & 0x10` (see ay_cpu_out: `data &= 0x10; if (last_beeper !=
	// data) { last_beeper = data; ... }`) -- always exactly 0 or 0x10, a
	// binary click generator with no volume register, not an oscillator
	// with a graded amplitude. This is the only "how loud right now"
	// signal this voice has. Updated on every port write; reflects the
	// state as of the last write, not necessarily "right now". Unipolar
	// by construction (0 or 0x10). Control-path peek only; do not call
	// from the audio render thread.
	int beeper_level() const { return last_beeper; }
	// Task 9 meter: how many times the beeper bit has changed (wraps); a changing count is sound, a held bit is not.
	unsigned beeper_toggles() const { return beeper_toggle_count; }
	// -- end voice-peek addition --

	struct file_t {
		header_t const* header;
		byte const* end;
		byte const* tracks;
	};
protected:
	blargg_err_t track_info_( track_info_t*, int track ) const;
	blargg_err_t load_mem_( byte const*, long );
	blargg_err_t start_track_( int );
	blargg_err_t run_clocks( blip_time_t&, int );
	void set_tempo_( double );
	void set_voice( int, Blip_Buffer*, Blip_Buffer*, Blip_Buffer* );
	void update_eq( blip_eq_t const& );
private:
	file_t file;

	cpu_time_t play_period;
	cpu_time_t next_play;
	Blip_Buffer* beeper_output;
	int beeper_delta;
	int last_beeper;
	unsigned beeper_toggle_count;
	int apu_addr;
	int cpc_latch;
	bool spectrum_mode;
	bool cpc_mode;

	// large items
	struct {
		byte padding1 [0x100];
		byte ram [0x10000 + 0x100];
	} mem;
	Ay_Apu apu;
	friend void ay_cpu_out( Ay_Cpu*, cpu_time_t, unsigned addr, int data );
	void cpu_out_misc( cpu_time_t, unsigned addr, int data );
};

#endif
