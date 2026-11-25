
#include "stdint.h"
#include "stdio.h"
#include "string.h"




cRSID_SIDheader* cRSID_processSIDfileData (cRSID_C64instance* C64, unsigned char* filedata, int filesize) {
 int i;
 unsigned short SIDdataOffset;
 cRSID_SIDheader *SIDheader;
 static const char MagicStringPSID[]="PSID";
 //static const char MagicStringRSID[]="RSID";

 C64->Interface.SIDheader = SIDheader = (cRSID_SIDheader*) filedata;

 //copy KERNAL & BASIC ROM contents into the RAM under them? (So PSIDs that don't select bank correctly will work better.)
 for (i=0xA000; i<0x10000; ++i) C64->RAMbank[i]=C64->ROMbanks[i];
 for (i=0x0000; i < 0xA000; ++i) C64->RAMbank[i]=0; //fresh start (maybe some bugged SIDs want 0 at certain RAM-locations)
 for (i=0xC000; i < 0xD000; ++i) C64->RAMbank[i]=0;

 if ( SIDheader->MagicString[0] != 'P' && SIDheader->MagicString[0] != 'R' ) return NULL;
 for (i=1; i < (int)(sizeof(MagicStringPSID)-1); ++i) { if (SIDheader->MagicString[i] != MagicStringPSID[i]) return NULL; }
 C64->Interface.RealSIDmode = ( SIDheader->MagicString[0] == 'R' );
 C64->RealSIDmode = C64->Interface.RealSIDmode; //update shadowed copy used by the audio thread (sound-thread shouldn't run at this moment)

 if (SIDheader->LoadAddressH==0 && SIDheader->LoadAddressL==0) { //load-address taken from first 2 bytes of the C64 PRG
  C64->Interface.LoadAddress = (filedata[SIDheader->HeaderSize+1]<<8) + (filedata[SIDheader->HeaderSize+0]);
  SIDdataOffset = SIDheader->HeaderSize+2;
 }
 else { //load-adress taken from SID-header
  C64->Interface.LoadAddress = (SIDheader->LoadAddressH<<8) + (SIDheader->LoadAddressL);
  SIDdataOffset = SIDheader->HeaderSize;
 }

 //Used by subtune-playback too after first init, so tunes that modify memory can start with a freshly loaded data:
 for (i=SIDdataOffset; i<filesize; ++i) C64->RAMbank [ C64->Interface.LoadAddress + (i-SIDdataOffset) ] = filedata[i];

 i = C64->Interface.LoadAddress + (filesize-SIDdataOffset);
 C64->Interface.EndAddress = (i<0x10000) ? i : 0xFFFF;

 C64->Interface.PSIDdigiMode = ( !C64->Interface.RealSIDmode && (SIDheader->ModelFormatStandard & 2) );

 for (i=0; i<=CRSID_SUBTUNE_AMOUNT_MAX; ++i) C64->Interface.SubtuneDurations[i] = 0; //no song-length database in this build
 if (C64->Interface.FallbackPlayTime > 0) {
    for (i=0; i<=CRSID_SUBTUNE_AMOUNT_MAX; ++i) { if (C64->Interface.SubtuneDurations[i] <= 0) C64->Interface.SubtuneDurations[i] = C64->Interface.FallbackPlayTime; }
 }


 return C64->Interface.SIDheader;
}

