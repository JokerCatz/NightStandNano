// YM2413 FM sound chip emulator interface

// Game_Music_Emu https://bitbucket.org/mpyne/game-music-emu/
#ifndef YM2413_EMU_H
#define YM2413_EMU_H

class Ym2413_Emu  {
	struct __OPLL* opll; // JC 0007: emu2413 names its struct __OPLL
	int quiet;           // JC 0011: consecutive calculated samples that were zero with every carrier muted (see run)
public:
	Ym2413_Emu();
	~Ym2413_Emu();

	// Set output sample rate and chip clock rates, in Hz. Returns non-zero
	// if error.
	int set_rate( double sample_rate, double clock_rate );

	// Reset to power-up state
	void reset();

	// Mute voice n if bit n (1 << n) of mask is set
	enum { channel_count = 14 };
	void mute_voices( int mask );

	// Write 'data' to 'addr'
	void write( int addr, int data );

	// Run and write pair_count samples to output
	typedef short sample_t;
	enum { out_chan_count = 2 }; // stereo
	void run( int pair_count, sample_t* out );

	// The chip (host tests compare its state with a chip calculated sample by sample)
	const struct __OPLL* chip() const { return opll; }
};

#endif
