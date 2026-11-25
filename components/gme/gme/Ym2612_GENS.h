// YM2612 FM sound chip emulator interface

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef YM2612_EMU_H
#define YM2612_EMU_H

struct Ym2612_GENS_Impl;

class Ym2612_GENS_Emu  {
	Ym2612_GENS_Impl* impl;
public:
	Ym2612_GENS_Emu() { impl = 0; }
	~Ym2612_GENS_Emu();

	// Set output sample rate and chip clock rates, in Hz. Returns non-zero
	// if error.
	const char* set_rate( double sample_rate, double clock_rate );

	// Reset to power-up state
	void reset();

	// Mute voice n if bit n (1 << n) of mask is set
	enum { channel_count = 6 };
	void mute_voices( int mask );

	// Write addr to register 0 then data to register 1
	void write0( int addr, int data );

	// Write addr to register 2 then data to register 3
	void write1( int addr, int data );

	// Run and add pair_count samples into current output buffer contents
	typedef short sample_t;
	enum { out_chan_count = 2 }; // stereo
	void run( int pair_count, sample_t* out );

	// JC instrument: voice peek, channel n's OP4 (always a carrier) level 0..255 over its top 24 dB
	int level( int n ) const;

	// JC instrument: render from copies of the sine (8 KB) and envelope (16 KB) tables; NULL, NULL: back to the
	// core's own. Sizes: sin_table_size / env_table_size. Process wide (the tables do not depend on the rates).
	enum { sin_table_size = 4096 * 2, env_table_size = (2 * 4096 + 8) * 2 };
	void fast_tables( void* sin_mem, void* env_mem );
};

#endif
