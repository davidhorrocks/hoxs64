#pragma once
#include "register.h"
#include "savestate.h"
#include "bpenum.h"
#include "errormsg.h"

#define BREAK_LIST_SIZE 999999

#define psNEGATIVE	(0x80)
#define psOVERFLOW	(0x40)
#define psBREAK		(0x10)
#define psDECIMAL	(0x8)
#define psINTERRUPT	(0x4)
#define psZERO		(0x2)
#define psCARRY		(0x1)
#define psNOT_NEGATIVE	(0x7F)
#define psNOT_OVERFLOW	(0xBF)
#define psNOT_BREAK		(0xEF)
#define psNOT_DECIMAL	(0xF7)
#define psNOT_INTERRUPT	(0xFB)
#define psNOT_ZERO		(0xFD)
#define psNOT_CARRY		(0xFE)

#define amINVALID		0
#define amIMPLIED		1
#define amIMMEDIATE		2
#define amZEROPAGE		3
#define amZEROPAGEX		4
#define amZEROPAGEY		5
#define amABSOLUTE		6
#define amABSOLUTEX		7
#define amABSOLUTEY		8
#define amINDIRECT		9
#define amINDIRECTX		10
#define amINDIRECTY		11
#define amRELATIVE		12


typedef struct {
	bit8	opcode;
	TCHAR	mnemonic[4];
	bit8	address_mode;
	bit8	undoc;
	bit8	size;
} InstructionInfo;

#define BRK_IMPLIED 0x00
#define ORA_INDIRECTX 0x01
#define HLT_IMPLIED	0x02
#define SLO_INDIRECTX 0x03
#define SKB_IMMEDIATE 0x04
#define ORA_ZEROPAGE 0x05
#define ASL_ZEROPAGE 0x06
#define SLO_ZEROPAGE 0x07
#define PHP_IMPLIED 0x08
#define ORA_IMMEDIATE 0x09
#define ASL_IMPLIED 0x0A
#define ANC_IMMEDIATE 0x0B
#define SKW_ABSOLUTE 0x0C
#define ORA_ABSOLUTE 0x0D
#define ASL_ABSOLUTE 0x0E
#define SLO_ABSOLUTE 0x0F

#define BPL_RELATIVE 0x10
#define ORA_INDIRECTY 0x11
#define HLT_IMPLIED_12 0x12
#define SLO_INDIRECTY 0x13
#define SKB_IMMEDIATE_14 0x14
#define ORA_ZEROPAGEX 0x15
#define ASL_ZEROPAGEX 0x16
#define SLO_ZEROPAGEX 0x17
#define CLC_IMPLIED 0x18
#define ORA_ABSOLUTEY 0x19
#define NOP_IMPLIED_1A 0x1A
#define SLO_ABSOLUTEY 0x1B
#define SKW_ABSOLUTEX 0x1C
#define ORA_ABSOLUTEX 0x1D
#define ASL_ABSOLUTEX 0x1E
#define SLO_ABSOLUTEX 0x1F


#define JSR_ABSOLUTE 0x20
#define AND_INDIRECTX 0x21
#define HLT_IMPLIED_22 0x22
#define RLA_INDIRECTX 0x23
#define BIT_ZEROPAGE 0x24
#define AND_ZEROPAGE 0x25
#define ROL_ZEROPAGE 0x26
#define RLA_ZEROPAGE 0x27
#define PLP_IMPLIED 0x28
#define AND_IMMEDIATE 0x29
#define ROL_IMPLIED 0x2A
#define ANC_IMMEDIATE_2B 0x2B
#define BIT_ABSOLUTE 0x2C
#define AND_ABSOLUTE 0x2D
#define ROL_ABSOLUTE 0x2E
#define RLA_ABSOLUTE 0x2F

#define BMI_RELATIVE 0x30
#define AND_INDIRECTY 0x31
#define HLT_IMPLIED_32 0x32
#define RLA_INDIRECTY 0x33
#define SKB_IMMEDIATE_34 0x34
#define AND_ZEROPAGEX 0x35
#define ROL_ZEROPAGEX 0x36
#define RLA_ZEROPAGEX 0x37
#define SEC_IMPLIED 0x38
#define AND_ABSOLUTEY 0x39
#define NOP_IMPLIED_3A 0x3A
#define RLA_ABSOLUTEY 0x3B
#define SKW_ABSOLUTEX_3C 0x3C
#define AND_ABSOLUTEX 0x3D
#define ROL_ABSOLUTEX 0x3E
#define RLA_ABSOLUTEX 0x3F


#define RTI_IMPLIED 0x40
#define EOR_INDIRECTX 0x41
#define HLT_IMPLIED_42 0x42
#define SRE_INDIRECTX 0x43
#define SKB_IMMEDIATE_44 0x44
#define EOR_ZEROPAGE 0x45
#define LSR_ZEROPAGE 0x46
#define SRE_ZEROPAGE 0x47
#define PHA_IMPLIED 0x48
#define EOR_IMMEDIATE 0x49
#define LSR_IMPLIED 0x4A
#define ALR_IMMEDIATE 0x4B
#define JMP_ABSOLUTE 0x4C
#define EOR_ABSOLUTE 0x4D
#define LSR_ABSOLUTE 0x4E
#define SRE_ABSOLUTE 0x4F


#define BVC_RELATIVE 0x50
#define EOR_INDIRECTY 0x51
#define HLT_IMPLIED_52 0x52
#define SRE_INDIRECTY 0x53
#define SKB_IMMEDIATE_54 0x54
#define EOR_ZEROPAGEX 0x55
#define LSR_ZEROPAGEX 0x56
#define SRE_ZEROPAGEX 0x57
#define CLI_IMPLIED 0x58
#define EOR_ABSOLUTEY 0x59
#define NOP_IMPLIED_5A 0x5A
#define SRE_ABSOLUTEY 0x5B
#define SKW_ABSOLUTEX_5C 0x5C
#define EOR_ABSOLUTEX 0x5D
#define LSR_ABSOLUTEX 0x5E
#define SRE_ABSOLUTEX 0x5F


#define RTS_IMPLIED 0x60
#define ADC_INDIRECTX 0x61
#define HLT_IMPLIED_62 0x62
#define RRA_INDIRECTX 0x63
#define SKB_IMMEDIATE_64 0x64
#define ADC_ZEROPAGE 0x65
#define ROR_ZEROPAGE 0x66
#define RRA_ZEROPAGE 0x67
#define PLA_IMPLIED 0x68
#define ADC_IMMEDIATE 0x69
#define ROR_IMPLIED 0x6A
#define ARR_IMMEDIATE 0x6B
#define JMP_INDIRECT 0x6C
#define ADC_ABSOLUTE 0x6D
#define ROR_ABSOLUTE 0x6E
#define RRA_ABSOLUTE 0x6F


#define BVS_RELATIVE 0x70
#define ADC_INDIRECTY 0x71
#define HLT_IMPLIED_72 0x72
#define RRA_INDIRECTY 0x73
#define SKB_IMMEDIATE_74 0x74
#define ADC_ZEROPAGEX 0x75
#define ROR_ZEROPAGEX 0x76
#define RRA_ZEROPAGEX 0x77
#define SEI_IMPLIED 0x78
#define ADC_ABSOLUTEY 0x79
#define NOP_IMPLIED_7A 0x7A
#define RRA_ABSOLUTEY 0x7B
#define SKW_ABSOLUTEX_7C 0x7C
#define ADC_ABSOLUTEX 0x7D
#define ROR_ABSOLUTEX 0x7E
#define RRA_ABSOLUTEX 0x7F


#define SKB_IMMEDIATE_80 0x80
#define STA_INDIRECTX 0x81
#define SKB_IMMEDIATE_82 0x82
#define SAX_INDIRECTX 0x83
#define STY_ZEROPAGE 0x84
#define STA_ZEROPAGE 0x85
#define STX_ZEROPAGE 0x86
#define SAX_ZEROPAGE 0x87
#define DEY_IMPLIED 0x88
#define SKB_IMMEDIATE_89 0x89
#define TXA_IMPLIED 0x8A
#define XAA_IMMEDIATE 0x8B
#define STY_ABSOLUTE 0x8C
#define STA_ABSOLUTE 0x8D
#define STX_ABSOLUTE 0x8E
#define SAX_ABSOLUTE 0x8F


#define BCC_RELATIVE 0x90
#define STA_INDIRECTY 0x91
#define HLT_IMPLIED_92 0x92
#define AXA_INDIRECTY 0x93
#define STY_ZEROPAGEX 0x94
#define STA_ZEROPAGEX 0x95
#define STX_ZEROPAGEY 0x96
#define SAX_ZEROPAGEY 0x97
#define TYA_IMPLIED 0x98
#define STA_ABSOLUTEY 0x99
#define TXS_IMPLIED 0x9A
#define TAS_ABSOLUTEY 0x9B
#define SAY_ABSOLUTEX 0x9C
#define STA_ABSOLUTEX 0x9D
#define XAS_ABSOLUTEY 0x9E
#define AXA_ABSOLUTEY 0x9F


#define LDY_IMMEDIATE 0xA0
#define LDA_INDIRECTX 0xA1
#define LDX_IMMEDIATE 0xA2
#define LAX_INDIRECTX 0xA3
#define LDY_ZEROPAGE 0xA4
#define LDA_ZEROPAGE 0xA5
#define LDX_ZEROPAGE 0xA6
#define LAX_ZEROPAGE 0xA7
#define TAY_IMPLIED 0xA8
#define LDA_IMMEDIATE 0xA9
#define TAX_IMPLIED 0xAA
#define OAL_IMMEDIATE 0xAB
#define LDY_ABSOLUTE 0xAC
#define LDA_ABSOLUTE 0xAD
#define LDX_ABSOLUTE 0xAE
#define LAX_ABSOLUTE 0xAF


#define BCS_RELATIVE 0xB0
#define LDA_INDIRECTY 0xB1
#define HLT_IMPLIED_B2 0xB2
#define LAX_INDIRECTY 0xB3
#define LDY_ZEROPAGEX 0xB4
#define LDA_ZEROPAGEX 0xB5
#define LDX_ZEROPAGEY 0xB6
#define LAX_ZEROPAGEY 0xB7
#define CLV_IMPLIED 0xB8
#define LDA_ABSOLUTEY 0xB9
#define TSX_IMPLIED 0xBA
#define LAS_ABSOLUTEY 0xBB
#define LDY_ABSOLUTEX 0xBC
#define LDA_ABSOLUTEX 0xBD
#define LDX_ABSOLUTEY 0xBE
#define LAX_ABSOLUTEY 0xBF


#define CPY_IMMEDIATE 0xC0
#define CMP_INDIRECTX 0xC1
#define SKB_IMMEDIATE_C2 0xC2
#define DCP_INDIRECTX 0xC3
#define CPY_ZEROPAGE 0xC4
#define CMP_ZEROPAGE 0xC5
#define DEC_ZEROPAGE 0xC6
#define DCP_ZEROPAGE 0xC7
#define INY_IMPLIED 0xC8
#define CMP_IMMEDIATE 0xC9
#define DEX_IMPLIED 0xCA
#define SBX_IMMEDIATE 0xCB
#define CPY_ABSOLUTE 0xCC
#define CMP_ABSOLUTE 0xCD
#define DEC_ABSOLUTE 0xCE
#define DCP_ABSOLUTE 0xCF


#define BNE_RELATIVE 0xD0
#define CMP_INDIRECTY 0xD1
#define HLT_IMPLIED_D2 0xD2
#define DCP_INDIRECTY 0xD3
#define SKB_IMMEDIATE_D4 0xD4
#define CMP_ZEROPAGEX 0xD5
#define DEC_ZEROPAGEX 0xD6
#define DCP_ZEROPAGEX 0xD7
#define CLD_IMPLIED 0xD8
#define CMP_ABSOLUTEY 0xD9
#define NOP_IMPLIED_DA 0xDA
#define DCP_ABSOLUTEY 0xDB
#define SKW_ABSOLUTEX_DC 0xDC
#define CMP_ABSOLUTEX 0xDD
#define DEC_ABSOLUTEX 0xDE
#define DCP_ABSOLUTEX 0xDF


#define CPX_IMMEDIATE 0xE0
#define SBC_INDIRECTX 0xE1
#define SKB_IMMEDIATE_E2 0xE2
#define ISB_INDIRECTX 0xE3
#define CPX_ZEROPAGE 0xE4
#define SBC_ZEROPAGE 0xE5
#define INC_ZEROPAGE 0xE6
#define ISB_ZEROPAGE 0xE7
#define INX_IMPLIED 0xE8
#define SBC_IMMEDIATE 0xE9
#define NOP_IMPLIED 0xEA
#define SBC_IMMEDIATE_EB 0xEB
#define CPX_ABSOLUTE 0xEC
#define SBC_ABSOLUTE 0xED
#define INC_ABSOLUTE 0xEE
#define ISB_ABSOLUTE 0xEF


#define BEQ_RELATIVE 0xF0
#define SBC_INDIRECTY 0xF1
#define HLT_IMPLIED_F2 0xF2
#define ISB_INDIRECTY 0xF3
#define SKB_IMMEDIATE_F4 0xF4
#define SBC_ZEROPAGEX 0xF5
#define INC_ZEROPAGEX 0xF6
#define ISB_ZEROPAGEX 0xF7
#define SED_IMPLIED 0xF8
#define SBC_ABSOLUTEY 0xF9
#define NOP_IMPLIED_FA 0xFA
#define ISB_ABSOLUTEY 0xFB
#define SKW_ABSOLUTEX_FC 0xFC
#define SBC_ABSOLUTEX 0xFD
#define INC_ABSOLUTEX 0xFE
#define ISB_ABSOLUTEX 0xFF


#define INT_IRQ					0x1
#define INT_NMI					0x2
#define INT_CLEAR_IRQ			0xFFFFFFFE
#define INT_CLEAR_NMI			0xFFFFFFFD
#define C_FETCH_OPCODE			0x0100
#define C_LOAD_PC				0x0101
#define C_LOAD_PC_2				0x0102
#define C_INDIRECTX				0x0103
#define C_INDIRECTX_READ		C_INDIRECTX
#define C_INDIRECTX_WRITE		C_INDIRECTX
#define C_INDIRECTX_2			0x0104
#define C_INDIRECTX_3			0x0105
#define C_INDIRECTX_4			0x0106
#define C_ZEROPAGE				0x0108
#define C_ZEROPAGE_READ			C_ZEROPAGE
#define C_ZEROPAGE_WRITE		C_ZEROPAGE
#define C_JMP_ABSOLUTE			0x0109
#define C_JMP_ABSOLUTE_2		0x010A
#define C_ABSOLUTE				0x010B
#define C_ABSOLUTE_READ			C_ABSOLUTE
#define C_ABSOLUTE_WRITE		C_ABSOLUTE
#define C_ABSOLUTE_2			0X010C
#define C_ZEROPAGE_READWRITE	0x010D
#define C_ZEROPAGE_READWRITE_2	0x010E
#define C_ZEROPAGE_READWRITE_3	0x010F
#define C_PHP_IMPLIED			0x0110
#define C_PHP_IMPLIED_2			0x0111
#define C_BRK_IMPLIED			0x0112
#define C_BRK_IMPLIED_2			0x0113
#define C_BRK_IMPLIED_3			0x0114
#define C_BRK_IMPLIED_4			0x0115
#define C_PHA_IMPLIED			0x0116
#define C_PHA_IMPLIED_2			0x0117
#define C_ABSOLUTE_READWRITE	0x0118
#define C_ABSOLUTE_READWRITE_2	0x0119
#define C_ABSOLUTE_READWRITE_3	0x011A
#define C_ABSOLUTE_READWRITE_4	0x011B
#define C_BRANCH				0x011C
#define C_BRANCH_2				0x011D
#define C_INDIRECTX_READWRITE	0x011E
#define C_INDIRECTX_READWRITE_2	0x011F
#define C_INDIRECTX_READWRITE_3	0x0120
#define C_INDIRECTX_READWRITE_4	0x0121
#define C_INDIRECTX_READWRITE_5	0x0122
#define C_INDIRECTX_READWRITE_6	0x0123
#define	C_PLA_IMPLIED			0x0124	
#define	C_PLA_IMPLIED_2			0x0125
#define	C_PLA_IMPLIED_3			0x0126
#define	C_PLP_IMPLIED			0x0127
#define	C_PLP_IMPLIED_2			0x0128
#define	C_PLP_IMPLIED_3			0x0129
#define	C_RTI					0x012A
#define	C_RTI_2					0x012B
#define	C_RTI_3					0x012C
#define	C_RTI_4					0x012D
#define	C_RTI_5					0x012E
#define	C_RTS					0x012F
#define	C_RTS_2					0x0130
#define	C_RTS_3					0x0131
#define	C_RTS_4					0x0132
#define	C_RTS_5					0x0133
#define	C_INDIRECTY_READ		0x0134
#define	C_INDIRECTY_READ_2		0x0135
#define	C_INDIRECTY_READ_3		0x0136
#define	C_INDIRECTY_READ_4		0x0137
#define	C_INDIRECTY_READWRITE	0x0138
#define	C_INDIRECTY_READWRITE_2	0x0139
#define	C_INDIRECTY_READWRITE_3	0x013A
#define	C_INDIRECTY_READWRITE_4	0x013B
#define	C_INDIRECTY_READWRITE_5	0x013C
#define	C_INDIRECTY_READWRITE_6	0x013D
#define	C_JSR_ABSOLUTE			0x013E
#define	C_JSR_ABSOLUTE_2		0x013F
#define	C_JSR_ABSOLUTE_3		0x0140
#define	C_JSR_ABSOLUTE_4		0x0141
#define	C_JSR_ABSOLUTE_5		0x0142
#define	C_JSR_ABSOLUTE_6		0x0143
#define	C_JMP_INDIRECT			0x0144
#define	C_JMP_INDIRECT_2		0x0145
#define	C_JMP_INDIRECT_3		0x0146
#define	C_JMP_INDIRECT_4		0x0147
#define	C_ZEROPAGEX				0x0148
#define	C_ZEROPAGEX_READ		C_ZEROPAGEX
#define	C_ZEROPAGEX_WRITE		C_ZEROPAGEX
#define	C_ZEROPAGEX_2			0x0149
#define	C_ZEROPAGEY				0x014A
#define	C_ZEROPAGEY_READ		C_ZEROPAGEY
#define	C_ZEROPAGEY_WRITE		C_ZEROPAGEY
#define	C_ZEROPAGEY_2			0x014B
#define	C_ZEROPAGEX_READWRITE	0x014C
#define	C_ZEROPAGEX_READWRITE_2	0x014D
#define	C_ZEROPAGEX_READWRITE_3	0x014E
#define	C_ZEROPAGEX_READWRITE_4	0x014F
#define	C_INDIRECTY_WRITE		0x0150
#define	C_INDIRECTY_WRITE_2		0x0151
#define	C_INDIRECTY_WRITE_3		0x0152
#define	C_INDIRECTY_WRITE_4		0x0153
#define	C_ABSOLUTEX_READWRITE	0x0154
#define	C_ABSOLUTEX_READWRITE_2	0x0155
#define	C_ABSOLUTEX_READWRITE_3	0x0156
#define	C_ABSOLUTEX_READWRITE_4	0x0157
#define	C_ABSOLUTEX_READWRITE_5	0x0158
#define	C_ABSOLUTEX_READ		0x0159
#define	C_ABSOLUTEX_READ_2		0x015A
#define	C_ABSOLUTEX_READ_3		0x015B
#define	C_ABSOLUTEY_READ		0x015C
#define	C_ABSOLUTEY_READ_2		0x015D
#define	C_ABSOLUTEY_READ_3		0x015E
#define C_ABSOLUTEX_WRITE		0x015F
#define C_ABSOLUTEX_WRITE_2		0x0160
#define C_ABSOLUTEX_WRITE_3		0x0161
#define C_ABSOLUTEY_WRITE		0x0162
#define C_ABSOLUTEY_WRITE_2		0x0163
#define C_ABSOLUTEY_WRITE_3		0x0164
#define C_ABSOLUTEY_READWRITE	0x0165
#define C_ABSOLUTEY_READWRITE_2	0x0166
#define C_ABSOLUTEY_READWRITE_3	0x0167
#define C_ABSOLUTEY_READWRITE_4	0x0168
#define C_ABSOLUTEY_READWRITE_5	0x0169
#define C_NMI					0x016A
#define C_NMI_2					0x016B
#define C_NMI_3					0x016C
#define C_NMI_4					0x016D
#define C_IRQ					0x016E
#define C_IRQ_2					0x016F
#define C_IRQ_3					0x0170
#define C_IRQ_4					0x0171
#define C_ILLEGAL				0x0172
#define C_SEI_IRQ				0x0173
#define C_SLO_2					0x0174
#define C_SKB_IMMEDIATE_2CLK	0x0175
#define C_SKB_IMMEDIATE_3CLK	0x0176
#define C_SKB_IMMEDIATE_3CLK_2	0x0177
#define C_SKB_IMMEDIATE_4CLK	0x0178
#define C_SKB_IMMEDIATE_4CLK_2	0x0179
#define C_SKB_IMMEDIATE_4CLK_3	0x017a
#define C_RESET					0x017b
#define C_RESET_1				0x017c
#define C_RESET_2				0x017d
#define C_RESET_3				0x017e
#define C_RESET_4				0x017f
#define C_RESET_5				0x0180
#define C_RESET_6				0x0181
#define C_RESET_7				0x0182



#define CPU6502_PUSH_PCL WriteByte(mSP-- + 0x0100, mPC.byte.loByte)
#define CPU6502_PUSH_PCH WriteByte(mSP-- + 0x0100, mPC.byte.hiByte)
#define CPU6502_PUSH_PS WriteByte(mSP-- + 0x0100, (fNEGATIVE <<7) | (fOVERFLOW <<6) | (1<<5) | (fBREAK <<4) | (fDECIMAL <<3) | (fINTERRUPT <<2) | (fZERO <<1) | (fCARRY))
#define CPU6502_PUSH_PS_B WriteByte(mSP-- + 0x0100, (fNEGATIVE <<7) | (fOVERFLOW <<6) | (1<<5) | (1 <<4) | (fDECIMAL <<3) | (fINTERRUPT <<2) | (fZERO <<1) | (fCARRY))
#define CPU6502_PUSH_PS_NO_B WriteByte(mSP-- + 0x0100, (fNEGATIVE <<7) | (fOVERFLOW <<6) | (1<<5) | (fDECIMAL <<3) | (fINTERRUPT <<2) | (fZERO <<1) | (fCARRY))
#define CPU6502_PUSH_A WriteByte(mSP-- + 0x0100, mA)

#define CPU6502_SET_PS(v) fNEGATIVE=(v & psNEGATIVE)!=0; \
	fOVERFLOW=(v & psOVERFLOW)!=0; \
	fBREAK=(v & psBREAK)!=0; \
	fDECIMAL=(v & psDECIMAL)!=0; \
	fINTERRUPT=(v & psINTERRUPT)!=0; \
	fZERO =(v & psZERO)!=0; \
	fCARRY=(v & psCARRY)!=0;

#define CPU6502_LOAD_PCL(v) mPC.byte.loByte=v
#define CPU6502_LOAD_PCH(v) mPC.byte.hiByte=v

//using namespace std;

class CPU6502 : public virtual IC6502, public IMonitorCpu, public ErrorMsg
{
public:
	CPU6502() noexcept;
	~CPU6502() = default;
	CPU6502(const CPU6502&) = delete;
	CPU6502& operator=(const CPU6502&) = delete;
	CPU6502(CPU6502&&) = delete;
	CPU6502& operator=(CPU6502&&) = delete;

	HRESULT Init(int ID, IBreakpointManager* pIBreakpointManager);
	void Cleanup();

	virtual bool ReadByte(bit16 address, bit8& data) = 0;
	virtual void WriteByte(bit16 address, bit8 data) = 0;

	//IMonitorCpu
	int GetCpuId() override;
	virtual bit8 MonReadByte(bit16 address, int memorymap) = 0;
	virtual void MonWriteByte(bit16 address, bit8 data, int memorymap) = 0;
	void GetCpuState(CPUState& state) override;
	bool IsBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) override;
	void DeleteBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) override;
	void EnableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) override;
	void DisableBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address) override;
	bool SetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, bool enabled, int initialSkipOnHitCount, int currentSkipOnHitCount) override;
	bool GetBreakpoint(DBGSYM::BreakpointType::BreakpointType bptype, bit16 address, BreakpointItem& breakpoint) override;
	void SetBreakOnInterruptTaken() noexcept override;
	void ClearBreakOnInterruptTaken() noexcept override;
	void SetStepCountBreakpoint(bit64 stepCount) noexcept override;
	void ClearStepCountBreakpoint() noexcept override;
	void SetStepOverBreakpoint() noexcept override;
	void ClearStepOverBreakpoint() noexcept override;
	void SetStepOutWithRtsRtiPlaTsx() noexcept override;
	void ClearStepOutWithRtsRtiPlaTsx() noexcept override;
	void SetStepOutWithRtsRti() override;
	void ClearStepOutWithRtsRti() noexcept override;
	void ClearTemporaryBreakpoints() noexcept override;
	void SetPC(bit16 address) override;
	void SetA(bit8 v) override;
	void SetX(bit8 v) override;
	void SetY(bit8 v) override;
	void SetSR(bit8 v) override;
	void SetSP(bit8 v) override;
	void SetDdr(bit8 v) override;
	void SetData(bit8 v) override;

	void InitReset(ICLK sysclock, bool poweronreset);
	//IRegister
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock) = 0;
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data) = 0;
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) = 0;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;
	int ID;

	virtual void SetIRQ(ICLK sysclock);
	virtual void ClearIRQ();
	virtual void ClearSlowIRQ();
	virtual void SetNMI(ICLK sysclock);
	virtual void ClearNMI();
	virtual void ConfigureMemoryMap();

	void SetRdyLow(ICLK sysclock) override;
	void SetRdyHigh(ICLK sysclock) override;
	void AddClockDelay() override;
	bool IsDebug() override;
	bit16 GetPC() override;
	virtual void PreventClockOverflow();
	virtual void OnHltInstruction();	

	bit16 decode_array[256] = {};
	bit16u mPC = {};
	bit8 mA = 0;
	bit8 mX = 0;
	bit8 mY = 0;
	bit8 mSP = 0;

	unsigned int fNEGATIVE = 0;
	unsigned int fOVERFLOW = 0;
	unsigned int fBREAK = 0;
	unsigned int fDECIMAL = 0;
	unsigned int fINTERRUPT = 0;
	unsigned int fZERO = 0;
	unsigned int fCARRY = 0;

	unsigned int m_cpu_sequence = 0;
	unsigned int m_cpu_final_sequence = 0;
	bit8         m_op_code = 0;
	bit8         m_not_used = 0;

	bit8 PROCESSOR_INTERRUPT = 0;
	bit8 IRQ = 0;
	bit8 NMI = 0;
	bit8 NMI_TRIGGER = 0;
	bit8 RDY = 0;
	bit16u m_CurrentOpcodeAddress = {};
	bool SOTrigger = false;
	ICLK SOTriggerClock = 0;
	bit16 jumpAddress = 0;

	int CheckExecute(bit16 address, bool bHitIt);
	void StartDebug();
	void StopDebug();
	bool IsWriteCycle();
	bool IsOpcodeFetch();
	bool IsInterruptInstruction();
	bool GetBreakOnInterruptTaken() noexcept;

	bit8 m_bDebug = 0;
	bool bSoftResetOnHltInstruction = false;
	bool bHardResetOnHltInstruction = false;
	bool bExitOnHltInstruction = false;

	static const InstructionInfo AssemblyData[256];

	ICLK FirstIRQClock = 0;
	ICLK FirstNMIClock = 0;
	ICLK RisingIRQClock = 0;
	ICLK FirstRdyLowClock = 0;
	ICLK LastRdyHighClock = 0;
	ICLK m_CurrentOpcodeClock = 0;
protected:
	bool m_bRdyLowInClock2OfSEI = false;

	bit16u addr = {};
	bit16u ptr = {};
	bit8 databyte = 0;
	unsigned int v = 0;
	bit8 axa_byte = 0;
	bool m_bBreakOnInterruptTaken = false;

	bit8 code_add(unsigned int _a, unsigned int _s);
	bit8 code_sub(unsigned int _a, unsigned int _s);
	bit8 code_cmp(unsigned int _a, unsigned int _s);
	bit8 code_arr(unsigned int _a, unsigned int _s);

	virtual void SyncChips(bool isWriteCycle) = 0;
	virtual void check_interrupts1();
	virtual void check_interrupts0();
	virtual void SyncVFlag();
	virtual void CheckForCartFreeze();

	void GetState(SsCpuCommon& state);
	void SetState(const SsCpuCommon& state);
private:
	void InitDecoder() noexcept;
	bool m_bEnableStepCountBreakpoint = false;
	bit64 m_StepCountRemaining = 0;
	bool m_bHasStepped = false;
	bool m_bEnableStepOverBreakpoint = false;
	bool m_bStepOverGotNextAddress = false;
	bit16 m_stepOverAddressBreakpoint = 0;
	bool m_bStepOverBreakNextInstruction = false;
	bool m_bEnableStepOutWithRtsRtiPlaPlpTxs = false;
	bool m_bEnableStepOutWithRtsRti = false;
	bit8 m_stepOutStackPointer = 0;
	IBreakpointManager* m_pIBreakpointManager = nullptr;
};
