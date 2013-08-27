#include <windows.h>
#include <assert.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "savestate.h"
#include "register.h"
#include "bpenum.h"
#include "c6502.h"

const InstructionInfo CPU6502::AssemblyData[256]=
{
	{0x00,TEXT("BRK"),amIMPLIED,0,1,},
	{0x01,TEXT("ORA"),amINDIRECTX,0,2,},
	{0x02,TEXT("HLT"),amIMPLIED,1,1},
	{0x03,TEXT("SLO"),amINDIRECTX,1,2},
	{0x04,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x05,TEXT("ORA"),amZEROPAGE,0,2},
	{0x06,TEXT("ASL"),amZEROPAGE,0,2},
	{0x07,TEXT("SLO"),amZEROPAGE,1,2},

	{0x08,TEXT("PHP"),amIMPLIED,0,1},
	{0x09,TEXT("ORA"),amIMMEDIATE,0,2},
	{0x0A,TEXT("ASL"),amIMPLIED,0,1},
	{0x0B,TEXT("ANC"),amIMMEDIATE,1,2},
	{0x0C,TEXT("SKW"),amABSOLUTE,1,3},
	{0x0D,TEXT("ORA"),amABSOLUTE,0,3},
	{0x0E,TEXT("ASL"),amABSOLUTE,0,3},
	{0x0F,TEXT("SLO"),amABSOLUTE,1,3},

	{0x10,TEXT("BPL"),amRELATIVE,0,2},
	{0x11,TEXT("ORA"),amINDIRECTY,0,2},
	{0x12,TEXT("HLT"),amIMPLIED,1,1},
	{0x13,TEXT("SLO"),amINDIRECTY,1,2},
	{0x14,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x15,TEXT("ORA"),amZEROPAGEX,0,2},
	{0x16,TEXT("ASL"),amZEROPAGEX,0,2},
	{0x17,TEXT("SLO"),amZEROPAGEX,1,2},

	{0x18,TEXT("CLC"),amIMPLIED,0,1},
	{0x19,TEXT("ORA"),amABSOLUTEY,0,3},
	{0x1a,TEXT("NOP"),amIMPLIED,1,1},
	{0x1b,TEXT("SLO"),amABSOLUTEY,1,3},
	{0x1c,TEXT("SKW"),amABSOLUTEX,1,3},
	{0x1d,TEXT("ORA"),amABSOLUTEX,0,3},
	{0x1e,TEXT("ASL"),amABSOLUTEX,0,3},
	{0x1f,TEXT("SLO"),amABSOLUTEX,1,3},


	{0x20,TEXT("JSR"),amABSOLUTE,0,3},
	{0x21,TEXT("AND"),amINDIRECTX,0,2},
	{0x22,TEXT("HLT"),amIMPLIED,1,1},
	{0x23,TEXT("RLA"),amINDIRECTX,1,2},
	{0x24,TEXT("BIT"),amZEROPAGE,0,2},
	{0x25,TEXT("AND"),amZEROPAGE,0,2},
	{0x26,TEXT("ROL"),amZEROPAGE,0,2},
	{0x27,TEXT("RLA"),amZEROPAGE,1,2},

	{0x28,TEXT("PLP"),amIMPLIED,0,1},
	{0x29,TEXT("AND"),amIMMEDIATE,0,2},
	{0x2a,TEXT("ROL"),amIMPLIED,0,1},
	{0x2b,TEXT("ANC"),amIMMEDIATE,1,2},
	{0x2c,TEXT("BIT"),amABSOLUTE,0,3},
	{0x2d,TEXT("AND"),amABSOLUTE,0,3},
	{0x2e,TEXT("ROL"),amABSOLUTE,0,3},
	{0x2f,TEXT("RLA"),amABSOLUTE,1,3},

	{0x30,TEXT("BMI"),amRELATIVE,0,2},
	{0x31,TEXT("AND"),amINDIRECTY,0,2},
	{0x32,TEXT("HLT"),amIMPLIED,1,1},
	{0x33,TEXT("RLA"),amINDIRECTY,1,2},
	{0x34,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x35,TEXT("AND"),amZEROPAGEX,0,2},
	{0x36,TEXT("ROL"),amZEROPAGEX,0,2},
	{0x37,TEXT("RLA"),amZEROPAGEX,1,2},

	{0x38,TEXT("SEC"),amIMPLIED,0,1},
	{0x39,TEXT("AND"),amABSOLUTEY,0,3},
	{0x3a,TEXT("NOP"),amIMPLIED,1,1},
	{0x3b,TEXT("RLA"),amABSOLUTEY,1,3},
	{0x3c,TEXT("SKW"),amABSOLUTEX,1,3},
	{0x3d,TEXT("AND"),amABSOLUTEX,0,3},
	{0x3e,TEXT("ROL"),amABSOLUTEX,0,3},
	{0x3f,TEXT("RLA"),amABSOLUTEX,1,3},


	{0x40,TEXT("RTI"),amIMPLIED,0,1},
	{0x41,TEXT("EOR"),amINDIRECTX,0,2},
	{0x42,TEXT("HLT"),amIMPLIED,1,1},
	{0x43,TEXT("SRE"),amINDIRECTX,1,2},
	{0x44,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x45,TEXT("EOR"),amZEROPAGE,0,2},
	{0x46,TEXT("LSR"),amZEROPAGE,0,2},
	{0x47,TEXT("SRE"),amZEROPAGE,1,2},

	{0x48,TEXT("PHA"),amIMPLIED,0,1},
	{0x49,TEXT("EOR"),amIMMEDIATE,0,2},
	{0x4a,TEXT("LSR"),amIMPLIED,0,1},
	{0x4b,TEXT("ALR"),amIMMEDIATE,1,2},
	{0x4c,TEXT("JMP"),amABSOLUTE,0,3},
	{0x4d,TEXT("EOR"),amABSOLUTE,0,3},
	{0x4e,TEXT("LSR"),amABSOLUTE,0,3},
	{0x4f,TEXT("SRE"),amABSOLUTE,1,3},

	{0x50,TEXT("BVC"),amRELATIVE,0,2},
	{0x51,TEXT("EOR"),amINDIRECTY,0,2},
	{0x52,TEXT("HLT"),amIMPLIED,1,1},
	{0x53,TEXT("SRE"),amINDIRECTY,1,2},
	{0x54,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x55,TEXT("EOR"),amZEROPAGEX,0,2},
	{0x56,TEXT("LSR"),amZEROPAGEX,0,2},
	{0x57,TEXT("SRE"),amZEROPAGEX,1,2},

	{0x58,TEXT("CLI"),amIMPLIED,0,1},
	{0x59,TEXT("EOR"),amABSOLUTEY,0,3},
	{0x5a,TEXT("NOP"),amIMPLIED,1,1},
	{0x5b,TEXT("SRE"),amABSOLUTEY,1,3},
	{0x5c,TEXT("SKW"),amABSOLUTEX,1,3},
	{0x5d,TEXT("EOR"),amABSOLUTEX,0,3},
	{0x5e,TEXT("LSR"),amABSOLUTEX,0,3},
	{0x5f,TEXT("SRE"),amABSOLUTEX,1,3},


	{0x60,TEXT("RTS"),amIMPLIED,0,1},
	{0x61,TEXT("ADC"),amINDIRECTX,0,2},
	{0x62,TEXT("HLT"),amIMPLIED,1,1},
	{0x63,TEXT("RRA"),amINDIRECTX,1,2},
	{0x64,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x65,TEXT("ADC"),amZEROPAGE,0,2},
	{0x66,TEXT("ROR"),amZEROPAGE,0,2},
	{0x67,TEXT("RRA"),amZEROPAGE,1,2},

	{0x68,TEXT("PLA"),amIMPLIED,0,1},
	{0x69,TEXT("ADC"),amIMMEDIATE,0,2},
	{0x6a,TEXT("ROR"),amIMPLIED,0,1},
	{0x6b,TEXT("ARR"),amIMMEDIATE,1,2},
	{0x6c,TEXT("JMP"),amINDIRECT,0,2},
	{0x6d,TEXT("ADC"),amABSOLUTE,0,3},
	{0x6e,TEXT("ROR"),amABSOLUTE,0,3},
	{0x6f,TEXT("RRA"),amABSOLUTE,1,3},

	{0x70,TEXT("BVS"),amRELATIVE,0,2},
	{0x71,TEXT("ADC"),amINDIRECTY,0,2},
	{0x72,TEXT("HLT"),amIMPLIED,1,1},
	{0x73,TEXT("RRA"),amINDIRECTY,1,2},
	{0x74,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x75,TEXT("ADC"),amZEROPAGEX,0,2},
	{0x76,TEXT("ROR"),amZEROPAGEX,0,2},
	{0x77,TEXT("RRA"),amZEROPAGEX,1,2},

	{0x78,TEXT("SEI"),amIMPLIED,0,1},
	{0x79,TEXT("ADC"),amABSOLUTEY,0,3},
	{0x7a,TEXT("NOP"),amIMPLIED,1,1},
	{0x7b,TEXT("RRA"),amABSOLUTEY,1,3},
	{0x7c,TEXT("SKW"),amABSOLUTEX,1,3},
	{0x7d,TEXT("ADC"),amABSOLUTEX,0,3},
	{0x7e,TEXT("ROR"),amABSOLUTEX,0,3},
	{0x7f,TEXT("RRA"),amABSOLUTEX,1,3},


	{0x80,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x81,TEXT("STA"),amINDIRECTX,0,2},
	{0x82,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x83,TEXT("SAX"),amINDIRECTX,1,2},
	{0x84,TEXT("STY"),amZEROPAGE,0,2},
	{0x85,TEXT("STA"),amZEROPAGE,0,2},
	{0x86,TEXT("STX"),amZEROPAGE,0,2},
	{0x87,TEXT("SAX"),amZEROPAGE,1,2},

	{0x88,TEXT("DEY"),amIMPLIED,0,1},
	{0x89,TEXT("SKB"),amIMMEDIATE,1,2},
	{0x8a,TEXT("TXA"),amIMPLIED,0,1},
	{0x8b,TEXT("XAA"),amIMMEDIATE,1,2},
	{0x8c,TEXT("STY"),amABSOLUTE,0,3},
	{0x8d,TEXT("STA"),amABSOLUTE,0,3},
	{0x8e,TEXT("STX"),amABSOLUTE,0,3},
	{0x8f,TEXT("SAX"),amABSOLUTE,1,3},

	{0x90,TEXT("BCC"),amRELATIVE,0,2},
	{0x91,TEXT("STA"),amINDIRECTY,0,2},
	{0x92,TEXT("HLT"),amIMPLIED,1,1},
	{0x93,TEXT("AXA"),amINDIRECTY,1,2},
	{0x94,TEXT("STY"),amZEROPAGEX,0,2},
	{0x95,TEXT("STA"),amZEROPAGEX,0,2},
	{0x96,TEXT("STX"),amZEROPAGEY,0,2},
	{0x97,TEXT("SAX"),amZEROPAGEY,1,2},

	{0x98,TEXT("TYA"),amIMPLIED,0,1},
	{0x99,TEXT("STA"),amABSOLUTEY,0,3},
	{0x9a,TEXT("TXS"),amIMPLIED,0,1},
	{0x9b,TEXT("TAS"),amABSOLUTEY,1,3},
	{0x9c,TEXT("SAY"),amABSOLUTEX,1,3},
	{0x9d,TEXT("STA"),amABSOLUTEX,0,3},
	{0x9e,TEXT("XAS"),amABSOLUTEY,1,3},
	{0x9f,TEXT("AXA"),amABSOLUTEY,1,3},


	{0xA0,TEXT("LDY"),amIMMEDIATE,0,2},
	{0xA1,TEXT("LDA"),amINDIRECTX,0,2},
	{0xA2,TEXT("LDX"),amIMMEDIATE,0,2},
	{0xA3,TEXT("LAX"),amINDIRECTX,1,2},
	{0xA4,TEXT("LDY"),amZEROPAGE,0,2},
	{0xA5,TEXT("LDA"),amZEROPAGE,0,2},
	{0xA6,TEXT("LDX"),amZEROPAGE,0,2},
	{0xA7,TEXT("LAX"),amZEROPAGE,1,2},

	{0xA8,TEXT("TAY"),amIMPLIED,0,1},
	{0xA9,TEXT("LDA"),amIMMEDIATE,0,2},
	{0xAa,TEXT("TAX"),amIMPLIED,0,1},
	{0xAb,TEXT("OAL"),amIMMEDIATE,1,2},
	{0xAc,TEXT("LDY"),amABSOLUTE,0,3},
	{0xAd,TEXT("LDA"),amABSOLUTE,0,3},
	{0xAe,TEXT("LDX"),amABSOLUTE,0,3},
	{0xAf,TEXT("LAX"),amABSOLUTE,1,3},

	{0xB0,TEXT("BCS"),amRELATIVE,0,2},
	{0xB1,TEXT("LDA"),amINDIRECTY,0,2},
	{0xB2,TEXT("HLT"),amIMPLIED,1,1},
	{0xB3,TEXT("LAX"),amINDIRECTY,1,2},
	{0xB4,TEXT("LDY"),amZEROPAGEX,0,2},
	{0xB5,TEXT("LDA"),amZEROPAGEX,0,2},
	{0xB6,TEXT("LDX"),amZEROPAGEY,0,2},
	{0xB7,TEXT("LAX"),amZEROPAGEY,1,2},

	{0xB8,TEXT("CLV"),amIMPLIED,0,1},
	{0xB9,TEXT("LDA"),amABSOLUTEY,0,3},
	{0xBA,TEXT("TSX"),amIMPLIED,0,1},
	{0xBB,TEXT("LAS"),amABSOLUTEY,1,3},
	{0xBC,TEXT("LDY"),amABSOLUTEX,0,3},
	{0xBD,TEXT("LDA"),amABSOLUTEX,0,3},
	{0xBE,TEXT("LDX"),amABSOLUTEY,0,3},
	{0xBF,TEXT("LAX"),amABSOLUTEY,1,3},


	{0xC0,TEXT("CPY"),amIMMEDIATE,0,2},
	{0xC1,TEXT("CMP"),amINDIRECTX,0,2},
	{0xC2,TEXT("SKB"),amIMMEDIATE,1,2},
	{0xC3,TEXT("DCP"),amINDIRECTX,1,2},
	{0xC4,TEXT("CPY"),amZEROPAGE,0,2},
	{0xC5,TEXT("CMP"),amZEROPAGE,0,2},
	{0xC6,TEXT("DEC"),amZEROPAGE,0,2},
	{0xC7,TEXT("DCP"),amZEROPAGE,1,2},

	{0xC8,TEXT("INY"),amIMPLIED,0,1},
	{0xC9,TEXT("CMP"),amIMMEDIATE,0,2},
	{0xCA,TEXT("DEX"),amIMPLIED,0,1},
	{0xCB,TEXT("SBX"),amIMMEDIATE,1,2},
	{0xCC,TEXT("CPY"),amABSOLUTE,0,3},
	{0xCD,TEXT("CMP"),amABSOLUTE,0,3},
	{0xCE,TEXT("DEC"),amABSOLUTE,0,3},
	{0xCF,TEXT("DCP"),amABSOLUTE,1,3},

	{0xD0,TEXT("BNE"),amRELATIVE,0,2},
	{0xD1,TEXT("CMP"),amINDIRECTY,0,2},
	{0xD2,TEXT("HLT"),amIMPLIED,1,1},
	{0xD3,TEXT("DCP"),amINDIRECTY,1,2},
	{0xD4,TEXT("SKB"),amIMMEDIATE,1,2},
	{0xD5,TEXT("CMP"),amZEROPAGEX,0,2},
	{0xD6,TEXT("DEC"),amZEROPAGEX,0,2},
	{0xD7,TEXT("DCP"),amZEROPAGEX,1,2},

	{0xD8,TEXT("CLD"),amIMPLIED,0,1},
	{0xD9,TEXT("CMP"),amABSOLUTEY,0,3},
	{0xDA,TEXT("NOP"),amIMPLIED,1,1},
	{0xDB,TEXT("DCP"),amABSOLUTEY,1,3},
	{0xDC,TEXT("SKW"),amABSOLUTEX,1,3},
	{0xDD,TEXT("CMP"),amABSOLUTEX,0,3},
	{0xDE,TEXT("DEC"),amABSOLUTEX,0,3},
	{0xDF,TEXT("DCP"),amABSOLUTEX,1,3},


	{0xE0,TEXT("CPX"),amIMMEDIATE,0,2},
	{0xE1,TEXT("SBC"),amINDIRECTX,0,2},
	{0xE2,TEXT("SKB"),amIMMEDIATE,1,2},
	{0xE3,TEXT("ISB"),amINDIRECTX,1,2},
	{0xE4,TEXT("CPX"),amZEROPAGE,0,2},
	{0xE5,TEXT("SBC"),amZEROPAGE,0,2},
	{0xE6,TEXT("INC"),amZEROPAGE,0,2},
	{0xE7,TEXT("ISB"),amZEROPAGE,1,2},

	{0xE8,TEXT("INX"),amIMPLIED,0,1},
	{0xE9,TEXT("SBC"),amIMMEDIATE,0,2},
	{0xEA,TEXT("NOP"),amIMPLIED,0,1},
	{0xEB,TEXT("SBC"),amIMMEDIATE,1,2},
	{0xEC,TEXT("CPX"),amABSOLUTE,0,3},
	{0xED,TEXT("SBC"),amABSOLUTE,0,3},
	{0xEE,TEXT("INC"),amABSOLUTE,0,3},
	{0xEF,TEXT("ISB"),amABSOLUTE,1,3},

	{0xF0,TEXT("BEQ"),amRELATIVE,0,2},
	{0xF1,TEXT("SBC"),amINDIRECTY,0,2},
	{0xF2,TEXT("HLT"),amIMPLIED,1,1},
	{0xF3,TEXT("ISB"),amINDIRECTY,1,2},
	{0xF4,TEXT("SKB"),amIMMEDIATE,1,2},
	{0xF5,TEXT("SBC"),amZEROPAGEX,0,2},
	{0xF6,TEXT("INC"),amZEROPAGEX,0,2},
	{0xF7,TEXT("ISB"),amZEROPAGEX,1,2},

	{0xF8,TEXT("SED"),amIMPLIED,0,1},
	{0xF9,TEXT("SBC"),amABSOLUTEY,0,3},
	{0xFA,TEXT("NOP"),amIMPLIED,1,1},
	{0xFB,TEXT("ISB"),amABSOLUTEY,1,3},
	{0xFC,TEXT("SKW"),amABSOLUTEX,1,3},
	{0xFD,TEXT("SBC"),amABSOLUTEX,0,3},
	{0xFE,TEXT("INC"),amABSOLUTEX,0,3},
	{0xFF,TEXT("ISB"),amABSOLUTEX,1,3},
};

CPU6502::CPU6502()
{
	m_bDebug=0;
	InitDecoder();
	CurrentClock=0;
	FirstIRQClock=0;
	FirstNMIClock=0;
	RisingIRQClock=0;
	FirstBALowClock=0;
	LastBAHighClock=0;
	BA = 1;
	SOTrigger = false;
	SOTriggerClock = 0;
	m_bBALowInClock2OfSEI = false;
	m_bBreakOnInterruptTaken = false;
}

CPU6502::~CPU6502()
{
}

void CPU6502::InitDecoder()
{
int i;
	for (i=0; i<=255; i++)
		decode_array[i]=C_ILLEGAL;

	decode_array[BRK_IMPLIED]=C_BRK_IMPLIED;
	
	decode_array[ORA_IMMEDIATE]=ORA_IMMEDIATE;
	decode_array[ORA_ZEROPAGE]=C_ZEROPAGE;
	decode_array[ORA_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[ORA_ABSOLUTE]=C_ABSOLUTE;
	decode_array[ORA_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[ORA_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[ORA_INDIRECTX]=C_INDIRECTX;
	decode_array[ORA_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[AND_IMMEDIATE]=AND_IMMEDIATE;
	decode_array[AND_ZEROPAGE]=C_ZEROPAGE;
	decode_array[AND_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[AND_ABSOLUTE]=C_ABSOLUTE;
	decode_array[AND_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[AND_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[AND_INDIRECTX]=C_INDIRECTX;
	decode_array[AND_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[EOR_IMMEDIATE]=EOR_IMMEDIATE;
	decode_array[EOR_ZEROPAGE]=C_ZEROPAGE;
	decode_array[EOR_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[EOR_ABSOLUTE]=C_ABSOLUTE;
	decode_array[EOR_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[EOR_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[EOR_INDIRECTX]=C_INDIRECTX;
	decode_array[EOR_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[ADC_IMMEDIATE]=ADC_IMMEDIATE;
	decode_array[ADC_ZEROPAGE]=C_ZEROPAGE;
	decode_array[ADC_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[ADC_ABSOLUTE]=C_ABSOLUTE;
	decode_array[ADC_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[ADC_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[ADC_INDIRECTX]=C_INDIRECTX;
	decode_array[ADC_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[SBC_IMMEDIATE]=SBC_IMMEDIATE;
	decode_array[SBC_IMMEDIATE_EB]=SBC_IMMEDIATE;
	decode_array[SBC_ZEROPAGE]=C_ZEROPAGE;
	decode_array[SBC_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[SBC_ABSOLUTE]=C_ABSOLUTE;
	decode_array[SBC_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[SBC_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[SBC_INDIRECTX]=C_INDIRECTX;
	decode_array[SBC_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[CMP_IMMEDIATE]=CMP_IMMEDIATE;
	decode_array[CMP_ZEROPAGE]=C_ZEROPAGE;
	decode_array[CMP_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[CMP_ABSOLUTE]=C_ABSOLUTE;
	decode_array[CMP_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[CMP_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[CMP_INDIRECTX]=C_INDIRECTX;
	decode_array[CMP_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[CPX_IMMEDIATE]=CPX_IMMEDIATE;
	decode_array[CPX_ZEROPAGE]=C_ZEROPAGE;
	decode_array[CPX_ABSOLUTE]=C_ABSOLUTE;

	decode_array[CPY_IMMEDIATE]=CPY_IMMEDIATE;
	decode_array[CPY_ZEROPAGE]=C_ZEROPAGE;
	decode_array[CPY_ABSOLUTE]=C_ABSOLUTE;

	decode_array[DEC_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[DEC_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[DEC_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[DEC_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;

	decode_array[DEX_IMPLIED]=DEX_IMPLIED;

	decode_array[DEY_IMPLIED]=DEY_IMPLIED;

	decode_array[INX_IMPLIED]=INX_IMPLIED;

	decode_array[INY_IMPLIED]=INY_IMPLIED;

	decode_array[INC_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[INC_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[INC_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[INC_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;

	decode_array[LDA_IMMEDIATE]=LDA_IMMEDIATE;
	decode_array[LDA_ZEROPAGE]=C_ZEROPAGE;
	decode_array[LDA_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[LDA_ABSOLUTE]=C_ABSOLUTE;
	decode_array[LDA_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[LDA_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[LDA_INDIRECTX]=C_INDIRECTX;
	decode_array[LDA_INDIRECTY]=C_INDIRECTY_READ;

	decode_array[LDX_IMMEDIATE]=LDX_IMMEDIATE;
	decode_array[LDX_ZEROPAGE]=C_ZEROPAGE;
	decode_array[LDX_ZEROPAGEY]=C_ZEROPAGEY;
	decode_array[LDX_ABSOLUTE]=C_ABSOLUTE;
	decode_array[LDX_ABSOLUTEY]=C_ABSOLUTEY_READ;

	decode_array[LDY_IMMEDIATE]=LDY_IMMEDIATE;
	decode_array[LDY_ZEROPAGE]=C_ZEROPAGE;
	decode_array[LDY_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[LDY_ABSOLUTE]=C_ABSOLUTE;
	decode_array[LDY_ABSOLUTEX]=C_ABSOLUTEX_READ;

	decode_array[LSR_IMPLIED]=LSR_IMPLIED;
	decode_array[LSR_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[LSR_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[LSR_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[LSR_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;

	decode_array[ASL_IMPLIED]=ASL_IMPLIED;
	decode_array[ASL_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[ASL_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[ASL_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[ASL_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;

	decode_array[NOP_IMPLIED]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_1A]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_3A]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_5A]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_7A]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_DA]=NOP_IMPLIED;
	decode_array[NOP_IMPLIED_FA]=NOP_IMPLIED;

	decode_array[ROL_IMPLIED]=ROL_IMPLIED;
	decode_array[ROL_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[ROL_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[ROL_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[ROL_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	
	decode_array[ROR_IMPLIED]=ROR_IMPLIED;
	decode_array[ROR_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[ROR_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[ROR_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[ROR_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;

	decode_array[STA_ZEROPAGE]=C_ZEROPAGE;
	decode_array[STA_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[STA_ABSOLUTE]=C_ABSOLUTE;
	decode_array[STA_ABSOLUTEX]=C_ABSOLUTEX_WRITE;
	decode_array[STA_ABSOLUTEY]=C_ABSOLUTEY_WRITE;
	decode_array[STA_INDIRECTX]=C_INDIRECTX;
	decode_array[STA_INDIRECTY]=C_INDIRECTY_WRITE;

	decode_array[STX_ZEROPAGE]=C_ZEROPAGE;
	decode_array[STX_ZEROPAGEY]=C_ZEROPAGEY;
	decode_array[STX_ABSOLUTE]=C_ABSOLUTE;

	decode_array[STY_ZEROPAGE]=C_ZEROPAGE;
	decode_array[STY_ZEROPAGEX]=C_ZEROPAGEX;
	decode_array[STY_ABSOLUTE]=C_ABSOLUTE;

	decode_array[TAX_IMPLIED]=TAX_IMPLIED;
	decode_array[TAY_IMPLIED]=TAY_IMPLIED;
	decode_array[TXA_IMPLIED]=TXA_IMPLIED;
	decode_array[TYA_IMPLIED]=TYA_IMPLIED;
	decode_array[TSX_IMPLIED]=TSX_IMPLIED;
	decode_array[TXS_IMPLIED]=TXS_IMPLIED;

	decode_array[JMP_ABSOLUTE]=C_JMP_ABSOLUTE;
	decode_array[JMP_INDIRECT]=C_JMP_INDIRECT;
	decode_array[JSR_ABSOLUTE]=C_JSR_ABSOLUTE;
	decode_array[RTS_IMPLIED]=C_RTS;
	decode_array[RTI_IMPLIED]=C_RTI;
	decode_array[PHA_IMPLIED]=C_PHA_IMPLIED;
	decode_array[PHP_IMPLIED]=C_PHP_IMPLIED;
	decode_array[PLA_IMPLIED]=C_PLA_IMPLIED;
	decode_array[PLP_IMPLIED]=C_PLP_IMPLIED;

	decode_array[BEQ_RELATIVE]=BEQ_RELATIVE;
	decode_array[BNE_RELATIVE]=BNE_RELATIVE;
	decode_array[BMI_RELATIVE]=BMI_RELATIVE;
	decode_array[BPL_RELATIVE]=BPL_RELATIVE;
	decode_array[BCC_RELATIVE]=BCC_RELATIVE;
	decode_array[BCS_RELATIVE]=BCS_RELATIVE;
	decode_array[BVC_RELATIVE]=BVC_RELATIVE;
	decode_array[BVS_RELATIVE]=BVS_RELATIVE;		

	decode_array[CLC_IMPLIED]=CLC_IMPLIED;
	decode_array[SEC_IMPLIED]=SEC_IMPLIED;
	decode_array[CLI_IMPLIED]=CLI_IMPLIED;
	decode_array[SEI_IMPLIED]=SEI_IMPLIED;
	decode_array[CLD_IMPLIED]=CLD_IMPLIED;
	decode_array[SED_IMPLIED]=SED_IMPLIED;
	decode_array[CLV_IMPLIED]=CLV_IMPLIED;

	decode_array[BIT_ZEROPAGE]=C_ZEROPAGE;
	decode_array[BIT_ABSOLUTE]=C_ABSOLUTE;

	decode_array[SKB_IMMEDIATE]=C_SKB_IMMEDIATE_3CLK;
	decode_array[SKB_IMMEDIATE_80]=C_SKB_IMMEDIATE_2CLK;
	decode_array[SKB_IMMEDIATE_82]=C_SKB_IMMEDIATE_2CLK;
	decode_array[SKB_IMMEDIATE_C2]=C_SKB_IMMEDIATE_2CLK;
	decode_array[SKB_IMMEDIATE_E2]=C_SKB_IMMEDIATE_2CLK;
	decode_array[SKB_IMMEDIATE_14]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_34]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_44]=C_SKB_IMMEDIATE_3CLK;
	decode_array[SKB_IMMEDIATE_54]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_64]=C_SKB_IMMEDIATE_3CLK;
	decode_array[SKB_IMMEDIATE_74]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_D4]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_F4]=C_SKB_IMMEDIATE_4CLK;
	decode_array[SKB_IMMEDIATE_89]=C_SKB_IMMEDIATE_2CLK;

	decode_array[SKW_ABSOLUTE]=C_ABSOLUTE;
	decode_array[SKW_ABSOLUTEX]=C_ABSOLUTEX_READ;
	decode_array[SKW_ABSOLUTEX_3C]=C_ABSOLUTEX_READ;
	decode_array[SKW_ABSOLUTEX_5C]=C_ABSOLUTEX_READ;
	decode_array[SKW_ABSOLUTEX_7C]=C_ABSOLUTEX_READ;
	decode_array[SKW_ABSOLUTEX_DC]=C_ABSOLUTEX_READ;
	decode_array[SKW_ABSOLUTEX_FC]=C_ABSOLUTEX_READ;

	decode_array[HLT_IMPLIED]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_12]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_22]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_32]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_42]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_52]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_62]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_72]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_92]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_B2]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_D2]=HLT_IMPLIED;
	decode_array[HLT_IMPLIED_F2]=HLT_IMPLIED;

	decode_array[ANC_IMMEDIATE]=ANC_IMMEDIATE;
	decode_array[ANC_IMMEDIATE_2B]=ANC_IMMEDIATE;

	decode_array[SLO_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[SLO_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[SLO_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[SLO_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[SLO_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[SLO_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[SLO_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[RLA_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[RLA_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[RLA_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[RLA_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[RLA_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[RLA_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[RLA_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[SRE_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[SRE_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[SRE_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[SRE_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[SRE_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[SRE_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[SRE_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[RRA_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[RRA_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[RRA_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[RRA_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[RRA_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[RRA_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[RRA_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[SAX_ZEROPAGE]=C_ZEROPAGE_WRITE;
	decode_array[SAX_ZEROPAGEY]=C_ZEROPAGEY_WRITE;
	decode_array[SAX_ABSOLUTE]=C_ABSOLUTE_WRITE;
	decode_array[SAX_INDIRECTX]=C_INDIRECTX_WRITE;

	decode_array[RRA_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[RRA_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[RRA_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[RRA_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[RRA_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[RRA_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[RRA_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[LAX_ZEROPAGE]=C_ZEROPAGE_READ;
	decode_array[LAX_ZEROPAGEY]=C_ZEROPAGEY_READ;
	decode_array[LAX_ABSOLUTE]=C_ABSOLUTE_READ;
	decode_array[LAX_ABSOLUTEY]=C_ABSOLUTEY_READ;
	decode_array[LAX_INDIRECTY]=C_INDIRECTY_READ;
	decode_array[LAX_INDIRECTX]=C_INDIRECTX_READ;

	decode_array[DCP_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[DCP_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[DCP_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[DCP_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[DCP_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[DCP_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[DCP_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[ISB_ZEROPAGE]=C_ZEROPAGE_READWRITE;
	decode_array[ISB_ZEROPAGEX]=C_ZEROPAGEX_READWRITE;
	decode_array[ISB_ABSOLUTE]=C_ABSOLUTE_READWRITE;
	decode_array[ISB_ABSOLUTEY]=C_ABSOLUTEY_READWRITE;
	decode_array[ISB_ABSOLUTEX]=C_ABSOLUTEX_READWRITE;
	decode_array[ISB_INDIRECTY]=C_INDIRECTY_READWRITE;
	decode_array[ISB_INDIRECTX]=C_INDIRECTX_READWRITE;

	decode_array[ALR_IMMEDIATE]=ALR_IMMEDIATE;

	decode_array[ARR_IMMEDIATE]=ARR_IMMEDIATE;

	decode_array[XAA_IMMEDIATE]=XAA_IMMEDIATE;

	decode_array[OAL_IMMEDIATE]=OAL_IMMEDIATE;

	decode_array[SBX_IMMEDIATE]=SBX_IMMEDIATE;

	decode_array[TAS_ABSOLUTEY]=C_ABSOLUTEY_WRITE;

	decode_array[SAY_ABSOLUTEX]=C_ABSOLUTEX_WRITE;

	decode_array[XAS_ABSOLUTEY]=C_ABSOLUTEY_WRITE;

	decode_array[AXA_ABSOLUTEY]=C_ABSOLUTEY_WRITE;
	decode_array[AXA_INDIRECTY]=C_INDIRECTY_WRITE;

	decode_array[ANC_IMMEDIATE]=ANC_IMMEDIATE;

	decode_array[LAS_ABSOLUTEY]=C_ABSOLUTEY_READ;
}

void CPU6502::Cleanup()
{
}

int CPU6502::GetCpuId()
{
	return ID;
}

void CPU6502::GetCpuState(CPUState& state)
{
	state.A = mA;
	state.X = mX;
	state.Y = mY;
	state.PC = mPC.word;
	state.PC_CurrentOpcode = m_CurrentOpcodeAddress.word;
	state.Flags = (fNEGATIVE <<7) | (fOVERFLOW <<6) | (1<<5) | (fBREAK <<4) | (fDECIMAL <<3) | (fINTERRUPT <<2) | (fZERO <<1) | (fCARRY);
	state.SP = mSP;
	state.processor_interrupt = PROCESSOR_INTERRUPT;
	state.cpu_sequence = m_cpu_sequence;
	state.clock = CurrentClock;
	state.BA = BA;
	state.cycle = (int)(CurrentClock - m_CurrentOpcodeClock);
	state.IsInterruptInstruction = IsInterruptInstruction();
}

void CPU6502::SetBALow(ICLK sysclock)
{
	if (BA != 0)
	{
		FirstBALowClock = sysclock;
		if (this->CurrentClock == sysclock)
		{
			if (m_cpu_sequence == CLI_IMPLIED)
			{
				//If BA transitions to low in the second cycle of CLI then the IRQ check is performed as though the I flag was clear at the start of CLI.
				fINTERRUPT = 0;
			}
			else if (m_cpu_sequence == SEI_IMPLIED)
			{
				//If the CPU is in run ahead mode, we do not know if the CIA is going to want to back date an IRQ.
				//If both BA and IRQ transition to low in the second cycle of SEI then no IRQ occurs in the next instruction.
				m_bBALowInClock2OfSEI = true;
			}
		}
		BA = 0;
	}
	else if (((ICLKS)(FirstBALowClock - sysclock)) > 0)
		FirstBALowClock = sysclock;
}

void CPU6502::SetBAHigh(ICLK sysclock)
{
	if (BA == 0)
	{
		LastBAHighClock = sysclock;
		BA = 1;
	}
}

void CPU6502::SetIRQ(ICLK sysclock)
{
	if (IRQ==0)
	{
		FirstIRQClock = sysclock;
	}
	else if (((ICLKS)(FirstIRQClock - sysclock)) > 0)
		FirstIRQClock = sysclock;

	IRQ = 1;
}
void CPU6502::ClearIRQ()
{
	IRQ = 0;
}

void CPU6502::ClearSlowIRQ()
{
	if (IRQ!=0)
		RisingIRQClock = CurrentClock + 1;
	IRQ = 0;
}

void CPU6502::SetNMI(ICLK sysclock)
{
	if (NMI==0)
	{
		FirstNMIClock = sysclock;
		NMI_TRIGGER = 1;
	}
	else if (((ICLKS)(FirstNMIClock - sysclock)) > 0)
		FirstNMIClock = sysclock;
	NMI=1;
}
void CPU6502::ClearNMI()
{
	NMI=0;
}

void CPU6502::ConfigureMemoryMap()
{
}

bit8 CPU6502::code_arr(unsigned int _a, unsigned int _s)
{
unsigned int _al;
unsigned int _ah;
unsigned int _r;

	SyncVFlag();
	if (fDECIMAL==0)
	{
		_a = _a & _s;
		_a >>= 1;
		_a |= (fCARRY ? 0x80 : 0);
		fNEGATIVE = fCARRY;
		fZERO = (_a==0);
		fCARRY = (_a & 0x40) >> 6;
		fOVERFLOW = fCARRY ^ ((_a & 0x20) >> 5);
		return _a & 0xff;
	}
	else
	{
		_a = _a & _s;
		_al = _a >> 4;
		_ah = _a & 15;
		fNEGATIVE = fCARRY;
		_r = (_a >> 1);
		_r |= (fCARRY ? 0x80 : 0);
		fZERO = (_r==0);
		fOVERFLOW = ((_r ^ _a) & 0x40) >> 6;

		if ((_al + (_al & 1)) > 5)
			_r = (_r & 0xf0) | ((_r + 6) & 0xf);
		if (fCARRY = (_ah + (_ah & 1) > 5))
			_r = (_r + 0x60) & 0xff;

		return _r & 0xff;
	}	
}

bit8 CPU6502::code_cmp(unsigned int _a, unsigned int _s)
{
unsigned int _r;
	_r=_a - _s;
	_a=_r & 0xFF;
	fCARRY=!(_r>>8);
	fZERO=(_a==0);
	fNEGATIVE=(_a & 0x80) >> 7;
	return _a;
}

bit8 CPU6502::code_add(unsigned int _a, unsigned int _s)
{
unsigned int _al;
unsigned int _ah;
unsigned int _r;

	SyncVFlag();
	if (fDECIMAL==0){
		_r=_a + _s + fCARRY;
		fOVERFLOW=(~(_a^_s) & (_a^_r) & 0x80) >> 7;
		_a=_r & 0xFF;
		fCARRY=_r>>8;
		fZERO=(_a==0);
		fNEGATIVE=(_a & 0x80) >> 7;
	}
	else{
		_al=(_a & 15) + (_s & 15) + fCARRY;
		_ah=(_a >> 4) + (_s >> 4) + (_al > 9);
		if (_al > 9) _al += 6;
		fZERO=((_a + _s + fCARRY) & 0xFF) == 0;
		fNEGATIVE=(_ah & 0x08) >> 3;
		fOVERFLOW = (((_ah << 4) ^ _a) & 0x80 & ~((_a ^ _s) & 0x80)) >> 7;
		if (_ah > 9) _ah += 6;

		fCARRY = (_ah > 0x0F);
		_a = ((_ah << 4) | (_al & 0x0F)) & 0xFF;
	}
	return _a;
}

bit8 CPU6502::code_sub(unsigned int _a, unsigned int _s)
{
unsigned int _al;
unsigned int _ah;
unsigned int _r;

	SyncVFlag();
	if (fDECIMAL==0){
		_r=_a - _s - !fCARRY;
		fOVERFLOW=(((_a^_s) & (_a^_r)) & 0x80) >> 7;
		_a=_r & 0xFF;
		fCARRY=!(_r>>8);
		fZERO=(_a==0);
		fNEGATIVE=(_a & 0x80) >> 7;
	}
	else{
		_al = (_a & 0x0F) - (_s & 0x0F) - !fCARRY;
		if (_al & 0x10) _al -= 6;
		_ah = (_a >> 4) - (_s >> 4) - ((_al & 0x30)!=0);
		if ((_ah & 0x30)!=0) _ah -= 6;
		
		_r=(_a - _s - !fCARRY);

		fZERO = ((_r) & 255) == 0;
		fNEGATIVE = ((_r) & 128) != 0;
		fCARRY = ((_r) & 256) == 0;
		fOVERFLOW =( (_a ^ _s ^ _r) & 128)>> 7 ^ !fCARRY;

		_a = ((_ah << 4) | (_al & 0x0F)) & 0xFF;
	}
	return _a;
}


void CPU6502::InitReset(ICLK sysclock)
{
	CurrentClock = sysclock;
	BA=1;
	IRQ=0;	
	NMI=0;
	NMI_TRIGGER=0;
	FirstIRQClock=0;
	FirstNMIClock=0;
	RisingIRQClock = sysclock - 1;
	m_bBALowInClock2OfSEI = false;
	mPC.word=0x00ff;
	mA=0xaa;
	mX=0;
	mY=0;
	mSP=0;
	fNEGATIVE=0;
	fOVERFLOW=0;
	fBREAK=0;
	fDECIMAL=0;
	fINTERRUPT=0;
	fZERO=1;
	fCARRY=0;
	SOTrigger = false;
	SOTriggerClock = 0;
	m_cpu_sequence=C_RESET;
	PROCESSOR_INTERRUPT=0;	
	m_CurrentOpcodeAddress = mPC;
	m_CurrentOpcodeClock = sysclock;
}

void CPU6502::Reset(ICLK sysclock)
{
	InitReset(sysclock);
}

HRESULT CPU6502::Init(int ID, IBreakpointManager *pIBreakpointManager)
{	
	this->ID = ID;
	this->m_pIBreakpointManager = pIBreakpointManager;
	return S_OK;
}

ICLK CPU6502::GetCurrentClock()
{
	return CurrentClock;
}

void CPU6502::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CurrentClock = sysclock;
	FirstIRQClock += v;
	FirstNMIClock += v;
	RisingIRQClock += v;
	FirstBALowClock += v;
	LastBAHighClock += v;
	SOTriggerClock += v;
}

void CPU6502::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;

	if ((ICLKS)(CurrentClock - FirstIRQClock) >= CLOCKSYNCBAND_FAR)
		FirstIRQClock = ClockBehindNear;
	if ((ICLKS)(CurrentClock - FirstNMIClock) >= CLOCKSYNCBAND_FAR)
		FirstNMIClock = ClockBehindNear;
	if ((ICLKS)(CurrentClock - RisingIRQClock) >= CLOCKSYNCBAND_FAR)
		RisingIRQClock = ClockBehindNear;	
	if ((ICLKS)(CurrentClock - FirstBALowClock) >= CLOCKSYNCBAND_FAR)
		FirstBALowClock = ClockBehindNear;	
	if ((ICLKS)(CurrentClock - LastBAHighClock) >= CLOCKSYNCBAND_FAR)
		LastBAHighClock = ClockBehindNear;	
	
	if ((ICLKS)(CurrentClock - SOTriggerClock) >= CLOCKSYNCBAND_FAR)
		SOTriggerClock = ClockBehindNear;
}

void CPU6502::check_interrupts1()
{
ICLK currClock = CurrentClock;

	if (fINTERRUPT==0 && (IRQ!=0 || (ICLKS)(currClock - RisingIRQClock) <= 0) && (ICLKS)(currClock - FirstIRQClock) >= 1)
	{
		PROCESSOR_INTERRUPT|=INT_IRQ;
	}
	if (NMI_TRIGGER && (ICLKS)(currClock - FirstNMIClock) >= 1)
	{
  		NMI_TRIGGER=0;
		PROCESSOR_INTERRUPT|=INT_NMI;
	}
}

void CPU6502::check_interrupts0()
{
ICLK currClock = CurrentClock;
	if (fINTERRUPT==0 && (IRQ!=0 || (ICLKS)(currClock - RisingIRQClock) <= 0) && (ICLKS)(currClock - FirstIRQClock) >= 0)
	{
		PROCESSOR_INTERRUPT|=INT_IRQ;
	}
	if (NMI_TRIGGER && (ICLKS)(currClock - FirstIRQClock) >= 0)
	{
  		NMI_TRIGGER=0;
		PROCESSOR_INTERRUPT|=INT_NMI;
	}
}

void CPU6502::SyncVFlag()
{
}

void CPU6502::CheckForCartFreeze()
{
}

bool CPU6502::IsOpcodeFetch()
{
	return m_cpu_sequence == C_FETCH_OPCODE;
}

bool CPU6502::IsInterruptInstruction()
{
	switch(m_cpu_sequence)
	{
	case C_FETCH_OPCODE:
		if ((PROCESSOR_INTERRUPT & (INT_NMI | INT_IRQ))!=0)
			return true;
		break;
	case C_IRQ:
	case C_IRQ_2:
	case C_IRQ_3:
	case C_IRQ_4:
	case C_NMI:
	case C_NMI_2:
	case C_NMI_3:
	case C_NMI_4:
	case C_BRK_IMPLIED:
	case C_BRK_IMPLIED_2:
	case C_BRK_IMPLIED_3:
	case C_BRK_IMPLIED_4:
	case C_LOAD_PC:
	case C_LOAD_PC_2:
		return true;
	}
	return false;
}

#define CHECK_BA if (m_bDebug && BA==0) {++m_CurrentOpcodeClock;break;}

#define BA_AND_DEBUG (m_bDebug && BA==0)

void CPU6502::ExecuteCycle(ICLK sysclock)
{
unsigned int v;
	//The value of CurrentClock can be advanced by vic.ExecuteCycle()
	while ((ICLKS)(sysclock - CurrentClock) > 0)
	{
		CurrentClock++;
		switch (m_cpu_sequence)
		{
		case C_FETCH_OPCODE:
			m_op_code=ReadByte(mPC.word);
			CHECK_BA;
			if (PROCESSOR_INTERRUPT==0)
			{
				mPC.word++;
				m_cpu_sequence=decode_array[m_op_code];
				m_cpu_final_sequence=m_op_code;
			}
			else if(PROCESSOR_INTERRUPT & INT_NMI)
			{
				PROCESSOR_INTERRUPT=0;
				m_cpu_sequence=C_NMI;
			}
			else if((PROCESSOR_INTERRUPT & INT_IRQ))
			{
				PROCESSOR_INTERRUPT=0;
				m_cpu_sequence=C_IRQ;
			}
			break;
		case C_IRQ:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_IRQ_2:
			CPU6502_PUSH_PCH;
			m_cpu_sequence++;
			break;
		case C_IRQ_3:
			CPU6502_PUSH_PCL;
			m_cpu_sequence++;
			check_interrupts0();
			break;
		case C_IRQ_4:
			if (PROCESSOR_INTERRUPT & INT_NMI) 
			{
				PROCESSOR_INTERRUPT = 0;
				SyncVFlag();
				CPU6502_PUSH_PS_NO_B;
				fINTERRUPT=1;
				addr.word=0xFFFA;
				m_cpu_sequence=C_LOAD_PC;
			}
			else
			{
				PROCESSOR_INTERRUPT = 0;
				SyncVFlag();
 				CPU6502_PUSH_PS_NO_B;
				fINTERRUPT=1;
				addr.word=0xFFFE;
				m_cpu_sequence=C_LOAD_PC;
			}
			CheckForCartFreeze();
			break;
		case C_NMI:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_NMI_2:
			CPU6502_PUSH_PCH;
			m_cpu_sequence++;
			break;
		case C_NMI_3:
			CPU6502_PUSH_PCL;
			m_cpu_sequence++;
			break;
		case C_NMI_4:
			SyncVFlag();
			CPU6502_PUSH_PS_NO_B;
			fINTERRUPT=1;
			PROCESSOR_INTERRUPT=0;
			addr.word=0xFFFA;
			m_cpu_sequence=C_LOAD_PC;
			CheckForCartFreeze();
			break;
		case C_BRK_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_BRK_IMPLIED_2:
			CPU6502_PUSH_PCH;
			m_cpu_sequence++;
			break;
		case C_BRK_IMPLIED_3:
			CPU6502_PUSH_PCL;
			m_cpu_sequence++;
			check_interrupts0();
			break;
		case C_BRK_IMPLIED_4:
			if (PROCESSOR_INTERRUPT & INT_NMI) 
			{
				PROCESSOR_INTERRUPT = 0;
				fBREAK=1;
				SyncVFlag();
				CPU6502_PUSH_PS_B;
				fINTERRUPT=1;
				addr.word=0xFFFA;
				m_cpu_sequence=C_LOAD_PC;
			}
			else if (PROCESSOR_INTERRUPT & INT_IRQ)
			{				
				PROCESSOR_INTERRUPT = 0;
				fBREAK=1;
				SyncVFlag();
				CPU6502_PUSH_PS_B;
				fINTERRUPT=1;
				addr.word=0xFFFE;
				m_cpu_sequence=C_LOAD_PC;
			}
			else
			{
				PROCESSOR_INTERRUPT = 0;
				fBREAK=1;
				SyncVFlag();
				CPU6502_PUSH_PS_B;
				fINTERRUPT=1;
				addr.word=0xFFFE;
				m_cpu_sequence=C_LOAD_PC;
			}
			CheckForCartFreeze();
			break;
		case C_LOAD_PC:
			CPU6502_LOAD_PCL(ReadByte(addr.word));
			CHECK_BA;
			addr.word++;
			m_cpu_sequence++;
			break;
		case C_LOAD_PC_2:
			CPU6502_LOAD_PCH(ReadByte(addr.word));
			CHECK_BA;

			//A check_interrupts1() here does fail Wolfgang's NMI test
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_RESET:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_RESET_1:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_RESET_2:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_RESET_3:
			ReadByte(mSP + 0x0100);
			CHECK_BA;
			mSP--;
			m_cpu_sequence++;
			break;
		case C_RESET_4:
			ReadByte(mSP + 0x0100);
			CHECK_BA;
			mSP--;
			m_cpu_sequence++;
			break;
		case C_RESET_5:
			ReadByte(mSP + 0x0100);
			CHECK_BA;
			mSP--;
			fBREAK = 1;
			fINTERRUPT = 1;
			addr.word=0xFFFC;
			m_cpu_sequence++;
			break;
		case C_RESET_6:
			CPU6502_LOAD_PCL(ReadByte(addr.word));
			CHECK_BA;
			addr.word++;
			m_cpu_sequence++;
			break;
		case C_RESET_7:
			CPU6502_LOAD_PCH(ReadByte(addr.word));
			CHECK_BA;

			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_INDIRECTX:
			ptr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_2:
			ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte+=mX;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_3:
			addr.byte.loByte=ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_4:
			addr.byte.hiByte=ReadByte(ptr.word);
			CHECK_BA;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_INDIRECTX_READWRITE:
			ptr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_READWRITE_2:
			ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte+=mX;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_READWRITE_3:
			addr.byte.loByte=ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_READWRITE_4:
			addr.byte.hiByte=ReadByte(ptr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_READWRITE_5:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_INDIRECTX_READWRITE_6:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_INDIRECTY_READ:
			ptr.word= ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READ_2:
			addr.byte.loByte= ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READ_3:
			addr.byte.hiByte= ReadByte(ptr.word);
			CHECK_BA;
			addr.byte.loByte+=mY;
			if (addr.byte.loByte < mY)
				m_cpu_sequence++;
			else
			{
				m_cpu_sequence=m_cpu_final_sequence;
			}
			break;
		case C_INDIRECTY_READ_4:
			ReadByte(addr.word);
			CHECK_BA;
			addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_INDIRECTY_WRITE:
			ptr.word= ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_WRITE_2:
			addr.byte.loByte= ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_WRITE_3:
			addr.byte.hiByte= ReadByte(ptr.word);
			CHECK_BA;
			addr.byte.loByte+=mY;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_WRITE_4:
			ReadByte(addr.word);
			CHECK_BA;
			if (addr.byte.loByte < mY)
				addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_INDIRECTY_READWRITE:
			ptr.word= ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READWRITE_2:
			addr.byte.loByte= ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READWRITE_3:
			addr.byte.hiByte= ReadByte(ptr.word);
			CHECK_BA;
			addr.byte.loByte+=mY;
			m_cpu_sequence++;
			
		//step 4 is not needed here:- case C_INDIRECTY_READWRITE_4:
			//CHECK_BA;
			//ReadByte(addr.word);
			//FIXME remove C_INDIRECTY_READWRITE_4 and re-number the cpu sequence case constants
			if (addr.byte.loByte < mY)
				addr.byte.hiByte++;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READWRITE_5:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_INDIRECTY_READWRITE_6:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ZEROPAGE:
			addr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ZEROPAGE_READWRITE:
			addr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGE_READWRITE_2:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGE_READWRITE_3:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ZEROPAGEX:
			addr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGEX_2:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			addr.byte.loByte+=mX;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ZEROPAGEY:
			addr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGEY_2:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			addr.byte.loByte+=mY;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ZEROPAGEX_READWRITE:
			addr.word=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGEX_READWRITE_2:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			addr.byte.loByte+=mX;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGEX_READWRITE_3:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_ZEROPAGEX_READWRITE_4:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_JMP_ABSOLUTE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_JMP_ABSOLUTE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word=addr.word;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_JSR_ABSOLUTE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_JSR_ABSOLUTE_2:
			ReadByte(mSP + 0x0100);//Read from stack and throw away.
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_JSR_ABSOLUTE_3:
			CPU6502_PUSH_PCH;
			m_cpu_sequence++;
			break;
		case C_JSR_ABSOLUTE_4:
			CPU6502_PUSH_PCL;
			m_cpu_sequence++;
			break;
		case C_JSR_ABSOLUTE_5:
			addr.byte.hiByte= ReadByte(mPC.word);
			CHECK_BA;
			mPC = addr;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_JMP_INDIRECT:
			ptr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_JMP_INDIRECT_2:
			ptr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_JMP_INDIRECT_3:
			addr.byte.loByte=ReadByte(ptr.word);
			CHECK_BA;
			ptr.byte.loByte++;
			mPC.byte.loByte=addr.byte.loByte;
			m_cpu_sequence++;
			break;
		case C_JMP_INDIRECT_4:
			mPC.byte.hiByte=ReadByte(ptr.word);
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_ABSOLUTE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ABSOLUTE_READWRITE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTE_READWRITE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTE_READWRITE_3:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTE_READWRITE_4:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ABSOLUTEX_READ:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEX_READ_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mX;
			if (addr.byte.loByte < mX)
				m_cpu_sequence++;
			else
			{
				m_cpu_sequence=m_cpu_final_sequence;
			}
			break;
		case C_ABSOLUTEX_READ_3:
			ReadByte(addr.word);
			CHECK_BA;
			addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;		
		case C_ABSOLUTEY_READ:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEY_READ_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mY;
			if (addr.byte.loByte < mY)
				m_cpu_sequence++;
			else
			{
				m_cpu_sequence=m_cpu_final_sequence;
			}
			break;
		case C_ABSOLUTEY_READ_3:
			ReadByte(addr.word);
			CHECK_BA;
			addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;		
		case C_ABSOLUTEX_WRITE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;		
		case C_ABSOLUTEX_WRITE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mX;
			m_cpu_sequence++;
			break;		
		case C_ABSOLUTEX_WRITE_3:
			ReadByte(addr.word);
			CHECK_BA;
			if (addr.byte.loByte < mX)
				addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;		
		case C_ABSOLUTEY_WRITE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;		
		case C_ABSOLUTEY_WRITE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mY;
			m_cpu_sequence++;
			break;		
		case C_ABSOLUTEY_WRITE_3:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			if (addr.byte.loByte < mY)
				addr.byte.hiByte++;
			m_cpu_sequence=m_cpu_final_sequence;
			break;		
		case C_ABSOLUTEX_READWRITE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEX_READWRITE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mX;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEX_READWRITE_3:
			ReadByte(addr.word);
			CHECK_BA;
			if (addr.byte.loByte < mX)
				addr.byte.hiByte++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEX_READWRITE_4:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEX_READWRITE_5:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case C_ABSOLUTEY_READWRITE:
			addr.byte.loByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEY_READWRITE_2:
			addr.byte.hiByte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			addr.byte.loByte+=mY;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEY_READWRITE_3:
			ReadByte(addr.word);
			CHECK_BA;
			if (addr.byte.loByte < mY)
				addr.byte.hiByte++;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEY_READWRITE_4:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_ABSOLUTEY_READWRITE_5:
			WriteByte(addr.word,databyte);
			m_cpu_sequence=m_cpu_final_sequence;
			break;
		case ORA_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = ORA_ZEROPAGE;
		case ORA_ZEROPAGE:
		case ORA_ZEROPAGEX:
		case ORA_ABSOLUTE:
		case ORA_ABSOLUTEY:
		case ORA_ABSOLUTEX:
		case ORA_INDIRECTY:
		case ORA_INDIRECTX:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			mA|=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case AND_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = AND_ZEROPAGE;
		case AND_ZEROPAGE:
		case AND_ZEROPAGEX:
		case AND_ABSOLUTE:
		case AND_ABSOLUTEY:
		case AND_ABSOLUTEX:
		case AND_INDIRECTY:
		case AND_INDIRECTX:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			mA&=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case EOR_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = EOR_ZEROPAGE;
		case EOR_ZEROPAGE:
		case EOR_ZEROPAGEX:
		case EOR_ABSOLUTE:
		case EOR_ABSOLUTEY:
		case EOR_ABSOLUTEX:
		case EOR_INDIRECTY:
		case EOR_INDIRECTX:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			mA^=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ADC_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = ADC_ZEROPAGE;
		case ADC_ZEROPAGE:
		case ADC_ZEROPAGEX:
		case ADC_ABSOLUTE:
		case ADC_ABSOLUTEY:
		case ADC_ABSOLUTEX:
		case ADC_INDIRECTY:
		case ADC_INDIRECTX:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			mA = code_add(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SBC_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = SBC_ZEROPAGE;
		case SBC_ZEROPAGE:
		case SBC_ZEROPAGEX:
		case SBC_ABSOLUTE:
		case SBC_ABSOLUTEY:
		case SBC_ABSOLUTEX:
		case SBC_INDIRECTY:
		case SBC_INDIRECTX:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			mA = code_sub(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CMP_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = CMP_ZEROPAGE;
		case CMP_ZEROPAGE:
		case CMP_ZEROPAGEX:
		case CMP_ABSOLUTE:
		case CMP_ABSOLUTEX:
		case CMP_ABSOLUTEY:
		case CMP_INDIRECTX:
		case CMP_INDIRECTY:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			code_cmp(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CPX_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = CPX_ZEROPAGE;
		case CPX_ZEROPAGE:
		case CPX_ABSOLUTE:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			code_cmp(mX, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CPY_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = CPY_ZEROPAGE;
		case CPY_ZEROPAGE:
		case CPY_ABSOLUTE:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			code_cmp(mY, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case DEC_ZEROPAGE:
		case DEC_ZEROPAGEX:
		case DEC_ABSOLUTE:
		case DEC_ABSOLUTEX:
			databyte--;
			WriteByte(addr.word,databyte);
			fZERO = (databyte == 0);
			fNEGATIVE = (databyte & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;

		case DEX_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mX--;
			fZERO = (mX == 0);
			fNEGATIVE = (mX & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case DEY_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mY--;
			fZERO = (mY == 0);
			fNEGATIVE = (mY & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case INX_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mX++;
			fZERO = (mX == 0);
			fNEGATIVE = (mX & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case INY_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mY++;
			fZERO = (mY == 0);
			fNEGATIVE = (mY & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case INC_ZEROPAGE:
		case INC_ZEROPAGEX:
		case INC_ABSOLUTE:
		case INC_ABSOLUTEX:
			databyte++;
			WriteByte(addr.word,databyte);
			fZERO = (databyte == 0);
			fNEGATIVE = (databyte & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LDA_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = LDA_ZEROPAGE;
		case LDA_ZEROPAGE:
		case LDA_ZEROPAGEX:
		case LDA_ABSOLUTE:
		case LDA_ABSOLUTEX:
		case LDA_ABSOLUTEY:
		case LDA_INDIRECTX:
		case LDA_INDIRECTY:
			mA=ReadByte(addr.word);
			CHECK_BA;
			fZERO = (mA == 0);
			fNEGATIVE = (mA & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LDX_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = LDX_ZEROPAGE;
		case LDX_ZEROPAGE:
		case LDX_ZEROPAGEY:
		case LDX_ABSOLUTE:
		case LDX_ABSOLUTEY:
			mX=ReadByte(addr.word);
			CHECK_BA;
			fZERO = (mX == 0);
			fNEGATIVE = (mX & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LDY_IMMEDIATE:
			addr.word=mPC.word++;
			m_cpu_sequence = LDY_ZEROPAGE;
		case LDY_ZEROPAGE:
		case LDY_ZEROPAGEX:
		case LDY_ABSOLUTE:
		case LDY_ABSOLUTEX:
			mY=ReadByte(addr.word);
			CHECK_BA;
			fZERO = (mY == 0);
			fNEGATIVE = (mY & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LSR_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fCARRY=(mA & 0x1);
			mA>>=1;
			fNEGATIVE= 0;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LSR_ZEROPAGE:
		case LSR_ZEROPAGEX:
		case LSR_ABSOLUTE:
		case LSR_ABSOLUTEX:
			fCARRY=(databyte & 0x1);
			databyte>>=1;
			fNEGATIVE= 0;
			fZERO= (databyte==0);
			WriteByte(addr.word, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ASL_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fCARRY=(mA & 0x0080) >> 7;
			mA<<=1;
			fNEGATIVE= (mA & 0x0080) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ASL_ZEROPAGE:
		case ASL_ZEROPAGEX:
		case ASL_ABSOLUTE:
		case ASL_ABSOLUTEX:
			fCARRY=(databyte & 0x0080) >> 7;
			databyte<<=1;
			fNEGATIVE= (databyte & 0x0080) >> 7;
			fZERO= (databyte==0);
			WriteByte(addr.word, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ROL_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			v=mA;
			v<<=1;
			v|=fCARRY;
			fCARRY=(v & 0x0100) >> 8;
			fNEGATIVE=(v & 0x80) >> 7;
			mA=v & 0xFF;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ROL_ZEROPAGE:
		case ROL_ZEROPAGEX:
		case ROL_ABSOLUTE:
		case ROL_ABSOLUTEX:
			v=databyte;
			v<<=1;
			v|=fCARRY;
			fCARRY=(v & 0x0100) >> 8;
			fNEGATIVE=(v & 0x80) >> 7;
			v=v & 0xFF;
			fZERO= (v==0);
			WriteByte(addr.word, v);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;		
		case ROR_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			v=mA;
			v|= (fCARRY << 8);
			fCARRY= v & 1;
			v>>=1;
			fNEGATIVE=(v & 0x80) >> 7;
			fCARRY= mA & 1;
			mA=v & 0xFF;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ROR_ZEROPAGE:
		case ROR_ZEROPAGEX:
		case ROR_ABSOLUTE:
		case ROR_ABSOLUTEX:
			v=databyte;
			v|= (fCARRY << 8);
			fCARRY= v & 1;
			v>>=1;
			fNEGATIVE=(v & 0x80) >> 7;
			v=v & 0xFF;
			fZERO= (v==0);
			WriteByte(addr.word, v);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case STA_ZEROPAGE:
		case STA_ZEROPAGEX:
		case STA_ABSOLUTE:
		case STA_ABSOLUTEX:
		case STA_ABSOLUTEY:
		case STA_INDIRECTX:
		case STA_INDIRECTY:
			WriteByte(addr.word,mA);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;

		case STX_ZEROPAGE:
		case STX_ZEROPAGEY:
		case STX_ABSOLUTE:
			WriteByte(addr.word,mX);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case STY_ZEROPAGE:
		case STY_ZEROPAGEX:
		case STY_ABSOLUTE:
			WriteByte(addr.word,mY);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TAX_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mX=mA;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TAY_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mY=mA;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TXA_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mA=mX;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TYA_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mA=mY;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TSX_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mX=mSP;
			fNEGATIVE=(mX & 0x80) >> 7;
			fZERO= (mX==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case TXS_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			mSP=mX;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case NOP_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_PHA_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_PHA_IMPLIED_2:
			CPU6502_PUSH_A;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_PLA_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_PLA_IMPLIED_2:
			ReadByte(mSP + 0x0100);//Read from stack and throw away.
			CHECK_BA;
			mSP++;
			m_cpu_sequence++;
			break;
		case C_PLA_IMPLIED_3:
			mA=ReadByte(mSP + 0x0100);
			CHECK_BA;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case BPL_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fNEGATIVE==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			break;
		case BMI_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fNEGATIVE==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			break;
		case BNE_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fZERO==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			break;
		case BEQ_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fZERO==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			break;
		case BVC_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			SyncVFlag();
			if (fOVERFLOW==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			break;
		case BVS_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			SyncVFlag();
			if (fOVERFLOW==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			break;
		case BCC_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fCARRY==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			break;
		case BCS_RELATIVE:
			databyte=ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			if (fCARRY==0)
			{
				check_interrupts1();
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			else
			{
				check_interrupts1();
				m_cpu_sequence=C_BRANCH;
			}
			break;
		case C_BRANCH:
			ReadByte(mPC.word);
			CHECK_BA;
			addr.word=(bit16)((signed short)mPC.word + (signed short)(signed char)databyte);
			mPC.byte.loByte+=databyte;
			if (addr.byte.hiByte == mPC.byte.hiByte)
			{
				mPC.word = addr.word;
				m_cpu_sequence=C_FETCH_OPCODE;
				m_CurrentOpcodeAddress = mPC;
				m_CurrentOpcodeClock = CurrentClock;
			}
			else
			{
				m_cpu_sequence++;
			}
			break;
		case C_BRANCH_2:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word = addr.word;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_RTI:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_RTI_2:
			ReadByte(mSP + 0x0100);//Read from stack and throw away.
			CHECK_BA;
			mSP++;
			m_cpu_sequence++;
			break;
		case C_RTI_3:
			databyte = ReadByte(mSP + 0x0100);
			CHECK_BA;
			SyncVFlag();
			CPU6502_SET_PS(databyte);
			mSP++;
			m_cpu_sequence++;
			break;
		case C_RTI_4:
			CPU6502_LOAD_PCL(ReadByte(mSP + 0x0100));
			CHECK_BA;
			mSP++;
			m_cpu_sequence++;
			break;
		case C_RTI_5:
			CPU6502_LOAD_PCH(ReadByte(mSP + 0x0100));
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_RTS:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_RTS_2:
			ReadByte(mSP + 0x0100);//Read from stack and throw away.
			CHECK_BA;
			++mSP;
			m_cpu_sequence++;
			break;
		case C_RTS_3:
			CPU6502_LOAD_PCL(ReadByte(mSP + 0x0100));
			CHECK_BA;
			++mSP;
			m_cpu_sequence++;
			break;
		case C_RTS_4:
			CPU6502_LOAD_PCH(ReadByte(mSP + 0x0100));
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_RTS_5:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CLC_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fCARRY=0;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SEC_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fCARRY=1;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CLI_IMPLIED:
			ReadByte(mPC.word);
			if (BA_AND_DEBUG)
			{
				//If BA transitions to low in the second cycle of CLI then the IRQ check is performed as though the I flag was clear at the start of CLI.
				fINTERRUPT = 0;
				++m_CurrentOpcodeClock;
				break;
			}
			check_interrupts1();
			fINTERRUPT=0;

			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SEI_IMPLIED:
			ReadByte(mPC.word);
			if (BA_AND_DEBUG)
			{
				//If both BA and IRQ transition to low in the second cycle of SEI then no IRQ occurs in the next instruction.
				m_bBALowInClock2OfSEI = true;
				++m_CurrentOpcodeClock;
				break;
			}
			SyncChips();
			if (IRQ != 0 && m_bBALowInClock2OfSEI && (((ICLKS)(FirstIRQClock - FirstBALowClock)) >= 0))
			{
				fINTERRUPT = 1;
			}
			check_interrupts1();
			m_bBALowInClock2OfSEI = false;

			fINTERRUPT=1;
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_PHP_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_PHP_IMPLIED_2:
			SyncVFlag();
			CPU6502_PUSH_PS_B;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_PLP_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_PLP_IMPLIED_2:
			ReadByte(mSP + 0x0100);//Read from stack and throw away.
			CHECK_BA;
			mSP++;
			m_cpu_sequence++;
			break;
		case C_PLP_IMPLIED_3:
			databyte = ReadByte(mSP + 0x0100);
			CHECK_BA;
			check_interrupts1();
			SyncVFlag();
			CPU6502_SET_PS(databyte);
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CLD_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fDECIMAL=0;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SED_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			fDECIMAL=1;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case CLV_IMPLIED:
			ReadByte(mPC.word);
			CHECK_BA;
			SyncVFlag();
			fOVERFLOW=0;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case BIT_ZEROPAGE:
		case BIT_ABSOLUTE:
			databyte=ReadByte(addr.word);
			CHECK_BA;
			fNEGATIVE=(databyte & 0x80) >> 7;
			SyncVFlag();
			fOVERFLOW=(databyte & 0x40) >> 6;
			fZERO= (mA & databyte)==0;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_ILLEGAL:
			//FIXME
			Reset(CurrentClock);
			break;
		case SLO_INDIRECTY:
			fCARRY=(databyte & 0x0080) >> 7;
			databyte<<=1;
			WriteByte(addr.word, databyte);
			m_cpu_sequence=C_SLO_2;
			break;
		case SLO_ZEROPAGE:
		case SLO_ZEROPAGEX:
		case SLO_ABSOLUTE:
		case SLO_ABSOLUTEY:
		case SLO_ABSOLUTEX:
		case SLO_INDIRECTX:
			fCARRY=(databyte & 0x0080) >> 7;
			databyte<<=1;
			WriteByte(addr.word, databyte);
			mA|=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_SLO_2:
			WriteByte(addr.word, databyte);
			mA|=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case RLA_INDIRECTY:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=RLA_ZEROPAGE;
			break;
		case RLA_ZEROPAGE:
		case RLA_ZEROPAGEX:
		case RLA_ABSOLUTE:
		case RLA_ABSOLUTEY:
		case RLA_ABSOLUTEX:
		case RLA_INDIRECTX:
			v=databyte;
			v<<=1;
			v|=fCARRY;
			fCARRY=(v & 0x0100) >> 8;
			databyte=(bit8)v;
			WriteByte(addr.word, databyte);
			
			mA&=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SRE_INDIRECTY:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=SRE_ZEROPAGE;
			break;
		case SRE_ZEROPAGE:
		case SRE_ZEROPAGEX:
		case SRE_ABSOLUTE:
		case SRE_ABSOLUTEY:
		case SRE_ABSOLUTEX:
		case SRE_INDIRECTX:
			fCARRY=(databyte & 0x1);
			databyte>>=1;
			WriteByte(addr.word, databyte);
			
			mA^=databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case RRA_INDIRECTY:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=RRA_ZEROPAGE;
			break;
		case RRA_ZEROPAGE:
		case RRA_ZEROPAGEX:
		case RRA_ABSOLUTE:
		case RRA_ABSOLUTEY:
		case RRA_ABSOLUTEX:
		case RRA_INDIRECTX:
			v = databyte;
			v |= (fCARRY << 8);
			fCARRY = v & 1;
			v >>= 1;
			databyte= (bit8) v;
			WriteByte(addr.word, v);

			mA = code_add(mA, v);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SAX_ZEROPAGE:
		case SAX_ZEROPAGEY:
		case SAX_ABSOLUTE:
		case SAX_INDIRECTX:
			WriteByte(addr.word,mA & mX);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LAX_ZEROPAGE:
		case LAX_ZEROPAGEY:
		case LAX_ABSOLUTE:
		case LAX_ABSOLUTEY:
		case LAX_INDIRECTX:
		case LAX_INDIRECTY:
			mX=mA=ReadByte(addr.word);
			CHECK_BA;
			fZERO = (mA == 0);
			fNEGATIVE = (mA & 0x80) >>7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case DCP_INDIRECTY:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=DCP_ZEROPAGE;
			break;
		case DCP_ZEROPAGE:
		case DCP_ZEROPAGEX:
		case DCP_ABSOLUTE:
		case DCP_ABSOLUTEY:
		case DCP_ABSOLUTEX:
		case DCP_INDIRECTX:
			databyte--;
			WriteByte(addr.word,databyte);
			code_cmp(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ISB_INDIRECTY:
			WriteByte(addr.word, databyte);
			m_cpu_sequence=ISB_ZEROPAGE;
			break;
		case ISB_ZEROPAGE:
		case ISB_ZEROPAGEX:
		case ISB_ABSOLUTE:
		case ISB_ABSOLUTEY:
		case ISB_ABSOLUTEX:
		case ISB_INDIRECTX:
			databyte++;
			WriteByte(addr.word,databyte);
			mA = code_sub(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ALR_IMMEDIATE:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			mA &= databyte;
			fCARRY=(mA & 0x1);
			mA>>=1;
			fNEGATIVE= 0;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case ARR_IMMEDIATE:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			mA = code_arr(mA, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case XAA_IMMEDIATE://ANE
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			this->SyncChips();
			if (((ICLKS)(CurrentClock - this->LastBAHighClock)) > 1)
				mA |= 0xEE;
			else
				mA |= 0xEE;//BA just gone high

			////Made up rule to allow bit0 to transfer from X to A
			////Needed for some burner loaders like Obitron, DarkStar and Spectipede
			//if (databyte > 3 && (databyte & 0xf0) != 0)
			//	mA |= 0x1;

			mA &= mX & (bit8)databyte;
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case OAL_IMMEDIATE://LXA
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			mA|=0xEE;
			mA = mX = (mA & databyte);
			fNEGATIVE=(mA & 0x80) >> 7;
			fZERO= (mA==0);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SBX_IMMEDIATE:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			mX = code_cmp(mA & mX, databyte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_SKB_IMMEDIATE_2CLK:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_SKB_IMMEDIATE_3CLK:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_SKB_IMMEDIATE_3CLK_2:
			ReadByte(mPC.word);
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case C_SKB_IMMEDIATE_4CLK:
			ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			m_cpu_sequence++;
			break;
		case C_SKB_IMMEDIATE_4CLK_2:
			ReadByte(mPC.word);
			CHECK_BA;
			m_cpu_sequence++;
			break;
		case C_SKB_IMMEDIATE_4CLK_3:
			ReadByte(mPC.word);
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case SKW_ABSOLUTE:
		case SKW_ABSOLUTEX:
		case SKW_ABSOLUTEX_3C:
		case SKW_ABSOLUTEX_5C:
		case SKW_ABSOLUTEX_7C:
		case SKW_ABSOLUTEX_DC:
		case SKW_ABSOLUTEX_FC:
			ReadByte(addr.word);
			CHECK_BA;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case HLT_IMPLIED:
			break;
		case TAS_ABSOLUTEY://SHS
			this->SyncChips();
			if (((ICLKS)(CurrentClock - this->LastBAHighClock)) > 1)
			{
				axa_byte = ((bit8)((addr.word-mY) >> 8)+1);
			}
			else
			{
				axa_byte = 0xff;
			}
			mSP = (mA & mX);
			WriteByte(addr.word, mSP & axa_byte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;	
		case SAY_ABSOLUTEX://SHY
			this->SyncChips();
			if (((ICLKS)(CurrentClock - this->LastBAHighClock)) > 1)
			{
				axa_byte = ((bit8)((addr.word-mX) >> 8)+1);
			}
			else
			{
				axa_byte = 0xff;
			}
			WriteByte(addr.word, mY & axa_byte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;	
		case XAS_ABSOLUTEY://SHX
			this->SyncChips();
			if (((ICLKS)(CurrentClock - this->LastBAHighClock)) > 1)
			{
				axa_byte = ((bit8)((addr.word-mY) >> 8)+1);
			}
			else
			{
				axa_byte = 0xff;
			}
			WriteByte(addr.word, mX & axa_byte);
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;	
		case AXA_ABSOLUTEY://SHAAY
		case AXA_INDIRECTY://SHAIY AHX
			this->SyncChips();
			if (((ICLKS)(CurrentClock - this->LastBAHighClock)) > 1)
			{
				axa_byte = ((bit8)((addr.word-mY) >> 8)+1);
			}
			else
			{
				axa_byte = 0xff;
			}
			WriteByte(addr.word, mA & mX & axa_byte);

			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;	
		case ANC_IMMEDIATE:
			databyte = ReadByte(mPC.word);
			CHECK_BA;
			mPC.word++;
			mA &= databyte;
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			fCARRY= fNEGATIVE;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		case LAS_ABSOLUTEY:
			databyte = ReadByte(addr.word);
			CHECK_BA;
			mSP = mX = mA = (mSP & databyte);
			fZERO= (mA==0);
			fNEGATIVE= (mA & 0x80) >> 7;
			check_interrupts1();
			m_cpu_sequence=C_FETCH_OPCODE;
			m_CurrentOpcodeAddress = mPC;
			m_CurrentOpcodeClock = CurrentClock;
			break;
		}
	}
}

bool CPU6502::IsWriteCycle()
{
	switch (m_cpu_sequence)
	{
		case C_FETCH_OPCODE:
			return false;
		case C_IRQ:
			return false;
		case C_IRQ_2:
		case C_IRQ_3:
		case C_IRQ_4:
			return true;
		case C_NMI:
			return false;
		case C_NMI_2:
		case C_NMI_3:
		case C_NMI_4:
			return true;
		case C_BRK_IMPLIED:
			return false;
		case C_BRK_IMPLIED_2:
		case C_BRK_IMPLIED_3:
		case C_BRK_IMPLIED_4:
			return true;
		case C_LOAD_PC:
			return false;
		case C_LOAD_PC_2:
			return false;
		case C_RESET:
		case C_RESET_1:
		case C_RESET_2:
		case C_RESET_3:
		case C_RESET_4:
		case C_RESET_5:
		case C_RESET_6:
		case C_RESET_7:
			return false;
		case C_INDIRECTX:
		case C_INDIRECTX_2:
		case C_INDIRECTX_3:
		case C_INDIRECTX_4:
			return false;
		case C_INDIRECTX_READWRITE:
		case C_INDIRECTX_READWRITE_2:
		case C_INDIRECTX_READWRITE_3:
		case C_INDIRECTX_READWRITE_4:
		case C_INDIRECTX_READWRITE_5:
			return false;
		case C_INDIRECTX_READWRITE_6:
			return true;
		case C_INDIRECTY_READ:
		case C_INDIRECTY_READ_2:
		case C_INDIRECTY_READ_3:
		case C_INDIRECTY_READ_4:
			return false;
		case C_INDIRECTY_WRITE:
		case C_INDIRECTY_WRITE_2:
		case C_INDIRECTY_WRITE_3:
		case C_INDIRECTY_WRITE_4:
			return false;
		case C_INDIRECTY_READWRITE:
		case C_INDIRECTY_READWRITE_2:
		case C_INDIRECTY_READWRITE_3:
		//step 4 is not needed here:- 
		case C_INDIRECTY_READWRITE_4:
		case C_INDIRECTY_READWRITE_5:
			return false;
		case C_INDIRECTY_READWRITE_6:
			return true;
		case C_ZEROPAGE:
		case C_ZEROPAGE_READWRITE:
		case C_ZEROPAGE_READWRITE_2:
			return false;
		case C_ZEROPAGE_READWRITE_3:
			return true;
		case C_ZEROPAGEX:
		case C_ZEROPAGEX_2:
			return false;
		case C_ZEROPAGEY:
		case C_ZEROPAGEY_2:
			return false;
		case C_ZEROPAGEX_READWRITE:
		case C_ZEROPAGEX_READWRITE_2:
		case C_ZEROPAGEX_READWRITE_3:
			return false;
		case C_ZEROPAGEX_READWRITE_4:
			return true;
		case C_JMP_ABSOLUTE:
		case C_JMP_ABSOLUTE_2:
		case C_JSR_ABSOLUTE:
		case C_JSR_ABSOLUTE_2:
			return false;
		case C_JSR_ABSOLUTE_3:
			return true;
		case C_JSR_ABSOLUTE_4:
			return true;
		case C_JSR_ABSOLUTE_5:
			return false;
		case C_JMP_INDIRECT:
		case C_JMP_INDIRECT_2:
		case C_JMP_INDIRECT_3:
		case C_JMP_INDIRECT_4:
			return false;
		case C_ABSOLUTE:
		case C_ABSOLUTE_2:
			return false;
		case C_ABSOLUTE_READWRITE:
		case C_ABSOLUTE_READWRITE_2:
		case C_ABSOLUTE_READWRITE_3:
			return false;
		case C_ABSOLUTE_READWRITE_4:
			return true;
		case C_ABSOLUTEX_READ:
		case C_ABSOLUTEX_READ_2:
		case C_ABSOLUTEX_READ_3:
			return false;
		case C_ABSOLUTEY_READ:
		case C_ABSOLUTEY_READ_2:
		case C_ABSOLUTEY_READ_3:
			return false;
		case C_ABSOLUTEX_WRITE:
		case C_ABSOLUTEX_WRITE_2:
		case C_ABSOLUTEX_WRITE_3:
			return false;
		case C_ABSOLUTEY_WRITE:
		case C_ABSOLUTEY_WRITE_2:
		case C_ABSOLUTEY_WRITE_3:
			return false;
		case C_ABSOLUTEX_READWRITE:
		case C_ABSOLUTEX_READWRITE_2:
		case C_ABSOLUTEX_READWRITE_3:
		case C_ABSOLUTEX_READWRITE_4:
			return false;
		case C_ABSOLUTEX_READWRITE_5:
			return true;
		case C_ABSOLUTEY_READWRITE:
		case C_ABSOLUTEY_READWRITE_2:
		case C_ABSOLUTEY_READWRITE_3:
		case C_ABSOLUTEY_READWRITE_4:
			return false;
		case C_ABSOLUTEY_READWRITE_5:
			return true;
		case ORA_IMMEDIATE:
		case ORA_ZEROPAGE:
		case ORA_ZEROPAGEX:
		case ORA_ABSOLUTE:
		case ORA_ABSOLUTEY:
		case ORA_ABSOLUTEX:
		case ORA_INDIRECTY:
		case ORA_INDIRECTX:
			return false;
		case AND_IMMEDIATE:
		case AND_ZEROPAGE:
		case AND_ZEROPAGEX:
		case AND_ABSOLUTE:
		case AND_ABSOLUTEY:
		case AND_ABSOLUTEX:
		case AND_INDIRECTY:
		case AND_INDIRECTX:
			return false;
		case EOR_IMMEDIATE:
		case EOR_ZEROPAGE:
		case EOR_ZEROPAGEX:
		case EOR_ABSOLUTE:
		case EOR_ABSOLUTEY:
		case EOR_ABSOLUTEX:
		case EOR_INDIRECTY:
		case EOR_INDIRECTX:
			return false;
		case ADC_IMMEDIATE:
		case ADC_ZEROPAGE:
		case ADC_ZEROPAGEX:
		case ADC_ABSOLUTE:
		case ADC_ABSOLUTEY:
		case ADC_ABSOLUTEX:
		case ADC_INDIRECTY:
		case ADC_INDIRECTX:
			return false;
		case SBC_IMMEDIATE:
		case SBC_ZEROPAGE:
		case SBC_ZEROPAGEX:
		case SBC_ABSOLUTE:
		case SBC_ABSOLUTEY:
		case SBC_ABSOLUTEX:
		case SBC_INDIRECTY:
		case SBC_INDIRECTX:
			return false;
		case CMP_IMMEDIATE:
		case CMP_ZEROPAGE:
		case CMP_ZEROPAGEX:
		case CMP_ABSOLUTE:
		case CMP_ABSOLUTEX:
		case CMP_ABSOLUTEY:
		case CMP_INDIRECTX:
		case CMP_INDIRECTY:
			return false;
		case CPX_IMMEDIATE:
		case CPX_ZEROPAGE:
		case CPX_ABSOLUTE:
			return false;
		case CPY_IMMEDIATE:
		case CPY_ZEROPAGE:
		case CPY_ABSOLUTE:
			return false;
		case DEC_ZEROPAGE:
		case DEC_ZEROPAGEX:
		case DEC_ABSOLUTE:
		case DEC_ABSOLUTEX:
			return true;
		case DEX_IMPLIED:
			return false;
		case DEY_IMPLIED:
			return false;
		case INX_IMPLIED:
			return false;
		case INY_IMPLIED:
			return false;
		case INC_ZEROPAGE:
		case INC_ZEROPAGEX:
		case INC_ABSOLUTE:
		case INC_ABSOLUTEX:
			return true;
		case LDA_IMMEDIATE:
		case LDA_ZEROPAGE:
		case LDA_ZEROPAGEX:
		case LDA_ABSOLUTE:
		case LDA_ABSOLUTEX:
		case LDA_ABSOLUTEY:
		case LDA_INDIRECTX:
		case LDA_INDIRECTY:
			return false;
		case LDX_IMMEDIATE:
		case LDX_ZEROPAGE:
		case LDX_ZEROPAGEY:
		case LDX_ABSOLUTE:
		case LDX_ABSOLUTEY:
			return false;
		case LDY_IMMEDIATE:
		case LDY_ZEROPAGE:
		case LDY_ZEROPAGEX:
		case LDY_ABSOLUTE:
		case LDY_ABSOLUTEX:
			return false;
		case LSR_IMPLIED:
			return false;
		case LSR_ZEROPAGE:
		case LSR_ZEROPAGEX:
		case LSR_ABSOLUTE:
		case LSR_ABSOLUTEX:
			return true;
		case ASL_IMPLIED:
			return false;
		case ASL_ZEROPAGE:
		case ASL_ZEROPAGEX:
		case ASL_ABSOLUTE:
		case ASL_ABSOLUTEX:
			return true;
		case ROL_IMPLIED:
			return false;
		case ROL_ZEROPAGE:
		case ROL_ZEROPAGEX:
		case ROL_ABSOLUTE:
		case ROL_ABSOLUTEX:
			return true;		
		case ROR_IMPLIED:
			return false;
		case ROR_ZEROPAGE:
		case ROR_ZEROPAGEX:
		case ROR_ABSOLUTE:
		case ROR_ABSOLUTEX:
			return true;
		case STA_ZEROPAGE:
		case STA_ZEROPAGEX:
		case STA_ABSOLUTE:
		case STA_ABSOLUTEX:
		case STA_ABSOLUTEY:
		case STA_INDIRECTX:
		case STA_INDIRECTY:
			return true;
		case STX_ZEROPAGE:
		case STX_ZEROPAGEY:
		case STX_ABSOLUTE:
			return true;
		case STY_ZEROPAGE:
		case STY_ZEROPAGEX:
		case STY_ABSOLUTE:
			return true;
		case TAX_IMPLIED:
			return false;
		case TAY_IMPLIED:
			return false;
		case TXA_IMPLIED:
			return false;
		case TYA_IMPLIED:
			return false;
		case TSX_IMPLIED:
			return false;
		case TXS_IMPLIED:
			return false;
		case NOP_IMPLIED:
			return false;
		case C_PHA_IMPLIED:
			return false;
		case C_PHA_IMPLIED_2:
			return true;
		case C_PLA_IMPLIED:
		case C_PLA_IMPLIED_2:
		case C_PLA_IMPLIED_3:
			return false;
		case BPL_RELATIVE:
		case BMI_RELATIVE:
		case BNE_RELATIVE:
		case BEQ_RELATIVE:
		case BVC_RELATIVE:
		case BVS_RELATIVE:
		case BCC_RELATIVE:
		case BCS_RELATIVE:
		case C_BRANCH:
		case C_BRANCH_2:
			return false;
		case C_RTI:
		case C_RTI_2:
		case C_RTI_3:
		case C_RTI_4:
		case C_RTI_5:
			return false;
		case C_RTS:
		case C_RTS_2:
		case C_RTS_3:
		case C_RTS_4:
		case C_RTS_5:
			return false;
		case CLC_IMPLIED:
			return false;
		case SEC_IMPLIED:
			return false;
		case CLI_IMPLIED:
			return false;
		case SEI_IMPLIED:
			return false;
		case C_PHP_IMPLIED:
			return false;
		case C_PHP_IMPLIED_2:
			return true;
		case C_PLP_IMPLIED:
		case C_PLP_IMPLIED_2:
		case C_PLP_IMPLIED_3:
			return false;
		case CLD_IMPLIED:
			return false;
		case SED_IMPLIED:
			return false;
		case CLV_IMPLIED:
			return false;
		case BIT_ZEROPAGE:
		case BIT_ABSOLUTE:
			return false;
		case C_ILLEGAL:
			return false;
		case SLO_INDIRECTY:
		case SLO_ZEROPAGE:
		case SLO_ZEROPAGEX:
		case SLO_ABSOLUTE:
		case SLO_ABSOLUTEY:
		case SLO_ABSOLUTEX:
		case SLO_INDIRECTX:
		case C_SLO_2:
			return true;
		case RLA_INDIRECTY:
		case RLA_ZEROPAGE:
		case RLA_ZEROPAGEX:
		case RLA_ABSOLUTE:
		case RLA_ABSOLUTEY:
		case RLA_ABSOLUTEX:
		case RLA_INDIRECTX:
			return true;
		case SRE_INDIRECTY:
		case SRE_ZEROPAGE:
		case SRE_ZEROPAGEX:
		case SRE_ABSOLUTE:
		case SRE_ABSOLUTEY:
		case SRE_ABSOLUTEX:
		case SRE_INDIRECTX:
			return true;
		case RRA_INDIRECTY:
		case RRA_ZEROPAGE:
		case RRA_ZEROPAGEX:
		case RRA_ABSOLUTE:
		case RRA_ABSOLUTEY:
		case RRA_ABSOLUTEX:
		case RRA_INDIRECTX:
			return true;
		case SAX_ZEROPAGE:
		case SAX_ZEROPAGEY:
		case SAX_ABSOLUTE:
		case SAX_INDIRECTX:
			return true;
		case LAX_ZEROPAGE:
		case LAX_ZEROPAGEY:
		case LAX_ABSOLUTE:
		case LAX_ABSOLUTEY:
		case LAX_INDIRECTX:
		case LAX_INDIRECTY:
			return false;
		case DCP_INDIRECTY:
		case DCP_ZEROPAGE:
		case DCP_ZEROPAGEX:
		case DCP_ABSOLUTE:
		case DCP_ABSOLUTEY:
		case DCP_ABSOLUTEX:
		case DCP_INDIRECTX:
			return true;
		case ISB_INDIRECTY:
		case ISB_ZEROPAGE:
		case ISB_ZEROPAGEX:
		case ISB_ABSOLUTE:
		case ISB_ABSOLUTEY:
		case ISB_ABSOLUTEX:
		case ISB_INDIRECTX:
			return true;
		case ALR_IMMEDIATE:
			return false;
		case ARR_IMMEDIATE:
			return false;
		case XAA_IMMEDIATE:
			return false;
		case OAL_IMMEDIATE:
			return false;
		case SBX_IMMEDIATE:
			return false;
		case C_SKB_IMMEDIATE_2CLK:
		case C_SKB_IMMEDIATE_3CLK:
		case C_SKB_IMMEDIATE_3CLK_2:
		case C_SKB_IMMEDIATE_4CLK:
		case C_SKB_IMMEDIATE_4CLK_2:
		case C_SKB_IMMEDIATE_4CLK_3:
			return false;
		case SKW_ABSOLUTE:
		case SKW_ABSOLUTEX:
		case SKW_ABSOLUTEX_3C:
		case SKW_ABSOLUTEX_5C:
		case SKW_ABSOLUTEX_7C:
		case SKW_ABSOLUTEX_DC:
		case SKW_ABSOLUTEX_FC:
			return false;
		case HLT_IMPLIED:
			return false;
		case TAS_ABSOLUTEY:
			return true;
		case SAY_ABSOLUTEX:
			return true;
		case XAS_ABSOLUTEY:
			return true;
		case AXA_ABSOLUTEY:
		case AXA_INDIRECTY:
			return true;
		case ANC_IMMEDIATE:
			return false;
		case LAS_ABSOLUTEY:
			return false;
		default:
			return false;
	}
}

void CPU6502::SetPC(bit16 address)
{
	m_cpu_sequence=C_FETCH_OPCODE;
	mPC.word = address;
	m_CurrentOpcodeAddress.word = address;
	m_CurrentOpcodeClock = CurrentClock;	
}

void CPU6502::SetA(bit8 v)
{
	this->mA = v;
}

void CPU6502::SetX(bit8 v)
{
	this->mX = v;
}

void CPU6502::SetY(bit8 v)
{
	this->mY = v;
}

void CPU6502::SetSR(bit8 v)
{
	fCARRY = v & 1;
	fZERO = (v & 2)!=0;
	fINTERRUPT = (v & 4)!=0;
	fDECIMAL = (v & 8)!=0;
	fBREAK = (v & 16)!=0;
	fOVERFLOW = (v & 64)!=0;
	fNEGATIVE = (v & 128)!=0;
}

void CPU6502::SetSP(bit8 v)
{
	this->mSP = v;
}

void CPU6502::SetDdr(bit8 v)
{
}

void CPU6502::SetData(bit8 v)
{
}

void CPU6502::GetState(SsCpuCommon &state)
{
	ZeroMemory(&state, sizeof(state));
	state.Model = 0;
	state.PC = mPC.word;
	state.A = mA;
	state.X = mX;
	state.Y = mY;
	state.SP = mSP;
	state.SR = (fNEGATIVE <<7) | (fOVERFLOW <<6) | (1<<5) | (fBREAK <<4) | (fDECIMAL <<3) | (fINTERRUPT <<2) | (fZERO <<1) | (fCARRY);
	state.CurrentClock = CurrentClock;
	state.SOTriggerClock = SOTriggerClock;
	state.FirstIRQClock = FirstIRQClock;
	state.FirstNMIClock = FirstNMIClock;
	state.RisingIRQClock = RisingIRQClock;
	state.FirstBALowClock = FirstBALowClock;
	state.LastBAHighClock = LastBAHighClock;
	state.m_CurrentOpcodeClock = m_CurrentOpcodeClock;
	state.m_cpu_sequence = m_cpu_sequence;
	state.m_cpu_final_sequence = m_cpu_final_sequence;
	state.m_op_code = m_op_code;
	state.m_CurrentOpcodeAddress = m_CurrentOpcodeAddress.word;
	state.addr = addr.word;
	state.ptr = ptr.word;
	state.databyte = databyte;
	state.SOTrigger = SOTrigger ? 1 : 0;
	state.m_bBALowInClock2OfSEI = m_bBALowInClock2OfSEI ? 1 : 0;
	state.BA = BA;
	state.PROCESSOR_INTERRUPT = PROCESSOR_INTERRUPT;
	state.IRQ = IRQ;
	state.NMI = NMI;
	state.NMI_TRIGGER = NMI_TRIGGER;
}

void CPU6502::SetState(const SsCpuCommon &state)
{
	//state.Model = 0;
	mPC.word = state.PC;
	mA = state.A;
	mX = state.X;
	mY = state.Y;
	mSP = state.SP;
	fNEGATIVE = (state.SR >> 7) & 1;
	fOVERFLOW = (state.SR >> 6) & 1;
	//(1<<5)
	fBREAK = (state.SR >> 4) & 1;
	fDECIMAL = (state.SR >> 3) & 1;
	fINTERRUPT = (state.SR >> 2) & 1;
	fZERO = (state.SR >> 1) & 1;
	fCARRY = (state.SR) & 1;
	CurrentClock = state.CurrentClock;
	SOTriggerClock = state.SOTriggerClock;
	FirstIRQClock = state.FirstIRQClock;
	FirstNMIClock = state.FirstNMIClock;
	RisingIRQClock = state.RisingIRQClock;
	FirstBALowClock = state.FirstBALowClock;
	LastBAHighClock = state.LastBAHighClock;
	m_CurrentOpcodeClock = state.m_CurrentOpcodeClock;
	m_cpu_sequence = state.m_cpu_sequence;
	m_cpu_final_sequence = state.m_cpu_final_sequence;
	m_op_code = state.m_op_code;
	m_CurrentOpcodeAddress.word = state.m_CurrentOpcodeAddress;
	addr.word = state.addr;
	ptr.word = state.ptr;
	databyte = state.databyte;
	SOTrigger = state.SOTrigger != 0;
	m_bBALowInClock2OfSEI = state.m_bBALowInClock2OfSEI != 0;
	BA = state.BA;
	PROCESSOR_INTERRUPT = state.PROCESSOR_INTERRUPT;
	IRQ = state.IRQ;
	NMI = state.NMI;
	NMI_TRIGGER = state.NMI_TRIGGER;	
}
