// YM2612 FM sound chip emulator interface

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef YM2612_EMU_H
#define YM2612_EMU_H

typedef void Ym2612_Nuked_Impl;

class Ym2612_Nuked_Emu  {
	Ym2612_Nuked_Impl* impl;
	double prev_sample_rate;
	double prev_clock_rate;
public:
	Ym2612_Nuked_Emu();
	~Ym2612_Nuked_Emu();

	// Set output sample rate and chip clock rates, in Hz. Returns non-zero
	// if error.
	const char* set_rate( double sample_rate, double clock_rate );

	// Reset to power-up state
	void reset();

	// Mute voice n if bit n (1 << n) of mask is set
	enum { channel_count = 6 };
	void mute_voices( int mask );

	// -- voice-peek addition (RP2350_player, tools/gme-patches/0001-voice-peek.patch) --
	// Read-only peek at FM channel i's current accumulated output sample
	// (Nuked-OPN2's internal ym3438_t::ch_out[i] -- see OPN2_ChGenerate in
	// Ym2612_Nuked.cpp: `chip->ch_out[channel] = chip->ch_acc[channel]`,
	// indexed by the register-programmed logical channel number 0-5, the
	// same numbering `write0`/`write1`'s channel-select register field
	// uses -- not the chip's internal visit order, which is scrambled for
	// pipelining but always writes back to this same per-channel slot).
	// This is the per-channel FM accumulator right before the final
	// DAC/pan mixdown (OPN2_ChOutput) -- the closest thing to "how loud is
	// this FM channel right now" reachable without rewriting the core.
	// `ch_acc`/`sum` are clamped to -256..255 in OPN2_ChGenerate just
	// before being latched into ch_out, so this is bipolar in that range;
	// sign is waveform phase, not loudness (same convention as the other
	// signed accessors in this patch -- take abs()). No other gme YM2612
	// backend (GENS/MAME) is patched: this project's vendored build only
	// compiles the Nuked core (no VGM_YM2612_GENS/VGM_YM2612_MAME define
	// anywhere in its CMake), so only this one needed a peek. Implemented
	// in Ym2612_Nuked.cpp (not inline here) because `impl` is an opaque
	// pointer at this header's scope -- ym3438_t is only a complete type
	// inside the .cpp, same reason every method above is defined there
	// instead of inline. Control-path peek only; do not call from the
	// audio render thread.
	int channel_out( int i ) const;
	// -- end voice-peek addition --

	// Write addr to register 0 then data to register 1
	void write0( int addr, int data );

	// Write addr to register 2 then data to register 3
	void write1( int addr, int data );

	// Run and add pair_count samples into current output buffer contents
	typedef short sample_t;
	enum { out_chan_count = 2 }; // stereo
	void run( int pair_count, sample_t* out );
};

#endif

