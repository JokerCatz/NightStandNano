// Atari XL/XE SAP music file emulator

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef SAP_EMU_H
#define SAP_EMU_H

#include "Classic_Emu.h"
#include "Sap_Apu.h"
#include "Sap_Cpu.h"

class Sap_Emu : private Sap_Cpu, public Classic_Emu {
	typedef Sap_Cpu cpu;
public:
	static gme_type_t static_type() { return gme_sap_type; }
public:
	Sap_Emu();
	~Sap_Emu();

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only access to the internal Sap_Apu chip(s) for per-voice
	// peeking (Sap_Apu::osc_last_amp). Mirrors Nsf_Emu::apu_()'s existing
	// pointer-accessor convention (gme/Nsf_Emu.h); apu/apu2 were private
	// with no such accessor before this addition. apu2 only carries live
	// state for stereo SAPs (info.stereo, voice_count 8); mono SAPs
	// (voice_count 4, the common case) never touch it, so it is a valid
	// object either way, just unused/silent.
	Sap_Apu* apu_() { return &apu; }
	Sap_Apu* apu2_() { return &apu2; }
	// -- end voice-peek addition --

	struct info_t {
		byte const* rom_data;
		const char* warning;
		long init_addr;
		long play_addr;
		long music_addr;
		int  type;
		int  track_count;
		int  fastplay;
		bool stereo;
		char author    [256];
		char name      [256];
		char copyright [ 32];

		// -- SAP TIME tag addition (RP2350_player, tools/gme-patches/0003-sap-time.patch) --
		// Per-subtune duration parsed from the file's "TIME mm:ss[.fraction]
		// [LOOP]" header lines (SAP spec, one line per subtune, in file
		// order), in milliseconds; -1 means "no TIME line for this
		// subtune, or it didn't parse" -- falls back to gme's normal
		// length/intro_length/loop_length/150000ms default chain in
		// gme.cpp's gme_track_info(), exactly as before this patch. A
		// fixed-size array (rather than a dynamically-sized container)
		// keeps this a small, self-contained addition; any TIME line past
		// this many subtunes is parsed (so later lines stay aligned with
		// their real subtune index) but has nowhere left to be stored, so
		// its value is simply discarded. No file in this project's sample
		// corpus (assets/local/atari/*.sap) has more than 16 subtunes.
		enum { max_track_times = 64 };
		long track_times [max_track_times];
		// -- end SAP TIME tag addition --
	};
protected:
	blargg_err_t track_info_( track_info_t*, int track ) const;
	blargg_err_t load_mem_( byte const*, long );
	blargg_err_t start_track_( int );
	blargg_err_t run_clocks( blip_time_t&, int );
	void set_tempo_( double );
	void set_voice( int, Blip_Buffer*, Blip_Buffer*, Blip_Buffer* );
	void update_eq( blip_eq_t const& );
public: private: friend class Sap_Cpu;
	int cpu_read( sap_addr_t );
	void cpu_write( sap_addr_t, int );
	void cpu_write_( sap_addr_t, int );
private:
	info_t info;

	byte const* file_end;
	sap_time_t scanline_period;
	sap_time_t next_play;
	sap_time_t time_mask;
	Sap_Apu apu;
	Sap_Apu apu2;

	// large items
	struct {
		byte padding1 [0x100];
		byte ram [0x10000 + 0x100];
	} mem;
	Sap_Apu_Impl apu_impl;

	sap_time_t play_period() const;
	void call_play();
	void cpu_jsr( sap_addr_t );
	void call_init( int track );
	void run_routine( sap_addr_t );
};

#endif
