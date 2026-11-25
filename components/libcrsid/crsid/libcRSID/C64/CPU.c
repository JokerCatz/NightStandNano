
//cRSID CPU-emulation


enum cRSID_StatusFlagBitValues { N=0x80, V=0x40, B=0x10, D=0x08, I=0x04, Z=0x02, C=0x01 };


//static cRSID_C64instance* const C64 = &cRSID_C64; //could be a parameter but function-call is faster this way if only 1 main CPU exists
//static cRSID_CPUinstance* const CPU = &C64->CPU; //&C64->CPU;

//#define PC C64->CPU.PC  //static unsigned int PC;
//static unsigned char ST, X, Y;
//static short int A, SP;


void cRSID_initCPU (cRSID_C64instance* C64, unsigned short mempos) {
 C64->CPU.PC = mempos; C64->CPU.A = 0; C64->CPU.X = 0; C64->CPU.Y = 0;
 C64->CPU.ST = 0x04; C64->CPU.SP = 0xFF; C64->CPU.PrevNMI = 0;
}



/*static INLINE void loadReg (void) {
 PC = C64->CPU.PC; SP = C64->CPU.SP; ST = C64->CPU.ST; A = C64->CPU.A; X = C64->CPU.X; Y = C64->CPU.Y;
}
static INLINE void storeReg (void) {
 C64->CPU.PC = PC; C64->CPU.SP = SP; C64->CPU.ST = ST; C64->CPU.A = A; C64->CPU.X = X; C64->CPU.Y = Y;
}*/

static INLINE unsigned char rd (cRSID_C64instance* C64, FASTVAR unsigned short address) {
 FASTVAR unsigned char value;
 value = *cRSID_getMemReadPtr(C64, address);
 if ( TIGHTLY (C64->RealSIDmode) ) {
  if ( LIKELY (C64->RAMbank[1] & 3) ) {
   if ( RARELY (address==0xDC0D) ) { cRSID_acknowledgeCIAIRQ( &C64->CIA[1] ); }
   else if ( RARELY (address==0xDD0D) ) { cRSID_acknowledgeCIAIRQ( &C64->CIA[2] ); }
  }
 }
 return value;
}

static INLINE void wr (cRSID_C64instance* C64, FASTVAR unsigned short address, FASTVAR unsigned char data) {
 *cRSID_getMemWritePtr(C64, address)=data;
 if ( LIKELY (C64->RealSIDmode && (C64->RAMbank[1] & 3)) ) {
  //if(data&1) { //only writing 1 to $d019 bit0 would acknowledge, not any value (but RMW instructions write $d019 back before mod.)
   if ( RARELY (address==0xD019) ) { cRSID_acknowledgeVICrasterIRQ(C64); }
  //}
 }
}

static INLINE void wr2 (cRSID_C64instance* C64, FASTVAR unsigned short address, FASTVAR unsigned char data) { //PSID-hack specific memory-write
 FASTVAR int Tmp;
 *cRSID_getMemWritePtr(C64, address)=data;
 if ( LIKELY (C64->RAMbank[1] & 3) ) {

  if ( TIGHTLY (C64->RealSIDmode) ) {
   /*if (address<0xdc00 && 0xd800 <= address) C64->IObankRD[address] = C64->IObankWR[address];
   else*/ if ( RARELY ( (address & 0xFE00) == 0xDC00 ) ) {
    switch (address) {
     case 0xDC0D: cRSID_writeCIAIRQmask( &C64->CIA[1], data ); break;
     case 0xDD0D: cRSID_writeCIAIRQmask( &C64->CIA[2], data ); break;
     case 0xDC0C: C64->IObankRD[address]=data;  break; //mirror WR to RD (e.g. if byte at DC0C is used as RTI)
     case 0xDD0C: C64->IObankRD[address]=data;  break; //mirror WR to RD (e.g. Wonderland_XIII_tune_1.sid)
     case 0xDC05: cRSID_writeCIAlatchAhi( &C64->CIA[1], data ); break;
     case 0xDC07: cRSID_writeCIAlatchBhi( &C64->CIA[1], data ); break;
     case 0xDD05: cRSID_writeCIAlatchAhi( &C64->CIA[2], data ); break;
     case 0xDD07: cRSID_writeCIAlatchBhi( &C64->CIA[2], data ); break;
    }
   }
   //#ifdef CRSID_PLATFORM_PC //just for info displayer
   // else if (address==0xDC05 || address==0xDC04) C64->FrameCycles = ( (C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05]<<8)) );
   //#endif
   else if( RARELY (address==0xD019 && data&1) ) { //only writing 1 to $d019 bit0 would acknowledge
    cRSID_acknowledgeVICrasterIRQ(C64);
   }
  }

  else { //PSID-mode
   switch (address) {
    case 0xDC05: case 0xDC04:
     if ( RARELY (C64->Interface.TimerSource) ) { //dynamic CIA-setting (Galway/Rubicon workaround)
      C64->Interface.FrameCycles = ( (C64->IObankWR[0xDC04] + (C64->IObankWR[0xDC05]<<8)) ); //<< CRSID_CLOCK_FRACTIONAL_BITS) / C64->SampleClockRatio;
     }
     break;
    case 0xDC08: C64->IObankRD[0xDC08] = data; break; //refresh TOD-clock
    case 0xDC09: C64->IObankRD[0xDC09] = data; break; //refresh TOD-clock
    case 0xD012: //dynamic VIC IRQ-rasterline setting (Microprose Soccer V1 workaround)
     if (C64->PrevRasterLine >= 0) { //was $d012 set before? (or set only once?)
      if (C64->IObankWR[0xD012] != C64->PrevRasterLine) {
       Tmp = C64->IObankWR[0xD012] - C64->PrevRasterLine;
       if (Tmp<0) Tmp += C64->VIC.RasterLines;
       C64->FrameCycleCnt = C64->Interface.FrameCycles - Tmp * C64->VIC.RasterRowCycles;
      }
     }
     C64->PrevRasterLine = C64->IObankWR[0xD012];
     break;
   }
  }

 }
}


static INLINE void addrModeImmediate (cRSID_C64instance* C64)
{ ++C64->CPU.PC; C64->CPU.Addr = C64->CPU.PC; C64->CPU.Cycles=2; } //imm.

static INLINE void addrModeZeropage (cRSID_C64instance* C64)
{ ++C64->CPU.PC; C64->CPU.Addr = rd(C64, C64->CPU.PC); C64->CPU.Cycles=3; } //zp

static INLINE void addrModeAbsolute (cRSID_C64instance* C64) {
 ++C64->CPU.PC; C64->CPU.Addr = rd(C64, C64->CPU.PC);
 ++C64->CPU.PC; C64->CPU.Addr += rd(C64, C64->CPU.PC)<<8; C64->CPU.Cycles=4;
} //abs

static INLINE void addrModeZeropageXindexed (cRSID_C64instance* C64)
{ ++C64->CPU.PC; C64->CPU.Addr = (rd(C64, C64->CPU.PC) + C64->CPU.X) & 0xFF; C64->CPU.Cycles=4; } //zp,x (with zeropage-wraparound of 6502)

static INLINE void addrModeZeropageYindexed (cRSID_C64instance* C64)
{ ++C64->CPU.PC; C64->CPU.Addr = (rd(C64, C64->CPU.PC) + C64->CPU.Y) & 0xFF; C64->CPU.Cycles=4; } //zp,y (with zeropage-wraparound of 6502)

static INLINE void addrModeXindexed (cRSID_C64instance* C64) { // abs,x (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
 ++C64->CPU.PC; C64->CPU.Addr = rd(C64, C64->CPU.PC) + C64->CPU.X;
 ++C64->CPU.PC; C64->CPU.SamePage = (C64->CPU.Addr <= 0xFF); C64->CPU.Addr += rd(C64, C64->CPU.PC)<<8;
 C64->CPU.Cycles = 5;
}

static INLINE void addrModeYindexed (cRSID_C64instance* C64) { // abs,y (only STA is 5 cycles, others are 4 if page not crossed, RMW:7)
 ++C64->CPU.PC; C64->CPU.Addr = rd(C64, C64->CPU.PC) + C64->CPU.Y; ++C64->CPU.PC;
 C64->CPU.SamePage = (C64->CPU.Addr <= 0xFF); C64->CPU.Addr += rd(C64, C64->CPU.PC)<<8; C64->CPU.Cycles=5;
}

static INLINE void addrModeIndirectYindexed (cRSID_C64instance* C64) { // (zp),y (only STA is 6 cycles, others are 5 if page not crossed, RMW:8)
 ++C64->CPU.PC; C64->CPU.Addr = rd(C64, rd(C64, C64->CPU.PC)) + C64->CPU.Y;
 C64->CPU.SamePage = (C64->CPU.Addr <= 0xFF); C64->CPU.Addr += rd(C64,  (rd(C64, C64->CPU.PC)+1)&0xFF ) << 8;
 C64->CPU.Cycles = 6;
}

static INLINE void addrModeXindexedIndirect (cRSID_C64instance* C64) { // (zp,x)
 ++C64->CPU.PC;
 C64->CPU.Addr = ( rd(C64,  rd(C64, C64->CPU.PC) + C64->CPU.X ) & 0xFF )
                      + ( ( rd(C64,  rd(C64, C64->CPU.PC) + C64->CPU.X + 1 ) & 0xFF ) << 8 );
 C64->CPU.Cycles = 6;
}


static INLINE void clrC (cRSID_C64instance* C64) { C64->CPU.ST &= ~C; } //clear Carry-flag
static INLINE void setC (cRSID_C64instance* C64, FASTVAR unsigned char expr)
{ C64->CPU.ST &= ~C; C64->CPU.ST |= (expr!=0); } //set Carry-flag if expression is not zero, else clear it

static INLINE void clrNZC (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z|C); } //clear flags

static INLINE void clrNVZC (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|V|Z|C); } //clear flags

static INLINE void setNZbyA (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z); C64->CPU.ST |= ((!C64->CPU.A)<<1) | (C64->CPU.A&N); } //set Negative-flag and Zero-flag based on result in Accumulator

static INLINE void setNZbyT (cRSID_C64instance* C64, FASTVAR short int t)
{ t&=0xFF; C64->CPU.ST &= ~(N|Z); C64->CPU.ST |= ((!t)<<1) | (t&N); }

static INLINE void setNZbyX (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z); C64->CPU.ST |= ((!C64->CPU.X)<<1) | (C64->CPU.X&N); } //set Negative-flag and Zero-flag based on result in X-register

static INLINE void setNZbyY (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z); C64->CPU.ST |= ((!C64->CPU.Y)<<1) | (C64->CPU.Y&N); } //set Negative-flag and Zero-flag based on result in Y-register

static INLINE void setNZbyM (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z); C64->CPU.ST |= ((!rd(C64, C64->CPU.Addr))<<1) | (rd(C64, C64->CPU.Addr)&N); } //set Negative-flag and Zero-flag based on result at Memory-Address

static INLINE void setNZCbyAdd (cRSID_C64instance* C64)
{ C64->CPU.ST &= ~(N|Z|C); C64->CPU.ST |= (C64->CPU.A&N)|(C64->CPU.A>255); C64->CPU.A&=0xFF; C64->CPU.ST|=(!C64->CPU.A)<<1; } //after increase/addition

static INLINE void setVbyAdd (cRSID_C64instance* C64, FASTVAR unsigned char m, FASTVAR short int t) {
 C64->CPU.ST &= ~V; C64->CPU.ST |= ( (~(t^m)) & (t^C64->CPU.A) & N ) >> 1;
} //calculate V-flag from A and T (previous A) and input2 (Memory)

static INLINE void setNZCbySub (cRSID_C64instance* C64, FASTVAR signed short t)
{ C64->CPU.ST &= ~(N|Z|C); C64->CPU.ST |= (t&N) | (t>=0); /*t&=0xFF;*/ C64->CPU.ST |= ((!(t&0xFF))<<1); }

static INLINE void push (cRSID_C64instance* C64, FASTVAR unsigned char value)
{ C64->RAMbank[0x100+C64->CPU.SP] = value; --C64->CPU.SP; C64->CPU.SP&=0xFF; } //push a value to stack

static INLINE unsigned char pop (cRSID_C64instance* C64)
{ ++C64->CPU.SP; C64->CPU.SP&=0xFF; return C64->RAMbank[0x100+C64->CPU.SP]; } //pop a value from stack




unsigned char cRSID_emulateCPU (cRSID_C64instance* C64) { //the CPU emulation for SID/PRG playback (ToDo: CIA/VIC-IRQ/NMI/RESET vectors, BCD-mode)

 //enum cRSID_StatusFlagBitValues { N=0x80, V=0x40, B=0x10, D=0x08, I=0x04, Z=0x02, C=0x01 };

 static const unsigned char FlagSwitches[] = { 0x01, 0x21, 0x04, 0x24, 0x00, 0x40, 0x08, 0x28 },
                            BranchFlags[] = { 0x80, 0x40, 0x01, 0x02 };

 //static signed char /*Cycles,*/ SamePage;

 FASTVAR unsigned int /*Addr,*/ PrevPC;
 FASTVAR unsigned char IR;
 FASTVAR short int T;


 //loadReg();
 PrevPC = C64->CPU.PC;    //if (C64->CPU.PC==C64->Interface.InitAddress) printf ( "Init:$%4.4X, A:$%2.2X\n", C64->Interface.InitAddress, C64->CPU.A );
 IR = rd(C64, C64->CPU.PC); C64->CPU.Cycles=2; C64->CPU.SamePage=0; //'Cycles': ensure smallest 6510 instruction runtime (for implied/register addressing-modes)


 if (IR&1) {  //nybble2:  1/5/9/D:accu.instructions, 3/7/B/F:illegal opcodes

  switch ( (IR & 0x1F) >> 1 ) { //value-forming to cause jump-table //PC wraparound not handled inside to save codespace
   case    0:  case    1: addrModeXindexedIndirect(C64); break; //(zp,x)
   case    2:  case    3: addrModeZeropage(C64); break;
   case    4:  case    5: addrModeImmediate(C64); break;
   case    6:  case    7: addrModeAbsolute(C64); break;
   case    8:  case    9: addrModeIndirectYindexed(C64); break; //(zp),y (5..6 cycles, 8 for R-M-W)
   case  0xA:             addrModeZeropageXindexed(C64); break; //zp,x
   case  0xB:             if ((IR&0xC0)!=0x80) addrModeZeropageXindexed(C64); //zp,x for illegal opcodes
                          else addrModeZeropageYindexed(C64); //zp,y for LAX/SAX illegal opcodes
                          break;
   case  0xC:  case  0xD: addrModeYindexed(C64); break;
   case  0xE:             addrModeXindexed(C64); break;
   case  0xF:             if ((IR&0xC0)!=0x80) addrModeXindexed(C64); //abs,x for illegal opcodes
                          else addrModeYindexed(C64); //abs,y for LAX/SAX illegal opcodes
                          break;
  }
  C64->CPU.Addr &= 0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) { //value-forming to cause gapless case-values and faster jump-table creation from switch-case

   case 0: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //ORA / SLO(ASO)=ASL+ORA
            if ( RARELY ((IR&3) == 3) ) { clrNZC(C64); setC(C64, rd(C64, C64->CPU.Addr)>=N); wr(C64,  C64->CPU.Addr, rd(C64, C64->CPU.Addr)<<1 ); C64->CPU.Cycles+=2; } //for SLO
            else C64->CPU.Cycles -= C64->CPU.SamePage;
            C64->CPU.A |= rd(C64, C64->CPU.Addr); setNZbyA(C64); //ORA
           }
           else { C64->CPU.A &= rd(C64, C64->CPU.Addr); setNZbyA(C64); setC(C64,  C64->CPU.A >= N ); } //ANC (AND+Carry=bit7)
           break;

   case 1: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //AND / RLA (ROL+AND)
            if ( RARELY ((IR&3) == 3) ) { //for RLA
             T = (rd(C64, C64->CPU.Addr)<<1) + (C64->CPU.ST&C); clrNZC(C64); setC(C64, T>255); T&=0xFF; wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2;
            }
            else C64->CPU.Cycles -= C64->CPU.SamePage;
            C64->CPU.A &= rd(C64, C64->CPU.Addr); setNZbyA(C64); //AND
           }
           else { C64->CPU.A &= rd(C64, C64->CPU.Addr); setNZbyA(C64); setC(C64,  C64->CPU.A >= N ); } //ANC (AND+Carry=bit7)
           break;

   case 2: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //EOR / SRE(LSE)=LSR+EOR
            if ( RARELY ((IR&3) == 3) ) { clrNZC(C64); setC(C64, rd(C64, C64->CPU.Addr)&1); wr(C64, C64->CPU.Addr,rd(C64, C64->CPU.Addr)>>1); C64->CPU.Cycles+=2; } //for SRE
            else C64->CPU.Cycles -= C64->CPU.SamePage;
            C64->CPU.A ^= rd(C64, C64->CPU.Addr); setNZbyA(C64); //EOR
           }
           else { C64->CPU.A &= rd(C64, C64->CPU.Addr); setC(C64,  C64->CPU.A & 1 ); C64->CPU.A >>= 1; C64->CPU.A &= 0xFF; setNZbyA(C64); } //ALR(ASR)=(AND+LSR)
           break;

   case 3: if ( MOSTLY ((IR&0x1F) != 0xB) ) { //RRA (ROR+ADC) / ADC
            if( RARELY ((IR&3) == 3) ) { //for RRA
             T = (rd(C64, C64->CPU.Addr)>>1) + ((C64->CPU.ST&C)<<7); clrNZC(C64); setC(C64, T&1); wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2;
            }
            else C64->CPU.Cycles -= C64->CPU.SamePage;
            T = C64->CPU.A; C64->CPU.A += rd(C64, C64->CPU.Addr) + (C64->CPU.ST & C);
            if ( RARELY ((C64->CPU.ST & D) && (C64->CPU.A&0xF)>9) ) { C64->CPU.A+=0x10; C64->CPU.A&=0xF0; } //BCD?
            setNZCbyAdd(C64); setVbyAdd(C64, rd(C64, C64->CPU.Addr),T); //ADC
           }
           else { // ARR (AND+ROR, bit0 not going to C, but C and bit7 get exchanged.)
            C64->CPU.A &= rd(C64, C64->CPU.Addr); //T = C64->CPU.A; //T = C64->CPU.A + rd(C64, C64->CPU.Addr) + (C64->CPU.ST & C);
            //C64->CPU.ST&=~V; C64->CPU.ST |= ((T&N)>>1)^(T&V); //setVbyAdd(C64, rd(C64, C64->CPU.Addr),T); //V-flag set by intermediate ADC mechanism: (A&mem)+mem ?!
            T = C64->CPU.A; C64->CPU.A = (C64->CPU.A>>1) + ((C64->CPU.ST&C)<<7); setC(C64, T>=N); setNZbyA(C64);
            C64->CPU.ST &= ~V; C64->CPU.ST |= (T & V) ^ (C64->CPU.A & V); //corrected: V is set accoring to whether rotate changes Accu bit 6: Tbit6^Abit6)
           }
           break;

   case 4: if ( RARELY ((IR&0x1F) == 0xB) ) { C64->CPU.A = C64->CPU.X & rd(C64, C64->CPU.Addr); setNZbyA(C64); } //XAA (TXA+AND), highly unstable on real 6502!
           else if ( RARELY ((IR&0x1F) == 0x1B) ) { C64->CPU.SP = C64->CPU.A & C64->CPU.X; wr(C64,  C64->CPU.Addr, C64->CPU.SP & ((C64->CPU.Addr>>8)+1) ); } //TAS(SHS) (SP=A&X, mem=S&H} - unstable on real 6502
           else { wr2(C64,  C64->CPU.Addr, C64->CPU.A & (RARELY((IR&3)==3)? C64->CPU.X:0xFF) ); } //STA / SAX (at times same as AHX/SHX/SHY) (illegal)
           break;

   case 5: if ( MOSTLY ((IR&0x1F) != 0x1B) ) { C64->CPU.A=rd(C64, C64->CPU.Addr); if(RARELY((IR&3)==3)) C64->CPU.X=C64->CPU.A; } //LDA / LAX (illegal, used by my 1 rasterline player) (LAX #imm is unstable on C64)
           else { C64->CPU.A=C64->CPU.X=C64->CPU.SP = rd(C64, C64->CPU.Addr) & C64->CPU.SP; } //LAS(LAR)
           setNZbyA(C64); C64->CPU.Cycles -= C64->CPU.SamePage;
           break;

   case 6: if( MOSTLY ((IR&0x1F) != 0xB) ) { // CMP / DCP(DEC+CMP)
            if ( RARELY ((IR&3) == 3) ) { wr(C64, C64->CPU.Addr,rd(C64, C64->CPU.Addr)-1); C64->CPU.Cycles+=2;} //DCP
            else C64->CPU.Cycles -= C64->CPU.SamePage;
            T = C64->CPU.A - rd(C64, C64->CPU.Addr);
           }
           else { C64->CPU.X = T = (C64->CPU.A & C64->CPU.X) - rd(C64, C64->CPU.Addr); C64->CPU.X &= 0xFF; } //SBX(AXS)  //SBX (AXS) (CMP+DEX at the same time)
           setNZCbySub(C64,  T );
           break;

   case 7: if( RARELY ((IR&3)==3 && (IR&0x1F)!=0xB) ) { wr(C64,  C64->CPU.Addr, rd(C64, C64->CPU.Addr)+1 ); C64->CPU.Cycles+=2; } //ISC(ISB)=INC+SBC / SBC
           else C64->CPU.Cycles -= C64->CPU.SamePage;
           T = C64->CPU.A; C64->CPU.A -= rd(C64, C64->CPU.Addr) + !(C64->CPU.ST & C);
           setNZCbySub(C64,  C64->CPU.A ); C64->CPU.A &= 0xFF; setVbyAdd(C64,  ~rd(C64, C64->CPU.Addr), T );
           break;
  }
 }


 else if (IR&2) {  //nybble2:  2:illegal/LDX, 6:A/X/INC/DEC, A:Accu-shift/reg.transfer/NOP, E:shift/X/INC/DEC

  switch (IR & 0x1F) { //Addressing modes
   case    2: addrModeImmediate(C64); break;
   case    6: addrModeZeropage(C64); break;
   case  0xE: addrModeAbsolute(C64); break;
   case 0x16: if ( (IR&0xC0) != 0x80 ) addrModeZeropageXindexed(C64); //zp,x
              else addrModeZeropageYindexed(C64); //zp,y
              break;
   case 0x1E: if ( (IR&0xC0) != 0x80 ) addrModeXindexed(C64); //abs,x
              else addrModeYindexed(C64); //abs,y
              break;
  }
  C64->CPU.Addr&=0xFFFF;

  switch ( (IR & 0xE0) >> 5 ) {

   case 0: clrC(C64); //clear C for ASL //the rest of case 0 and 1 are identical but newer GCC gave notifications about 'fallthrough', so duplicated it
           if ( CALMLY ((IR&0xF)==0xA) ) { C64->CPU.A = (C64->CPU.A << 1) + (C64->CPU.ST & C); setNZCbyAdd(C64); } //ASL/ROL (Accu)
           else { T = (rd(C64, C64->CPU.Addr)<<1) + (C64->CPU.ST & C); setC(C64, T>255); setNZbyT(C64, T); wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2; } //RMW (Read-Write-Modify)
           break;
   case 1: if ( CALMLY ((IR&0xF)==0xA) ) { C64->CPU.A = (C64->CPU.A << 1) + (C64->CPU.ST & C); setNZCbyAdd(C64); } //ASL/ROL (Accu)
           else { T = (rd(C64, C64->CPU.Addr)<<1) + (C64->CPU.ST & C); setC(C64, T>255); setNZbyT(C64, T); wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2; } //RMW (Read-Write-Modify)
           break;

   case 2: clrC(C64); //clear C for LSR //the rest of case 2 and 3 are identical but newer GCC gave notifications about 'fallthrough', so duplicated it
           if ( CALMLY ((IR&0xF)==0xA) ) { T = C64->CPU.A; C64->CPU.A= (C64->CPU.A >> 1) + ((C64->CPU.ST&C) << 7); setC(C64, T&1); C64->CPU.A &= 0xFF; setNZbyA(C64); } //LSR/ROR (Accu)
           else { T = (rd(C64, C64->CPU.Addr)>>1) + ((C64->CPU.ST&C) << 7); setC(C64,  rd(C64, C64->CPU.Addr) & 1 ); setNZbyT(C64, T); wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2; } //memory (RMW)
           break;
   case 3: if ( CALMLY ((IR&0xF)==0xA) ) { T = C64->CPU.A; C64->CPU.A= (C64->CPU.A >> 1) + ((C64->CPU.ST&C) << 7); setC(C64, T&1); C64->CPU.A &= 0xFF; setNZbyA(C64); } //LSR/ROR (Accu)
           else { T = (rd(C64, C64->CPU.Addr)>>1) + ((C64->CPU.ST&C) << 7); setC(C64,  rd(C64, C64->CPU.Addr) & 1 ); setNZbyT(C64, T); wr(C64, C64->CPU.Addr,T); C64->CPU.Cycles+=2; } //memory (RMW)
           break;

   case 4: if ( MOSTLY (IR&4) ) { wr2(C64,  C64->CPU.Addr, C64->CPU.X ); } //STX
           else if ( RARELY (IR&0x10) ) C64->CPU.SP = C64->CPU.X; //TXS
           else { C64->CPU.A = C64->CPU.X; setNZbyA(C64); } //TXA
           break;

   case 5: if ( MOSTLY ((IR&0xF) != 0xA) ) { C64->CPU.X = rd(C64, C64->CPU.Addr); C64->CPU.Cycles -= C64->CPU.SamePage; } //LDX
           else if ( RARELY (IR & 0x10) ) C64->CPU.X = C64->CPU.SP; //TSX
           else C64->CPU.X = C64->CPU.A; //TAX
           setNZbyX(C64);
           break;

   case 6: if ( TIGHTLY (IR&4) ) { wr(C64, C64->CPU.Addr,rd(C64, C64->CPU.Addr)-1); setNZbyM(C64); C64->CPU.Cycles+=2; } //DEC
           else { --C64->CPU.X; setNZbyX(C64); } //DEX
           break;

   case 7: if ( TIGHTLY (IR&4) ) { wr(C64, C64->CPU.Addr,rd(C64, C64->CPU.Addr)+1); setNZbyM(C64); C64->CPU.Cycles+=2; } //INC/NOP
           break;
  }
 }


 else if ( (IR & 0xC) == 8 ) {  //nybble2:  8:register/statusflag
  if ( IR&0x10 ) {
   if ( IR == 0x98 ) { C64->CPU.A = C64->CPU.Y; setNZbyA(C64); } //TYA
   else { //CLC/SEC/CLI/SEI/CLV/CLD/SED
    if (FlagSwitches[IR>>5] & 0x20) C64->CPU.ST |= (FlagSwitches[IR>>5] & 0xDF);
    else C64->CPU.ST &= ~( FlagSwitches[IR>>5] & 0xDF );
   }
  }
  else {
   switch ( (IR & 0xF0) >> 5 ) {
    case 0: push(C64,  C64->CPU.ST ); C64->CPU.Cycles=3; break; //PHP
    case 1: C64->CPU.ST = pop(C64); C64->CPU.Cycles=4; break; //PLP
    case 2: push(C64,  C64->CPU.A ); C64->CPU.Cycles=3; break; //PHA
    case 3: C64->CPU.A = pop(C64); setNZbyA(C64); C64->CPU.Cycles=4; break; //PLA
    case 4: --C64->CPU.Y; setNZbyY(C64); break; //DEY
    case 5: C64->CPU.Y = C64->CPU.A; setNZbyY(C64); break; //TAY
    case 6: ++C64->CPU.Y; setNZbyY(C64); break; //INY
    case 7: ++C64->CPU.X; setNZbyX(C64); break; //INX
   }
  }
 }


 else {  //nybble2:  0: control/branch/Y/compare  4: Y/compare  C:Y/compare/JMP

  if ( (IR&0x1F) == 0x10 ) { //BPL/BMI/BVC/BVS/BCC/BCS/BNE/BEQ  relative branch
   ++C64->CPU.PC;
   T = rd(C64,  C64->CPU.PC ); if (T & 0x80) T -= 0x100;
   if (IR & 0x20) {
    if (C64->CPU.ST & BranchFlags[IR>>6]) { C64->CPU.PC += T; C64->CPU.Cycles=3; }
   }
   else {
    if ( !(C64->CPU.ST & BranchFlags[IR>>6]) ) { C64->CPU.PC += T; C64->CPU.Cycles=3; } //plus 1 cycle if page is crossed?
   }
  }

  else {  //nybble2:  0:Y/control/Y/compare  4:Y/compare  C:Y/compare/JMP
   switch (IR&0x1F) { //Addressing modes
    case    0: addrModeImmediate(C64); break; //imm. (or abs.low for JSR/BRK)
    case    4: addrModeZeropage(C64); break;
    case  0xC: addrModeAbsolute(C64); break;
    case 0x14: addrModeZeropageXindexed(C64); break; //zp,x
    case 0x1C: addrModeXindexed(C64); break; //abs,x
   }
   C64->CPU.Addr &= 0xFFFF;

   switch ( (IR & 0xE0) >> 5 ) {

    case 0: if( TIGHTLY (!(IR&4)) ) { //BRK / NOP-absolute/abs,x/zp/zp,x
             push(C64, (C64->CPU.PC+2-1) >> 8); push(C64, (C64->CPU.PC+2-1) & 0xFF); push(C64, C64->CPU.ST|B); C64->CPU.ST |= I; //BRK
             C64->CPU.PC = rd(C64, 0xFFFE) + (rd(C64, 0xFFFF)<<8) - 1; C64->CPU.Cycles=7;
            }
            else if ( RARELY (IR == 0x1C) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            break;

    case 1: if ( CALMLY (IR & 0xF) ) { //BIT / NOP-abs,x/zp,x
             if ( MOSTLY (!(IR&0x10)) ) { C64->CPU.ST &= 0x3D; C64->CPU.ST |= (rd(C64, C64->CPU.Addr) & 0xC0) | ( (!(C64->CPU.A & rd(C64, C64->CPU.Addr))) << 1 ); } //BIT
             else if ( RARELY (IR == 0x3C) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            }
            else { //JSR
             push(C64,  (C64->CPU.PC+2-1) >> 8 ); push(C64,  (C64->CPU.PC+2-1) & 0xFF );
             C64->CPU.PC = rd(C64, C64->CPU.Addr) + rd(C64, C64->CPU.Addr+1) * 256 - 1; C64->CPU.Cycles=6;
            }
            break;

    case 2: if ( MOSTLY (IR & 0xF) ) { //JMP / NOP-abs,x/zp/zp,x
             if ( MOSTLY (IR == 0x4C) ) { //JMP
              C64->CPU.PC = ( RARELY ( C64->RealSIDmode && C64->NMI && (PrevPC == 0xDC02 || PrevPC == 0xDC04) ) ) ?
                                 (unsigned int) ( ( (/*rd(C64, C64->CPU.PC+1)==2?0:*/0x800) + (C64->CPU.Addr&0xFF) ) - 1 ) //Workaround: Hi_Fi_Sky.sid/WonderLand-XII/Hunters_Moon/File_Deleted/Storebror.sid and the like needs cycle/subcycle-exact emulation to work well. This value is OK (though jittery) for most tunes (as WebSID proved).
                                 : (C64->CPU.Addr - 1);
              C64->CPU.Cycles=3; rd(C64, C64->CPU.Addr+1); //a read from jump-address highbyte to acknowledge CIA-IRQ is used in some tunes with 'jmp DC0C' or 'jmp DD0C' (e.g. Wonderland_XIII_tune_1.sid or Hi_Fi_Sky.sid)
              //if (C64->CPU.Addr==PrevPC) {storeReg(); C64->Returned=1; return 0xFF;} //turn self-jump mainloop (after init) into idle time
             }
             else if ( RARELY (IR==0x5C) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            }
            else { //RTI
             C64->CPU.ST = pop(C64); T = pop(C64); C64->CPU.PC = (pop(C64) << 8) + T - 1; C64->CPU.Cycles=6;
             if ( LIKELY (C64->Returned && C64->CPU.SP >= 0xFF) ) { ++C64->CPU.PC; /*storeReg();*/ return 0xFE; }
            }
            break;

    case 3: if ( CALMLY (IR & 0xF) ) { //JMP() (indirect) / NOP-abs,x/zp/zp,x
             if ( MOSTLY (IR == 0x6C) ) { //JMP() (indirect)
              C64->CPU.PC = rd(C64,  (C64->CPU.Addr&0xFF00) + ((C64->CPU.Addr+1)&0xFF) ); //(with highbyte-wraparound bug)
              C64->CPU.PC = (C64->CPU.PC << 8) + rd(C64, C64->CPU.Addr) - 1; C64->CPU.Cycles=5;
             }
             else if ( RARELY (IR == 0x7C) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            }
            else { //RTS
             if ( RARELY (C64->CPU.SP >= 0xFF) ) {/*storeReg();*/ C64->Returned=1; return 0xFF;} //Init returns, provide idle-time between IRQs
             T=pop(C64); C64->CPU.PC = (pop(C64) << 8) + T; C64->CPU.Cycles=6;
            }
            break;

    case 4: if ( MOSTLY (IR & 4) ) { wr2(C64,  C64->CPU.Addr, C64->CPU.Y ); } //STY / NOP #imm
            break;

    case 5: C64->CPU.Y = rd(C64, C64->CPU.Addr); setNZbyY(C64); C64->CPU.Cycles -= C64->CPU.SamePage; //LDY
            break;

    case 6: if ( MOSTLY (!(IR&0x10)) ) { //CPY / NOP abs,x/zp,x
             T = C64->CPU.Y - rd(C64, C64->CPU.Addr); setNZCbySub(C64,  T ); //CPY
            }
            else if ( RARELY (IR==0xDC) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            break;

    case 7: if ( MOSTLY (!(IR&0x10)) ) { //CPX / NOP abs,x/zp,x
             T = C64->CPU.X - rd(C64, C64->CPU.Addr); setNZCbySub(C64,  T ); //CPX
            }
            else if ( RARELY (IR==0xFC) ) C64->CPU.Cycles -= C64->CPU.SamePage; //NOP abs,x
            break;
   }
  }
 }


 ++C64->CPU.PC; //PC&=0xFFFF;

 //storeReg();


 if ( CALMLY (!C64->RealSIDmode) ) { //substitute KERNAL IRQ-return in PSID (e.g. Microprose Soccer)
  if ( RARELY ( (C64->RAMbank[1]&3)>1 && PrevPC<0xE000 && (C64->CPU.PC==0xEA31 || C64->CPU.PC==0xEA81 || C64->CPU.PC==0xEA7E) ) ) return 0xFE;
 }


 return C64->CPU.Cycles;
}



//INLINE void push (C64, unsigned char value) { C64->CPU.cRSID_C64.RAMbank[0x100+C64->CPU.SP] = value; --C64->CPU.SP; C64->CPU.SP&=0xFF; } //push a value to stack

 //handle entering into IRQ and NMI interrupt
static INLINE char cRSID_handleCPUinterrupts (cRSID_C64instance* C64) {
 enum StatusFlagBitValues { B=0x10, I=0x04 };

 if ( RARELY (C64->NMI > C64->CPU.PrevNMI) ) { //if IRQ and NMI at the same time, NMI is serviced first (or is it?!)
  //C64->CPU.ST &= ~B;
  push(C64, C64->CPU.PC>>8); push(C64, C64->CPU.PC&0xFF); push(C64, C64->CPU.ST); C64->CPU.ST |= I;
  C64->CPU.PC = *cRSID_getMemReadPtr(C64, 0xFFFA) + (*cRSID_getMemReadPtr(C64, 0xFFFB)<<8); //NMI-vector
  C64->CPU.PrevNMI = C64->NMI;
  return 1;
 }
 else if ( RARELY (C64->IRQ && !(C64->CPU.ST&I) ) ) {
  //C64->CPU.ST &= ~B;
  push(C64, C64->CPU.PC>>8); push(C64, C64->CPU.PC&0xFF); push(C64, C64->CPU.ST); C64->CPU.ST |= I;
  C64->CPU.PC = *cRSID_getMemReadPtr(C64, 0xFFFE) + (*cRSID_getMemReadPtr(C64, 0xFFFF)<<8); //maskable IRQ-vector
  C64->CPU.PrevNMI = C64->NMI;
  return 1;
 }
 C64->CPU.PrevNMI = C64->NMI; //prepare for NMI edge-detection

 return 0;
}
