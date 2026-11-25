


void cRSID_generateSound (cRSID_C64instance* C64, FASTVAR unsigned char *buf, FASTVAR unsigned short len) {
 //enum { FADE_SCALING = 16, MIX_SCALING = (256*FADE_SCALING) };
 FASTVAR unsigned short i; FASTVAR unsigned char j;
 cRSID_Output Output = {0,0};
 //FASTVAR signed short OutputL, OutputR;
 //FASTVAR signed int VolumeMul;

 //Refresh shadow registers that remain untouched during the audio-generation, to avoid discrepancies if main thread changes their originators
 C64->RealSIDmode = C64->Interface.RealSIDmode; //this one gets updated at SID-data rocessing too when the audio doesn't run, to let CPU routines use the up-to-date shadowed versions
 C64->AudioThread_SIDchipCount = C64->SIDchipCount;
 C64->Stereo = C64->AudioThread_SIDchipCount > 1 ? C64->Interface.Stereo : CRSID_CHANNELMODE_MONO; //this might be changed regularly during playback by the user, and maybe these too:
 C64->HighQualitySID = C64->Interface.HighQualitySID; C64->HighQualityResampler = C64->Interface.HighQualityResampler;

 for (i=0; i<len; i+=4) {
  for(j=0; j<C64->Interface.PlaybackSpeed; ++j) Output = cRSID_generateSample(C64);
  //VolumeMul = C64->Interface.MainVolume * C64->FadeLevel;
  // /* *(signed short*) &buf[i]*/ Output.L = Output.L * VolumeMul / MIX_SCALING;   //can we expect little-endianness?
  // /* *(signed short*) &buf[i+2]*/ Output.R = Output.R * VolumeMul / MIX_SCALING; //can we expect little-endianness?
  buf[i+0] = Output.L & 0xFF; buf[i+1] = Output.L >> 8;
  buf[i+2] = Output.R & 0xFF; buf[i+3] = Output.R >> 8;
 }
}


INLINE cRSID_Output cRSID_generateSample (cRSID_C64instance* C64) { //call this from custom buffer-filler
 enum { FADE_SCALING = 16, MIX_SCALING = (256*FADE_SCALING),
  SCRAMBLER_BITS = 8, DITHERNOISE_DIVSHIFTS = 6 /*5*/, DITHERNOISE_MID = ( ( 1 << (SCRAMBLER_BITS - DITHERNOISE_DIVSHIFTS) ) / 2 )
 };

 FASTVAR signed short VolumeMul; FASTVAR signed short Insert;
 cRSID_Output Output;
 FASTVAR int fb;

 if ( RARELY (C64->Interface.Paused) ) { Output.L = Output.R = 0; return Output; }

 //If calling cRSID_generateSample(C64) directly, don't forget to keep the audio-thread shadow-variables updated, as seen above in cRSID_generateSound(C64)
 //C64->RealSIDmode = C64->Interface.RealSIDmode; //this one gets updated at SID-data rocessing too when the audio doesn't run, to let CPU routines use the up-to-date shadowed versions

 Output = *cRSID_emulateC64(C64);

 if ( RARELY (C64->Interface.PSIDdigiMode) ) { Insert = cRSID_playPSIDdigi(C64); Output.L += Insert; Output.R += Insert; }

 VolumeMul = C64->Interface.MainVolume * C64->FadeLevel;
 Output.L = (Output.L * VolumeMul) / MIX_SCALING; //Having the attenuator before saturation allows avoiding filter/many-SIDs overdrive-distortion
 Output.R = (Output.R * VolumeMul) / MIX_SCALING; //(By simply adjusting the main volume knob to a lower level. Real SID distorts as well in those cases btw.)

 //Add dithering noise for more natural sound and to override any small quantization/correlated/mixdown noises (might add C64 bus-noise in the future)
 //This is my fast C64->DitherLFSR+scrambler-routine generating perfect even white-noise spectrum (measured with 'ENT' tool)
 //23bit C64->DitherLFSR (pow(2,23)-1=8388607 states) with XOR-ed taps of bit22 and bit13 (realized by 24 bits = 3 bytes and bit23^bit14 (LFSR_HH[7],LFSR_H[6]) feedback to bit1)
 fb = ( (C64->DitherLFSR & 0x800000) ^ ( (C64->DitherLFSR & 0x004000) << 9 ) ); C64->DitherLFSR <<= 1; if (fb) C64->DitherLFSR |= 2; //C64->DitherLFSR&=0xFFFFFE;
 //scramble 24bit value into a 8bit output (e.g. make C64->DitherLFSR 8bit output non-linear and therefore more random)
 //We use AND-masks to combine into output (instead of free bit-to-bit mapping)
 //(this method has ENT-correlation=0.000002, PI-error=0.02%, a nice evenly distributed spectrum that fits between -45.4..-45.8dB)
 Insert = (signed char) ( (unsigned char) ( ( ( (C64->DitherLFSR >> 16) & 0x99 ) + ( (C64->DitherLFSR >> 8) & 0x72 ) + (C64->DitherLFSR & 0xB4) + 0x7C ) & 0xFF ) >> DITHERNOISE_DIVSHIFTS ) - DITHERNOISE_MID;
 Output.L += Insert; Output.R += Insert;

 if ( RARELY(Output.L>=32767) ) Output.L=32767; else if ( RARELY (Output.L<=-32768) ) Output.L=-32768; //saturation arithmetic on overflow
 if ( RARELY(Output.R>=32767) ) Output.R=32767; else if ( RARELY (Output.R<=-32768) ) Output.R=-32768; //saturation arithmetic on overflow
 return Output;
}

