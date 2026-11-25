


static INLINE cRSID_Output* cRSID_emulateLightSIDs (cRSID_C64instance* C64) { //lightweight (all at samplerate-pace) SID-emulations: oscillators, waveforms, filter and complete output stages
 //static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp, Tmp2;
 cRSID_Output Output; //ChipChill: returned through C64->MixedOutput (it was a static)

 switch (C64->Stereo) {
  case CRSID_CHANNELMODE_MONO: { //if ( MOSTLY (C64->Stereo==CRSID_CHANNELMODE_MONO /*|| C64->AudioThread_SIDchipCount==1*/) ) { //mono
   Output.L /*= Output.R*/ = cRSID_emulateSID_light(C64,  &C64->SID[1] );
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light(C64,  &C64->SID[2] );
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light(C64,  &C64->SID[3] );
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) Output.L += cRSID_emulateSID_light(C64,  &C64->SID[4] );
   Output.R = Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount; //better being multiplied in a single place than for all SIDs
  } break;
  case CRSID_CHANNELMODE_STEREO: { //else { //stereo
   Tmp = cRSID_emulateSID_light(C64,  &C64->SID[1] );
   if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_LEFT) )  { Output.L = Tmp * 2; Output.R=0; }
   else if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Output.R = Tmp * 2; Output.L=0; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[2] );
    if (C64->SID[2].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (C64->SID[2].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[3] );
    if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_LEFT) )  Output.L += Tmp * 2;
    else if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[4] );
    if (C64->SID[4].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (C64->SID[4].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
   Output.R /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
  } break;
  case CRSID_CHANNELMODE_NARROW: {
   Tmp = cRSID_emulateSID_light(C64,  &C64->SID[1] );
   if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_LEFT) )  //Output.L = Tmp * 2;
   { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L = Tmp + Tmp2; Output.R = Tmp - Tmp2; }
   else if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_RIGHT) )  //Output.R = Tmp * 2;
   { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R = Tmp + Tmp2; Output.L = Tmp - Tmp2; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[2] );
    if (C64->SID[2].Channel == CRSID_CHANNEL_LEFT)  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if (C64->SID[2].Channel == CRSID_CHANNEL_RIGHT)  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[3] );
    if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_LEFT) )  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_RIGHT) )  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSID_light(C64,  &C64->SID[4] );
    if (C64->SID[4].Channel == CRSID_CHANNEL_LEFT)  //Output.L += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2; }
    else if (C64->SID[4].Channel == CRSID_CHANNEL_RIGHT)  //Output.R += Tmp * 2;
    { Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2; }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
   Output.R /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
  } break;
 }
 C64->MixedOutput = Output;
 return &C64->MixedOutput;
}



static INLINE void cRSID_emulateOversampledSIDwaves (cRSID_C64instance* C64) { //averaging oversampler, only oscillators & waveforms (called at samplerate-pace)
 FASTVAR unsigned char HQsampleCount = 0;  FASTVAR int Temp;
 //2-pole Chebyshev filter: coefficients for 0.075f (cutoff is 18.4734kHz at 5.5x oversampling):
 //a0=3.869430E-02, a1=7.738860E-02, a2=3.869430E-02,  //fixedpoint (1.0=16384): a0 = 634, a1 = 1268,  a2 = 634
 //                 b1=1.392667E+00, b2=-5.474446E-01; //fixedpoint (1.0=16384): (b0 = 0), b1 = 22817, b2 = -8969
 //sample 'n': output[n] = input[n]*a0 + (input[n-1]*a1 + output[n-1]*b1) + (input[n-2]*a2 + output[n-2]*b2)
 //'a=a0+a1+a2=2536' -> simplified (faster) formula: output = (input*a + output*b1 + prevoutput*b2) / 16384
 //but b1 and b2 is compensated to signed-sum to 16384 and to be (sums of) powers of 2:
 // a = 2048 , b1 = 22528 = 16384 + 4096 + 2048 = 16384 + 2048 * 3 , b2 = -8192
 // -> output = (input*2048 + output*(16384+3*2048) - prevoutput * 8192) >> 14
 // -> output += ( (input << 11) + ((output*3) << 11) - (prevoutput << 13) ) >> 14
 //*** Resulting even more simplified formula: output += ( input + output * 3 - (prevoutput << 2) ) >> 3;
 //2-pole Chybyshev coefficients for 0.1f (cutoff is 10th of sample-freq. 24.6312kHz at 5.5x oversampling):
 //a0=6.372802E-02, a1=1.274560E-01, a2=6.372802E-02  //fixedpoint (1.0=16384): a0 = 1044, a1 = 2088,  a2 = 1044
 //                 b1=1.194365E+00, b2=-4.492774E-01 //fixedpoint (1.0=16384): (b0 = 0),  b1 = 19568, b2 = -7361
 //'a=a0+a1+a2=4176', b1 and b2 is compensated to signed-sum to 16384 and to be (sums of) powers of 2:
 // a = 4096 , b1 = 20480 = 16384 + 4096, b2 = -8192
 //*** Resulting very simplified formula: output += ( input + output - (prevoutput << 1) ) >> 2;
 //4-pole Chebyshev 0.1f: is much more complex and needs much more precision, not worth implementing for RPI0
 //2.780755E-03, 1.112302E-02, 1.668453E-02,  1.112302E-02, 2.780755E-03
  //fixedpoint (1.0=16384): 46,  184,    273,   184,    46   -> sum = 733 -> 1024
 //    0,        2.764031E+00, -3.122854E+00, 1.664554E+00, -3.502232E-01
  //fixedpoint (1.0=16384): 0,  45286, -51165, 27272, -5738  -> 0, 49152, -53278, 24576, -6144   for example
 //(ChipChill: the filter states are in the instance now, C64->NonFiltSample1 etc.)
 cRSID_SIDwavOutput SIDwavOutput;

 HQsampleCount=0;
 C64->SID[1].NonFiltedSample = C64->SID[1].FilterInputSample = 0;
 C64->SID[2].NonFiltedSample = C64->SID[2].FilterInputSample = 0;
 C64->SID[3].NonFiltedSample = C64->SID[3].FilterInputSample = 0;
 C64->SID[4].NonFiltedSample = C64->SID[4].FilterInputSample = 0;

 while (C64->OverSampleCycleCnt <= C64->SampleClockRatio) {

  SIDwavOutput = cRSID_emulateHQwaves(C64,  &C64->SID[1], CRSID_OVERSAMPLING_CYCLES );
  Temp = C64->PrevNonFiltSample1; C64->PrevNonFiltSample1 = C64->NonFiltSample1;
  C64->NonFiltSample1 += ( SIDwavOutput.NonFilted + C64->NonFiltSample1 * 3 - (Temp << 2) ) >> 3; //( SIDwavOutput.NonFilted + C64->NonFiltSample1 - (Temp << 1) ) >> 2; //(SIDwavOutput.NonFilted - C64->NonFiltSample1) >> 2;
  Temp = C64->PrevFiltSample1; C64->PrevFiltSample1 = C64->FiltSample1;
  C64->FiltSample1 += ( SIDwavOutput.FilterInput + C64->FiltSample1 * 3 - (Temp << 2) ) >> 3; //( SIDwavOutput.FilterInput + C64->FiltSample1 - (Temp << 1) ) >> 2; //(SIDwavOutput.FilterInput - C64->FiltSample1) >> 2;
  C64->SID[1].NonFiltedSample += C64->NonFiltSample1; C64->SID[1].FilterInputSample += C64->FiltSample1;

  //2-pole Chebyshev-based fast integer-only fixed-point ~18kHz Nyquist/antialiasing-filters for all SIDs
  if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves(C64,  &C64->SID[2], CRSID_OVERSAMPLING_CYCLES );
   Temp = C64->PrevNonFiltSample2; C64->PrevNonFiltSample2 = C64->NonFiltSample2;
   C64->NonFiltSample2 += ( SIDwavOutput.NonFilted + C64->NonFiltSample2 * 3 - (Temp << 2) ) >> 3;
   Temp = C64->PrevFiltSample2; C64->PrevFiltSample2 = C64->FiltSample2;
   C64->FiltSample2 += ( SIDwavOutput.FilterInput + C64->FiltSample2 * 3 - (Temp << 2) ) >> 3;
   C64->SID[2].NonFiltedSample += C64->NonFiltSample2; C64->SID[2].FilterInputSample += C64->FiltSample2; //SIDwavOutput.FilterInput;
  }
  if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves(C64,  &C64->SID[3], CRSID_OVERSAMPLING_CYCLES );
   Temp = C64->PrevNonFiltSample3; C64->PrevNonFiltSample3 = C64->NonFiltSample3;
   C64->NonFiltSample3 += ( SIDwavOutput.NonFilted + C64->NonFiltSample3 * 3 - (Temp << 2) ) >> 3;
   Temp = C64->PrevFiltSample3; C64->PrevFiltSample3 = C64->FiltSample3;
   C64->FiltSample3 += ( SIDwavOutput.FilterInput + C64->FiltSample3 * 3 - (Temp << 2) ) >> 3;
   C64->SID[3].NonFiltedSample += C64->NonFiltSample3; C64->SID[3].FilterInputSample += C64->FiltSample3; //SIDwavOutput.FilterInput;
  }
  if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
   SIDwavOutput = cRSID_emulateHQwaves(C64,  &C64->SID[4], CRSID_OVERSAMPLING_CYCLES );
   Temp = C64->PrevNonFiltSample4; C64->PrevNonFiltSample4 = C64->NonFiltSample4;
   C64->NonFiltSample4 += ( SIDwavOutput.NonFilted + C64->NonFiltSample4 * 3 - (Temp << 2) ) >> 3;
   Temp = C64->PrevFiltSample4; C64->PrevFiltSample4 = C64->FiltSample4;
   C64->FiltSample4 += ( SIDwavOutput.FilterInput + C64->FiltSample4 * 3 - (Temp << 2) ) >> 3;
   C64->SID[4].NonFiltedSample += C64->NonFiltSample4; C64->SID[4].FilterInputSample += C64->FiltSample3; //SIDwavOutput.FilterInput;
  }
  ++HQsampleCount;
  C64->OverSampleCycleCnt += (CRSID_OVERSAMPLING_CYCLES << CRSID_CLOCK_FRACTIONAL_BITS);
 }
 C64->OverSampleCycleCnt -= C64->SampleClockRatio;

 //fast resampler - averaging of accumulated samples, decreases sound-aliasing further:
 C64->SID[1].NonFiltedSample /= HQsampleCount; C64->SID[1].FilterInputSample /= HQsampleCount;
 if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
  C64->SID[2].NonFiltedSample /= HQsampleCount; C64->SID[2].FilterInputSample /= HQsampleCount;
 }
 if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
  C64->SID[3].NonFiltedSample /= HQsampleCount; C64->SID[3].FilterInputSample /= HQsampleCount;
 }
 if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
  C64->SID[4].NonFiltedSample /= HQsampleCount; C64->SID[4].FilterInputSample /= HQsampleCount;
 }
 if ( RARELY (C64->VoiceTap) ) { //ChipChill (patch 0004): per-voice tap, averaged to the output rate as the mix is
  int SIDnumber, Voice;
  for (SIDnumber = 1; SIDnumber <= CRSID_SIDCOUNT_MAX; ++SIDnumber) for (Voice = 0; Voice < 3; ++Voice) {
   C64->SID[SIDnumber].TapOut[Voice] = C64->SID[SIDnumber].TapSum[Voice] / HQsampleCount;
   C64->SID[SIDnumber].TapSum[Voice] = 0;
  }
 }
}



static INLINE cRSID_Output* cRSID_emulateOversampledSIDoutputs (cRSID_C64instance* C64) { //called at samplerate-pace, filters and complete output stages
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp, Tmp2;
 cRSID_Output Output; //ChipChill: returned through C64->MixedOutput (it was a static)

 switch (C64->Stereo) {  //if ( MOSTLY (C64->Stereo==CRSID_CHANNELMODE_MONO /*|| C64->AudioThread_SIDchipCount==1*/) ) { //mono
  case CRSID_CHANNELMODE_MONO: {
   Output.L /*= Output.R*/ = cRSID_emulateSIDoutputStage(C64,  &C64->SID[1] ); //, C64->HighQualityResampler );
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage(C64,  &C64->SID[2] ); //, C64->HighQualityResampler );
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage(C64,  &C64->SID[3] ); //, C64->HighQualityResampler );
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) Output.L += cRSID_emulateSIDoutputStage(C64,  &C64->SID[4] ); //, C64->HighQualityResampler );
   Output.R = Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount; //better being multiplied in a single place than for all SIDs
  } break;
  case CRSID_CHANNELMODE_STEREO: {  //else { //stereo
   Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[1] ); //, C64->HighQualityResampler );
   if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_LEFT) ) { Output.L = Tmp * 2; Output.R=0; }
   else if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Output.R = Tmp * 2; Output.L=0; }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[2] ); //, C64->HighQualityResampler );
    if (C64->SID[2].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (C64->SID[2].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[3] ); //, C64->HighQualityResampler );
    if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_LEFT) ) Output.L += Tmp * 2;
    else if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[4] ); //, C64->HighQualityResampler );
    if (C64->SID[4].Channel == CRSID_CHANNEL_LEFT)  Output.L += Tmp * 2;
    else if (C64->SID[4].Channel == CRSID_CHANNEL_RIGHT) Output.R += Tmp * 2;
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
   Output.R /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
  } break;
  case CRSID_CHANNELMODE_NARROW: {  //narrowed stereo (channels are closer to each other and the center)
   Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[1] ); //, C64->HighQualityResampler );
   if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_LEFT) ) {
    Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L = Tmp + Tmp2; Output.R = Tmp - Tmp2;
   }
   else if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_RIGHT) ) {
    Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R = Tmp + Tmp2; Output.L = Tmp - Tmp2;
   }
   else Output.L = Output.R = Tmp;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[2] ); //, C64->HighQualityResampler );
    if (C64->SID[2].Channel == CRSID_CHANNEL_LEFT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if (C64->SID[2].Channel == CRSID_CHANNEL_RIGHT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[3] ); //, C64->HighQualityResampler );
    if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_LEFT) ) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_RIGHT) ) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateSIDoutputStage(C64,  &C64->SID[4] ); //, C64->HighQualityResampler );
    if (C64->SID[4].Channel == CRSID_CHANNEL_LEFT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.L += Tmp + Tmp2; Output.R += Tmp - Tmp2;
    }
    else if (C64->SID[4].Channel == CRSID_CHANNEL_RIGHT) {
     Tmp2 = Tmp >> CRSID_CHANNELPANNING_DIVSHIFTS;  Output.R += Tmp + Tmp2; Output.L += Tmp - Tmp2;
    }
    else { Output.L += Tmp; Output.R += Tmp; }
   }
   Output.L /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
   Output.R /= C64->Attenuation; //( (CHANNELS*VOLUME_MAX) + C64->Attenuation );  // * C64->AudioThread_SIDchipCount;
  } break;
 }
 C64->MixedOutput = Output;
 return &C64->MixedOutput;
}



static INLINE void cRSID_emulateHQresampledSIDs (cRSID_C64instance* C64) { //oscillators, waveforms, filter and attenuation (main-volume) (called at samplerate-pace, but core at oversampled rate)
 static enum {
  //RESAMPLER_FRACTIONAL_BITS = 12,
  FRACTIONAL_MUL = (1 << CRSID_RESAMPLER_FRACTIONAL_BITS), //4096 $1000
  FRACTIONAL_AND = (FRACTIONAL_MUL-1), //4095 $0FFF
  INTEGER_AND = (-FRACTIONAL_MUL), //$F000
  RESAMPLEBUFFER_SIZE = 16, //entries
   RESAMPLEBUFFER_SIZE_MUL = (RESAMPLEBUFFER_SIZE << CRSID_RESAMPLER_FRACTIONAL_BITS),
  SINCWINDOW_PERIODS = (CRSID_RESAMPLER_SINCWINDOW_PERIODS), //12, //half-sines
   SINCWINDOW_HALF1_LAST_PERIOD = (SINCWINDOW_PERIODS/2-1),
  SINCPERIOD_SAMPLES = (CRSID_RESAMPLER_SINCPERIOD_SAMPLES), //128, //entries
   SINCWINDOW_SIZE = (SINCWINDOW_PERIODS*SINCPERIOD_SAMPLES), //1536
   SINCPERIOD_BITS = ( CRSID_BYTE_LOG2( SINCPERIOD_SAMPLES ) ), //7, //log2(SINCPERIOD_SAMPLES)
    SINCPERIOD_BITS_REVERSE = (CRSID_RESAMPLER_FRACTIONAL_BITS-SINCPERIOD_BITS),
  //SINCWINDOW_RESOLUTION = 11, //bits
   SINCWINDOW_MAGNITUDE = (CRSID_RESAMPLER_SINCWINDOW_MAGNITUDE) //(1 << SINCWINDOW_RESOLUTION) //SINCWINDOW_VALUE_MAX = 2048 // (2**SINCWINDOW_RESOLUTION)
 } AudioResamplerParameters;

 FASTVAR signed int Tmp, Left, Right;
 FASTVAR signed char Mono, ResampleBufWritePos = 0;
 FASTVAR int SincWindowPos;
 //(ChipChill: the resampler state is in the instance now, C64->ResampleBufPos etc.)
 #include "SincWindow.h"

 cRSID_precalculateHQoutputParameters(C64,  &C64->SID[1] );
 if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters(C64,  &C64->SID[2] );
 if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters(C64,  &C64->SID[3] );
 if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) cRSID_precalculateHQoutputParameters(C64,  &C64->SID[4] );
 Mono = C64->Stereo==CRSID_CHANNELMODE_MONO /*|| C64->AudioThread_SIDchipCount==1*/;

 while (C64->ResampleBufPos < C64->NextResampleBufPos) {
  ResampleBufWritePos = (C64->ResampleBufPos >> CRSID_RESAMPLER_FRACTIONAL_BITS) - SINCWINDOW_HALF1_LAST_PERIOD;
   if (ResampleBufWritePos < 0) ResampleBufWritePos += RESAMPLEBUFFER_SIZE;
  SincWindowPos = SINCPERIOD_SAMPLES - ( (C64->ResampleBufPos & FRACTIONAL_AND) >> SINCPERIOD_BITS_REVERSE );

  if ( MOSTLY (Mono) ) { //mono
   Left /*= Right*/ = cRSID_emulateHQresampledSID(C64,  &C64->SID[1], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID(C64,  &C64->SID[2], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID(C64,  &C64->SID[3], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) Left += cRSID_emulateHQresampledSID(C64,  &C64->SID[4], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   //Right = Left;
   while (SincWindowPos < SINCWINDOW_SIZE) { //Resampling subsequent stereo samples to output-sample-buffer
    C64->ResampleBufferL[ResampleBufWritePos] += (Left * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    ++ResampleBufWritePos; if (ResampleBufWritePos >= RESAMPLEBUFFER_SIZE) ResampleBufWritePos=0;
    SincWindowPos += SINCPERIOD_SAMPLES;
   }
  }
  else { //stereo
   Tmp = cRSID_emulateHQresampledSID(C64,  &C64->SID[1], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
   if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_LEFT) ) { Left = Tmp * 2; Right=0; }
   else if ( RARELY (C64->SID[1].Channel == CRSID_CHANNEL_RIGHT) ) { Right = Tmp * 2; Left=0; }
   else Left = Right = Tmp;
   if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID(C64,  &C64->SID[2], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if (C64->SID[2].Channel == CRSID_CHANNEL_LEFT)  Left += Tmp * 2;
    else if (C64->SID[2].Channel == CRSID_CHANNEL_RIGHT) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID(C64,  &C64->SID[3], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_LEFT) ) Left += Tmp * 2;
    else if ( UNLIKELY (C64->SID[3].Channel == CRSID_CHANNEL_RIGHT) ) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) {
    Tmp = cRSID_emulateHQresampledSID(C64,  &C64->SID[4], CRSID_OVERSAMPLING_CYCLES ); //.Mix;
    if (C64->SID[4].Channel == CRSID_CHANNEL_LEFT)  Left += Tmp * 2;
    else if (C64->SID[4].Channel == CRSID_CHANNEL_RIGHT) Right += Tmp * 2;
    else { Left += Tmp; Right += Tmp; }
   }
   while (SincWindowPos < SINCWINDOW_SIZE) { //Resampling subsequent stereo samples to output-sample-buffer
    C64->ResampleBufferL[ResampleBufWritePos] += (Left * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    C64->ResampleBufferR[ResampleBufWritePos] += (Right * SincWindow[SincWindowPos]) / SINCWINDOW_MAGNITUDE; // >> SINCWINDOW_RESOLUTION;
    ++ResampleBufWritePos; if (ResampleBufWritePos >= RESAMPLEBUFFER_SIZE) ResampleBufWritePos=0;
    SincWindowPos += SINCPERIOD_SAMPLES;
   }
  }

  C64->ResampleBufPos += C64->OversampleClockRatio;
 }

 if (C64->ResampleBufPos >= RESAMPLEBUFFER_SIZE_MUL) C64->ResampleBufPos -= RESAMPLEBUFFER_SIZE_MUL;
  C64->NextResampleBufPos = (C64->ResampleBufPos & INTEGER_AND) + FRACTIONAL_MUL;
 C64->ResampledOutput.L = C64->ResampleBufferL[ResampleBufWritePos] / C64->OversampleClockRatioReciproc;
  C64->ResampleBufferL[ResampleBufWritePos] = 0;
 if ( RARELY (!Mono) ) { //stereo
  C64->ResampledOutput.R = C64->ResampleBufferR[ResampleBufWritePos] / C64->OversampleClockRatioReciproc;
   C64->ResampleBufferR[ResampleBufWritePos] = 0;
 }
}



static INLINE cRSID_Output* cRSID_emulateHQresampledSIDoutputs (cRSID_C64instance* C64) { //called at samplerate-pace, only adding digi and final attenuation
 static enum { VOLUME_MAX=0xF, CHANNELS=3+1 } SIDspecs; //digi-channel is counted too in attenuation

 FASTVAR signed int Tmp;
 cRSID_Output Output; //ChipChill: returned through C64->MixedOutput (it was a static)

 cRSID_emulateHQresampledSIDdigi(C64,  &C64->SID[1], &C64->ResampledOutput );
 if ( TIGHTLY (C64->SID[2].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi(C64,  &C64->SID[2], &C64->ResampledOutput );
 if ( TIGHTLY (C64->SID[3].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi(C64,  &C64->SID[3], &C64->ResampledOutput );
 if ( TIGHTLY (C64->SID[4].BaseAddress != 0) ) cRSID_emulateHQresampledSIDdigi(C64,  &C64->SID[4], &C64->ResampledOutput );

 Output.L = C64->ResampledOutput.L / C64->Attenuation; //( ( (CHANNELS*VOLUME_MAX) + C64->Attenuation ) );  // * C64->AudioThread_SIDchipCount ); //better being multiplied in a single place than for all SIDs
 Output.R = TIGHTLY (C64->Stereo >= CRSID_CHANNELMODE_STEREO /*&& C64->AudioThread_SIDchipCount>1*/) ?
             C64->ResampledOutput.R / C64->Attenuation //( ( (CHANNELS*VOLUME_MAX) + C64->Attenuation ) ) // * C64->AudioThread_SIDchipCount )
             : Output.L;

 C64->MixedOutput = Output;
 return &C64->MixedOutput; //&C64->ResampledOutput; //&Output;
}



static INLINE short cRSID_playPSIDdigi (cRSID_C64instance* C64) {
 //static cRSID_C64instance* C64 = &cRSID_C64;

 enum PSIDdigiSpecs {
  D418_VOL_RANGE=0x10, DIGI_MAX = (D418_VOL_RANGE-1), DIGI_MASK=(DIGI_MAX), DIGI_MID = (D418_VOL_RANGE/2),
  OUTPUT_RESOLUTION=16, SID_CHANNELS=3+1, NOMINAL_DIGI_RANGE = ( (1 << OUTPUT_RESOLUTION) / SID_CHANNELS ), //16384
  DIGI_VOLUME=128 /*55*/, DIGI_VOLUME_DENOM=16, NOMINAL_DIGI_MUL = (NOMINAL_DIGI_RANGE / D418_VOL_RANGE), //1024
  DIGI_MUL = (NOMINAL_DIGI_MUL * DIGI_VOLUME / DIGI_VOLUME_DENOM) * CRSID_PRESAT_ATT_DENOM / CRSID_PRESAT_ATT_NOM
 };

 FASTVAR unsigned char Shifts;
 FASTVAR unsigned short RatePeriod;
 //(ChipChill: the state that has to be kept between calls is in the instance now, C64->PSIDdigiOutput etc.)

 if ( C64->IObankWR[0xD41D] ) {
  C64->PSIDdigiPlaybackEnabled = (C64->IObankWR[0xD41D] >= 0xFE);
  C64->PSIDdigiPeriodCounter = 0; C64->PSIDdigiNybbleCounter = 0;
  C64->PSIDdigiSampleAddress = C64->IObankWR[0xD41E] + (C64->IObankWR[0xD41F]<<8);
  C64->PSIDdigiRepeatCounter = C64->IObankWR[0xD43F];
 }
 C64->IObankWR[0xD41D] = 0;

 if (C64->PSIDdigiPlaybackEnabled) {
  RatePeriod = C64->IObankWR[0xD45D] + (C64->IObankWR[0xD45E]<<8);
  if (RatePeriod) C64->PSIDdigiPeriodCounter += C64->CPUfrequency / RatePeriod;
  if ( C64->PSIDdigiPeriodCounter >= C64->SampleRate ) {
   C64->PSIDdigiPeriodCounter -= C64->SampleRate;

   if ( C64->PSIDdigiSampleAddress < C64->IObankWR[0xD43D] + (C64->IObankWR[0xD43E]<<8) ) {
    if (C64->PSIDdigiNybbleCounter) {
     Shifts = C64->IObankWR[0xD47D] ? 4:0;
     ++C64->PSIDdigiSampleAddress;
    }
    else Shifts = C64->IObankWR[0xD47D] ? 0:4;
    C64->PSIDdigiOutput = ( ( (C64->RAMbank[C64->PSIDdigiSampleAddress]>>Shifts) & DIGI_MASK) - DIGI_MID ) * DIGI_MUL; //* DIGI_VOLUME; //* (C64->IObankWR[0xD418]&0xF);
    C64->PSIDdigiNybbleCounter^=1;
   }
   else if (C64->PSIDdigiRepeatCounter) {
    C64->PSIDdigiSampleAddress = C64->IObankWR[0xD47F] + (C64->IObankWR[0xD47E]<<8);
    C64->PSIDdigiRepeatCounter--;
   }

  }
 }

 return (C64->PSIDdigiOutput / C64->Attenuation);
}

