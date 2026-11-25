// Sega Master System/Mark III, Sega Genesis/Mega Drive, BBC Micro VGM music file emulator

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef VGM_EMU_H
#define VGM_EMU_H

#include "Vgm_Emu_Impl.h"

// Emulates VGM music using SN76489/SN76496 PSG, YM2612, and YM2413 FM sound chips.
// Supports custom sound buffer and frequency equalization when VGM uses just the PSG.
// FM sound chips can be run at their proper rates, or slightly higher to reduce
// aliasing on high notes. Currently YM2413 support requires that you supply a
// YM2413 sound chip emulator. I can provide one I've modified to work with the library.
class Vgm_Emu : public Vgm_Emu_Impl {
public:
	// True if custom buffer and custom equalization are supported
	// TODO: move into Music_Emu and rename to something like supports_custom_buffer()
	bool is_classic_emu() const { return !uses_fm; }

	blargg_err_t set_multi_channel ( bool is_enabled ) override;

	// Disable running FM chips at higher than normal rate. Will result in slightly
	// more aliasing of high notes.
	void disable_oversampling( bool disable = true ) { disable_oversampling_ = disable; }

	// VGM header format
	enum { header_size = 0x40 };
	struct header_t
	{
		char tag [4];
		byte data_size [4];
		byte version [4];
		byte psg_rate [4];
		byte ym2413_rate [4];
		byte gd3_offset [4];
		byte track_duration [4];
		byte loop_offset [4];
		byte loop_duration [4];
		byte frame_rate [4];
		byte noise_feedback [2];
		byte noise_width;
		byte unused1;
		byte ym2612_rate [4];
		byte ym2151_rate [4];
		byte data_offset [4];
		byte unused2 [8];
	};

	// Header for currently loaded file
	header_t const& header() const { return *(header_t const*) data; }

	static gme_type_t static_type() { return gme_vgm_type; }

public:
	// deprecated
	using Music_Emu::load;
	blargg_err_t load( header_t const& h, Data_Reader& in ) // use Remaining_Reader
			{ return load_remaining_( &h, sizeof h, in ); }
	byte const* gd3_data( int* size_out = 0 ) const; // use track_info()

public:
	Vgm_Emu();
	~Vgm_Emu();

	// -- voice-peek additions (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only access to the internal PSG/FM chips for per-voice peeking.
	// `psg`/`ym2612`/`ym2413`/`psg_dual`/`dac_amp` are protected members of
	// the Vgm_Emu_Impl base (gme/Vgm_Emu_Impl.h), reachable from Vgm_Emu's
	// own member functions without any base-class header change -- these
	// are just the first accessors that do so. Mirrors Nsf_Emu::apu_()'s
	// existing pointer-accessor naming convention (gme/Nsf_Emu.h).
	//
	// psg_(i): PSG chip i (0 or 1; 1 only meaningful when psg_dual_() is
	// true, e.g. Sega Mark III/SMS export with a second discrete PSG).
	// Bounds-asserted like Nes_Apu::osc_last_amp.
	Sms_Apu* psg_( int i ) { assert( (unsigned) i < 2 ); return &psg [i]; }
	bool psg_dual_() const { return psg_dual; }
	// ym2612_(i)/ym2413_(i): the FM chip wrapper (i likewise 0 or 1 for a
	// dual-chip file); Ym_Emu<Emu> publicly derives Emu, so the pointer
	// converts to the plain chip type with no slicing. *_enabled_() lets
	// the voice-state adapter tell which FM chip (if either) is actually
	// driving this track before peeking it -- is_classic_emu() above only
	// says "is FM used at all", not which chip.
	Ym2612_Emu* ym2612_( int i ) { assert( (unsigned) i < 2 ); return &ym2612 [i]; }
	bool ym2612_enabled_() const { return ym2612 [0].enabled(); }
	Ym2413_Emu* ym2413_( int i ) { assert( (unsigned) i < 2 ); return &ym2413 [i]; }
	bool ym2413_enabled_() const { return ym2413 [0].enabled(); }
	// dac_amp_(): the VGM PCM/DAC stream's last-written sample (see
	// write_pcm() in Vgm_Emu_Impl.cpp), 0..255, or a negative sentinel
	// (dac_disabled ORed in) when the DAC stream isn't active. This is
	// gme_voice_count's 7th voice ("PCM") for an FM-mode VGM.
	int dac_amp_() const { return dac_amp; }
	// -- end voice-peek additions --

protected:
	blargg_err_t track_info_( track_info_t*, int track ) const override;
	blargg_err_t load_mem_( byte const*, long ) override;
	blargg_err_t set_sample_rate_( long sample_rate ) override;
	blargg_err_t start_track_( int ) override;
	blargg_err_t play_( long count, sample_t* ) override;
	blargg_err_t run_clocks( blip_time_t&, int ) override;
	void set_tempo_( double ) override;
	void mute_voices_( int mask ) override;
	void set_voice( int, Blip_Buffer*, Blip_Buffer*, Blip_Buffer* ) override;
	void update_eq( blip_eq_t const& ) override;
private:
	// removed; use disable_oversampling() and set_tempo() instead
	Vgm_Emu( bool oversample, double tempo = 1.0 );
	double fm_rate;
	uint32_t psg_rate;
	uint32_t vgm_rate;
	bool disable_oversampling_;
	bool uses_fm;
	blargg_err_t setup_fm();
};

#endif
