// MSX computer KSS music file emulator

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef KSS_EMU_H
#define KSS_EMU_H

#include "Classic_Emu.h"
#include "Kss_Scc_Apu.h"
#include "Kss_Cpu.h"
#include "Sms_Apu.h"
#include "Ay_Apu.h"
#include "Ym2413_Emu.h"

class Kss_Emu : private Kss_Cpu, public Classic_Emu {
	typedef Kss_Cpu cpu;
public:
	// KSS file header
	enum { header_size = 0x10 };
	struct header_t
	{
		byte tag [4];
		byte load_addr [2];
		byte load_size [2];
		byte init_addr [2];
		byte play_addr [2];
		byte first_bank;
		byte bank_mode;
		byte extra_header;
		byte device_flags;
	};

	enum { ext_header_size = 0x10 };
	struct ext_header_t
	{
		byte data_size [4];
		byte unused [4];
		byte first_track [2];
		byte last_tack [2];
		byte psg_vol;
		byte scc_vol;
		byte msx_music_vol;
		byte msx_audio_vol;
	};

	struct composite_header_t : header_t, ext_header_t { };

	// Header for currently loaded file
	composite_header_t const& header() const { return header_; }

	static gme_type_t static_type() { return gme_kss_type; }

	// JC 0009 (CHIP2A): render MSX-MUSIC (YM2413 on ports $7C/$7D) when the file has it. JC 0010 (OPLL): on by default since emu2413
	// became cheap enough for the board; fm_off (zero-initialised: .bss, not a .data byte in internal RAM) so MSX-MUSIC is ON by default (owner 2026-11-18, board run 3: 33-37 % of core 1).
	static bool fm_off;
public:
	Kss_Emu();
	~Kss_Emu();

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only access to the internal Ay_Apu/Scc_Apu for per-voice
	// peeking (Ay_Apu::osc_last_amp / Scc_Apu::osc_last_amp). Mirrors
	// Nsf_Emu::apu_()'s existing pointer-accessor convention
	// (gme/Nsf_Emu.h); ay/scc were private with no such accessor before
	// this addition. KSS voices are always AY (0-2) + SCC (3-7) --
	// set_voice_count(osc_count) in Kss_Emu.cpp is a fixed
	// Ay_Apu::osc_count + Scc_Apu::osc_count = 8, regardless of whether
	// the optional SN76489 (`sn`, already gme-internal, not exposed here)
	// is also present for this file.
	Ay_Apu* ay_() { return &ay; }
	Scc_Apu* scc_() { return &scc; }
	// -- end voice-peek addition --

protected:
	blargg_err_t track_info_( track_info_t*, int track ) const;
	blargg_err_t load_( Data_Reader& );
	blargg_err_t start_track_( int );
	blargg_err_t run_clocks( blip_time_t&, int );
	blargg_err_t play_( long count, sample_t out [] ) override; // JC 0009: adds the MSX-MUSIC (YM2413) samples
	void set_tempo_( double );
	void set_voice( int, Blip_Buffer*, Blip_Buffer*, Blip_Buffer* );
	void update_eq( blip_eq_t const& );
	void unload();
private:
	Rom_Data<page_size> rom;
	composite_header_t header_;

	bool scc_accessed;
	bool gain_updated;
	void update_gain();

	unsigned scc_enabled; // 0 or 0xC000
	int bank_count;
	void set_bank( int logical, int physical );
	int32_t bank_size() const { return (16 * 1024L) >> (header_.bank_mode >> 7 & 1); }

	blip_time_t play_period;
	blip_time_t next_play;
	int ay_latch;

	friend void kss_cpu_out( class Kss_Cpu*, cpu_time_t, unsigned addr, int data );
	friend int  kss_cpu_in( class Kss_Cpu*, cpu_time_t, unsigned addr );
	void cpu_write( unsigned addr, int data );
	friend void kss_cpu_write( class Kss_Cpu*, unsigned addr, int data );

	// large items
	static const unsigned int mem_size = 0x10000;
	byte ram [mem_size + cpu_padding];

	// JC 0009 (CHIP2A): MSX-MUSIC. Ports $7C / $7D feed an emu2413 (Ym2413_Emu); its samples are generated per emulated frame, at the sample
	// positions of the port writes, into a small ring that play_ mixes into the output (half amplitude). Not a voice: no meter, no mute.
	Ym2413_Emu* fm;                    // NULL unless the header's device flags select FM (bit 0)
	enum { fm_ring_size = 4096, fm_ev_max = 256 };
	short* fm_ring;                    // fm_ring_size stereo frames
	unsigned fm_rd, fm_wr;             // ring positions in frames (wrapping)
	int64_t fm_clk;                    // CPU clocks emulated since the track started
	int64_t fm_done;                   // FM frames generated since then
	struct fm_ev_t { int32_t t; byte a, d; } fm_evs [fm_ev_max];   // port writes of the frame being emulated (time = CPU clocks into it)
	int fm_nev;
	byte fm_latch;
	void fm_gen( long frames );
	void fm_render( blip_time_t duration );

	Ay_Apu ay;
	Scc_Apu scc;
	Sms_Apu* sn;
	byte unmapped_read  [0x100];
	byte unmapped_write [page_size + 1];
};

#endif
