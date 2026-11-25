// cRSID lightweight (integer-only) RealSID library (with API-calls) by Hermit (Mihaly Horvath), Year 2026
// License: WTF - do what the frick you want with this code, but please mention me as the original author
#define LIBCRSID_SOURCE


#include <stdlib.h>
#include <stdio.h>
//#include <stdint.h>
#include <unistd.h>

#include "libcRSID.h"

#include "Config.h"
#include "Optimize.h"

#include "C64/C64.c"
#include "host/file.c"
#include "host/audio.c"



cRSID_C64instance* cRSID_init (unsigned short samplerate, unsigned short buflen) {
 cRSID_C64instance* C64 = calloc( 1, sizeof(cRSID_C64instance) ); //zeroed, as the globals it replaces were
 if (C64 == NULL) return NULL;
 C64->DitherLFSR = 0x0055A5AA; //the initial values the former static variables had
 C64->NextResampleBufPos = 1 << CRSID_RESAMPLER_FRACTIONAL_BITS;

 C64->Interface.RealSIDmode = 1;
 C64->Interface.Stereo = CRSID_CHANNELMODE_MONO;
 C64->Interface.HighQualitySID = 1;
 C64->Interface.HighQualityResampler = 0; //1;  //was problematic (noisy) with lowpass-filtered triangles on all SID-channels
 C64->RealSIDmode = C64->Interface.RealSIDmode; C64->Stereo = C64->Interface.Stereo; //init shadow-variables to defaults for audio-thread as well
 C64->HighQualitySID = C64->Interface.HighQualitySID; C64->HighQualityResampler = C64->Interface.HighQualityResampler;
 C64->Interface.SelectedSIDmodel = 0; //default model and mode selections
 C64->Interface.PlaybackSpeed = 1;
 C64->Interface.MainVolume = 255; //230; //255; //230; //204; //255;
 C64->Interface.CLImode = 0; C64->Interface.AutoExit = C64->Interface.BuiltInMusic = C64->Interface.BuiltInMusicSize = 0; C64->Interface.BuiltInMusicData = NULL;
 C64->Interface.OpenedMusic = 0;
 C64->Interface.AutoAdvance = C64->Interface.FadeOut = 1;
 C64->FadeLevel=0xF;
 C64->Interface.PlayListSize = 0; C64->Interface.PlayListNumber = 1;
 C64->Interface.PlayListPlayPosition = C64->Interface.PlayListAdvance = 0;
 C64->Interface.SongLengths = NULL;  C64->Interface.KERNALfileData = NULL; C64->Interface.BASICfileData = NULL;

 C64 = cRSID_createC64(C64,  samplerate );
 if (buflen) return C64; //this is here just to eliminate unused 'buflen' variable warning

 return C64;
}



void cRSID_initSIDtune (cRSID_C64instance* C64, cRSID_SIDheader* SIDheader, int subtune) { //subtune: 1..255
 static const unsigned char PowersOf2[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
 unsigned int InitTimeout=10000000; //allowed instructions, value should be selected to allow for long-running memory-copiers in init-routines (e.g. Synth Sample)

 C64->Interface.PlaytimeExpired=0; C64->FadeLevel=0xF; C64->Interface.Paused = 1;

 if (subtune==0) subtune = 1;
 else if (subtune > SIDheader->SubtuneAmount) subtune = SIDheader->SubtuneAmount;    //printf( "Subtune: %d\n", subtune );
 C64->Interface.SubTune = subtune; C64->SecondCnt = C64->Interface.PlayTime = 0; C64->Interface.PlaybackSpeed=1; //C64->Interface.Paused = 0; //don't unpause this early, before init!

 cRSID_setC64(C64); cRSID_initC64(C64); //cRSID_writeMemC64(C64,0xD418,0xF); //set C64 hardware and init (reset) it
 //C64->Interface.Paused = 0;

 //determine init-address:
 C64->Interface.InitAddress = ((SIDheader->InitAddressH)<<8) + (SIDheader->InitAddressL); //get info from BASIC-startupcode for some tunes
 if (C64->RAMbank[1] == 0x37) { //are there SIDs with routine under IO area? some PSIDs don't set bank-registers themselves
  if ( (0xA000 <= C64->Interface.InitAddress && C64->Interface.InitAddress < 0xC000)
       || (C64->Interface.LoadAddress < 0xC000 && C64->Interface.EndAddress >= 0xA000) ) C64->RAMbank[1] = 0x36;
  else if (C64->Interface.InitAddress >= 0xE000 || C64->Interface.EndAddress >=0xE000) C64->RAMbank[1] = 0x35;
 }
 cRSID_initCPU(C64,  C64->Interface.InitAddress ); //prepare init-routine call
 C64->CPU.A = subtune - 1;

 if (!C64->Interface.RealSIDmode) {
  //call init-routine:
  for (InitTimeout=10000000; InitTimeout>0; InitTimeout--) { if ( cRSID_emulateCPU(C64)>=0xFE ) break; } //give error when timed out?
 }

 //determine timing-source, if CIA, replace FrameCycles previouisly set to VIC-timing
 if (subtune>32) C64->Interface.TimerSource = C64->Interface.SIDheader->SubtuneTimeSources[0] & 0x80; //subtunes above 32 should use subtune32's timing
 else C64->Interface.TimerSource = C64->Interface.SIDheader->SubtuneTimeSources[(32-subtune)>>3] & PowersOf2[(subtune-1)&7];
 if (C64->Interface.TimerSource || C64->IObankWR[0xDC05]!=0x40 || C64->IObankWR[0xDC04]!=0x24) { //CIA1-timing (probably multispeed tune)
  C64->Interface.FrameCycles = ( ( C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05]<<8) ) ); //<< 4) / C64->ClockRatio;
  C64->Interface.TimerSource = 1; //if init-routine changed DC04 or DC05, assume CIA-timing
 }

 //determine playaddress:
 C64->Interface.PlayAddress = (SIDheader->PlayAddressH<<8) + SIDheader->PlayAddressL;
 if (C64->Interface.PlayAddress) { //normal play-address called with JSR
  if (C64->RAMbank[1] == 0x37) { //are there SIDs with routine under IO area?
   if (0xA000 <= C64->Interface.PlayAddress && C64->Interface.PlayAddress < 0xC000) C64->RAMbank[1] = 0x36;
  }
  else if (C64->Interface.PlayAddress >= 0xE000) C64->RAMbank[1] = 0x35; //player under KERNAL (e.g. Crystal Kingdom Dizzy)
 }
 else { //IRQ-playaddress for multispeed-tunes set by init-routine (some tunes turn off KERNAL ROM but doesn't set IRQ-vector!)
  C64->Interface.PlayAddress = (C64->RAMbank[1] & 3) < 2 ? cRSID_readMem(C64, 0xFFFE) + (cRSID_readMem(C64, 0xFFFF)<<8) //cRSID_readMemC64(C64,0xFFFE) + (cRSID_readMemC64(C64,0xFFFF)<<8) //for PSID
                                                 : cRSID_readMem(C64, 0x314) + (cRSID_readMem(C64, 0x315)<<8); //: cRSID_readMemC64(C64,0x314) + (cRSID_readMemC64(C64,0x315)<<8);
  if (C64->Interface.PlayAddress==0) { //if 0, still try with RSID-mode fallback
   cRSID_initCPU(C64,  C64->Interface.PlayAddress ); //point CPU to play-routine
   C64->Finished=1; C64->Returned=1; C64->Interface.Paused = 0; return;
  }
 }

 if (!C64->Interface.RealSIDmode) {  //prepare (PSID) play-routine playback:
  cRSID_initCPU(C64,  C64->Interface.PlayAddress ); //point CPU to play-routine
  C64->FrameCycleCnt=0; C64->Finished=1; C64->SampleCycleCnt=0; //C64->CIAisSet=0;
 }
 else { C64->Finished=0; C64->Returned=0; }

 C64->Interface.Paused = 0;
}


char* cRSID_setSongLengthData (cRSID_C64instance* C64, char* filedata) {
 if (filedata != NULL) { C64->Interface.SongLengths = filedata; return C64->Interface.SongLengths; }
 return filedata;
}

unsigned char* cRSID_setKERNALdata (cRSID_C64instance* C64, unsigned char* filedata) {
 if (filedata != NULL) { C64->Interface.KERNALfileData = filedata; cRSID_setROMcontent(C64); return C64->Interface.KERNALfileData; }
 return filedata;
}

unsigned char* cRSID_setBASICdata (cRSID_C64instance* C64, unsigned char* filedata) {
 if (filedata != NULL) { C64->Interface.BASICfileData = filedata; cRSID_setROMcontent(C64); return C64->Interface.BASICfileData; }
 return filedata;
}


void cRSID_close (cRSID_C64instance* C64) {
 free(C64);
}


void cRSID_playSIDtune (cRSID_C64instance* C64) {
 C64->Interface.Paused = 0;
}


void cRSID_pauseSIDtune (cRSID_C64instance* C64) {
 C64->Interface.Paused = 1; //cause immediate stopping in audio-buffer thread, so SID-baseaddress changes during tune-switching won't give segfaults
}


void cRSID_startSubtune (cRSID_C64instance* C64, unsigned char subtune) {
 int i;
 cRSID_pauseSIDtune(C64);
 cRSID_processSIDfileData(C64,  (unsigned char*) C64->Interface.SIDfileData, C64->Interface.SIDfileSize );
 cRSID_initSIDtune(C64,  C64->Interface.SIDheader, subtune ); //C64->Interface.PlaybackSpeed=1; /*C64->Interface.Paused=0;*/
 cRSID_playSIDtune(C64);
}

void cRSID_nextSubtune(cRSID_C64instance* C64) { if (C64->Interface.SubTune<CRSID_SUBTUNE_AMOUNT_MAX-1) cRSID_startSubtune(C64, ++C64->Interface.SubTune); }

void cRSID_prevSubtune(cRSID_C64instance* C64) { if (C64->Interface.SubTune>0) cRSID_startSubtune(C64, --C64->Interface.SubTune); }






#undef LIBCRSID_SOURCE