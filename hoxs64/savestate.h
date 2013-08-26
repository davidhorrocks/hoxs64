#pragma once

#include "vicpixelbuffer.h"

# pragma pack (1)

namespace SsLib
{
	namespace SectionType
	{
		enum SectionType : __int32
		{
			C64Ram = 0,
			C64Cpu = 1,
			C64Cia1 = 2,
			C64Cia2 = 3,
			C64Vic = 4,
			C64Sid = 5,
		};
	}
};

struct SsHeader
{
	char Signature[0x1C];
	bit32 Version;
	bit32 HeaderSize;
	char EmulatorName[0x20];
};

struct SsSectionHeader
{
	bit32 size;
	bit32 id;
};

struct SsCpuCommon
{
	bit32 Model;
	bit16 PC;
	bit8 A;
	bit8 X;
	bit8 Y;
	bit8 SP;
	bit8 SR;
	bit32 CurrentClock;
	bit32 SOTriggerClock;
	bit32 FirstIRQClock;
	bit32 FirstNMIClock;
	bit32 RisingIRQClock;
	bit32 FirstBALowClock;
	bit32 LastBAHighClock;
	bit32 m_CurrentOpcodeClock;
	bit32 m_fade7clock;
	bit32 m_fade6clock;
	bit32 m_cpu_sequence;
	bit32 m_cpu_final_sequence;
	bit32 m_op_code;
	bit16 m_CurrentOpcodeAddress;
	bit16 addr;
	bit16 ptr;
	bit8 databyte;
	bit8 SOTrigger;
	bit8 m_bBALowInClock2OfSEI;
	bit8 BA;
	bit8 PROCESSOR_INTERRUPT;
	bit8 IRQ;
	bit8 NMI;
	bit8 NMI_TRIGGER;
};

struct SsCpuMain
{
	SsCpuCommon common;
	bit8 IRQ_VIC;	
	bit8 IRQ_CIA;
	bit8 IRQ_CRT;
	bit8 NMI_CIA;
	bit8 NMI_CRT;
	bit8 cpu_io_data;
	bit8 cpu_io_ddr;
	bit8 cpu_io_output;
	bit8 CASSETTE_SENSE;
};

struct SsCpuDisk
{
	SsCpuCommon common;
};

struct SsVicSprite
{
	bit32 dataBuffer;
	bit16 bufferPos;
	//The vic sprite x position for the sprite
	bit16 xPos;
	//The vic cycle (1-63) that contains the sprite position. 0 means the sprite has an x position that never matches a the x comparison.
	bit8 column;
	//The vic x position of the start of a vic cycle which may be be less than or equal to the sprite's x position.
	bit16 columnX;
	//The index to the host emulation pixel buffer array for first pixel of the sprite.
	bit16 xPixelInit;
	//The index to the host emulation pixel buffer array
	bit16 xPixel;
	bit16 dataLoadClock;
	bit16 dataLoadedClock;
	bit16 dataLoadIndex;
	bit8 shiftStatus;//enum SpriteShiftRegister
	bit8s shiftCounter;
	bit8 ff_XP;
	bit8 ff_MC;
	bit8 bleedMode;
	bit8 currentPixel;
	bit8 armDelay;
	bit8 drawCount;
	bit8 spriteNumber;
	bit8 spriteBit;
};

class SsVic6569
{
	bit8 cpu_next_op_code;
	// control reg 1
	bit8 vicECM;
	bit8 vicBMM;
	bit8 vicDEN;
	bit8 vicRSEL;
	bit32 vicYSCROLL;
	bit8 vicECM_BMM_MCM;
	bit8 vicECM_BMM_MCM_prev;
	// control reg 2
	bit8 vicRES;
	bit8 vicMCM;
	bit8 vicCSEL;
	bit8 vicXSCROLL;
	bit8 vicXSCROLL_Cycle57;
	//Memory pointers
	bit16 vicMemptrVM;
	bit16 vicMemptrCB;
	//Screen colors
	bit8 vicBorderColor;
	bit8 vicBackgroundColor[16];
	bit8 vicSaveBackgroundColor0;
	//RASTER
	bit16 vicRASTER_compare;

	bit8 vic_raster_cycle; // 1-63
	bit32 vic_raster_line;  // 0-311 PAL
	//INTERRUPT
	bit32 vicINTERRUPT_STATUS;
	bit32 vicINTERRUPT_ENABLE;
	bit8 vic_check_irq_in_cycle2;

	//Sprites
	SsVicSprite vicSprite[8];
	bit16 vicSpriteArmedOrActive;
	bit16 vicSpritePointer[8];
	bit8 ff_YP;
	bit8 vicSpriteDMA;
	bit8 vicSpriteDMAPrev;
	bit8 vicSpriteDisplay;
	bit8 vicSpriteYMatch;
	bit32 vicSpriteData[8];
	bit8 vicSpriteX[8];
	bit8 vicSpriteY[8];
	bit16 MC[8];
	bit16 MCBASE[8];
	bit8 vicClearingYExpandRegInClock15;
	bit8 vicSpriteMSBX;
	bit8 vicSpriteEnable;
	bit8 vicSpriteYExpand;
	bit8 vicSpriteXExpand;
	bit8 vicSpriteXExpandPrev;
	bit8 vicSpriteDataPriority;
	bit8 vicSpriteDataPriorityPrev;
	bit8 vicSpriteMultiColor;
	bit8 vicSpriteMultiColorPrev;
	bit8 vicSpriteSpriteCollision;
	bit8 vicSpriteDataCollision;
	bit8 vicNextSprite_sprite_collision;
	bit8 vicNextSprite_data_collision;
	bit8 vicCurrSprite_sprite_collision;
	bit8 vicCurrSprite_data_collision;
	bit8 vicSpriteSpriteInt;
	bit8 vicSpriteDataInt;
	bit32 clockSpriteMultiColorChange;
	bit32 clockSpriteDataPriorityChange;
	bit32 clockSpriteXExpandChange;
	bit32 clockReadSpriteDataCollision;
	bit32 clockReadSpriteSpriteCollision;

	bit32 clockFirstForcedBadlineCData;

	bit16 vicVC;
	bit16 vicRC;
	bit16 vicVCBASE;
	bit8 vicVMLI;
	bit8s vicAEC;
	bit8 vicIDLE;
	bit8 vicIDLE_DELAY;
	bit32 vicLastCData;
	bit32 vicLastCDataPrev;
	bit32 vicLastCDataPrev2;
	bit32 vicCDataCarry;
	bit8 vicLastGData;
	bit8 vicLastGDataPrev;
	bit8 vicLastGDataPrev2;
	bit32 vic_latchDEN;
	bit8 vic_lpx;
	bit8 vic_lpy;
	bit8 vicDRAMRefresh;
	bit8 LP_TRIGGER;
	bit8 vicLightPen;

	bit8 m_bVicBankChanging;
	bit8 vicBankChangeByte;

	bit8 de00_byte;

	bit8 ScreenPixelBuffer[PIXELBUFFER_COUNT][PAL_MAX_LINE+1][PIXELBUFFER_SIZE+1];
	bit8 LinePixelBuffer[PIXELBUFFER_COUNT][PIXELBUFFER_SIZE+1];
	bit32 FrameNumber;

	bit8 bVicRasterMatch;
	bit8 vic_in_display_y;
	bit8 vic_badline;
	bit8 vicBA_new;
	bit32 vic_top_compare;
	bit32 vic_bottom_compare;
	bit32 vic_left_compare;
	bit32 vic_right_compare;
	bit8 vicMainBorder;
	bit8 vicVerticalBorder; 
	bit8 vicCharDataOutputDisabled; 
	bit8 vicMainBorder_old;

	bit8 vic_border_part_38;
	bit8 vic_border_part_40;

	bit8 pixelMaskBuffer[(PIXELBUFFER_SIZE + 1) / 8];
	bit8 vic_sprite_collision_line[PIXELBUFFER_SIZE + 1];
	bit8 vic_allow_c_access;
	bit8 m_bVicModeChanging;
	bit32 VideoMatrix[NUM_SCREEN_COLUMNS];

	bit8 vicMemoryBankIndex;

	int m_iLastBackedUpFrameNumber;
};

struct SsCia
{
	bit32 DevicesClock;
	bit32 delay;
	bit32 feed;
	bit32 old_delay,old_feed;
	bit32 idle;
	bit16 dec_a,dec_b;
	bit32 no_change_count;
	bit32 flag_change;
	bit32 sp_change;
	bit32 f_flag_in;
	bit32 f_flag_out;
	bit32 f_sp_in;
	bit32 f_sp_out;
	bit32 f_cnt_in;
	bit32 f_cnt_out;
	bit8 pra_out;
	bit8 prb_out;
	bit8 ddra;
	bit8 ddrb;
	bit16u ta_counter;
	bit16u tb_counter;
	bit16u ta_latch;
	bit16u tb_latch;
	ICLKS tod_clock_reload;
	ICLKS tod_clock_rate;
	ICLKS tod_tick;
	ICLKS tod_clock_compare_band;
	bit8 tod_alarm;
	bit8 tod_read_freeze;	
	cia_tod tod_read_latch;
	bit8 tod_write_freeze;
	cia_tod tod_write_latch;
	cia_tod tod;
	cia_tod alarm;
	bit8 sdr;
	bit8 cra;
	bit8 crb;
	bit32 timera_output;
	bit32 timerb_output;
	bit8 icr;
	bit8 imr;
	bit8 Interrupt;
	bit8 serial_int_count;
	bit8 bEarlyIRQ;
	bit8 bTimerBbug;
	bit32 ClockReadICR;
	bit8 bPB67TimerMode;
	bit8 bPB67TimerOut;
	bit8 bPB67Toggle;
};

struct SsCia1
{
	SsCia cia;
};

struct SsCia2
{
	SsCia cia;
	bit8 c64_serialbus;
	bit8 m_commandedVicBankIndex;
};

struct SsSidVoice
{
	DWORD counter;
	DWORD frequency;
	WORD volume;
	DWORD sampleHoldDelay;
	BYTE envmode;
	BYTE wavetype;
	BYTE sync;
	BYTE ring_mod;
	BYTE sustain_level;
	BYTE zeroTheCounter;
	BYTE exponential_counter_period;
	DWORD attack_value;
	DWORD decay_value;
	DWORD release_value;
	DWORD pulse_width_counter;
	DWORD pulse_width_reg;
	bit16 sampleHold;
	bit16 lastSample;
	double fVolSample;
	BYTE gate;
	BYTE test;
	bit32 sidShiftRegister;
	BYTE keep_zero_volume;
	bit16 envelope_counter;
	bit16 envelope_compare;
	bit8 exponential_counter;
	bit8 control;
	bit32s shifterTestCounter;
};

struct SsDiskInterface
{
	bit8 m_d64_serialbus;
	bit8 m_d64_dipswitch;
	bit8 m_d64_protectOff;
	bit8 m_d64_sync;
	bit8 m_d64_forcesync;
	bit8 m_d64_soe_enable;
	bit8 m_d64_write_enable;
	bit8 mi_d64_diskchange;
	bit8 m_d64TrackCount;
	bit8 m_c64_serialbus_diskview;
	bit8 m_c64_serialbus_diskview_next;
	ICLKS m_diskChangeCounter;
	ICLK CurrentPALClock;
	ICLK m_changing_c64_serialbus_diskview_diskclock;
	ICLK m_driveWriteChangeClock;
	ICLK m_motorOffClock;
	ICLK m_headStepClock;
	ICLK m_pendingclocks;
	ICLK m_DiskThreadCommandedPALClock;
	ICLK m_DiskThreadCurrentPALClock;

	bit8 m_d64_diskwritebyte;

	bool m_bDiskMotorOn;
	bool m_bDriveLedOn;
	bool m_bDriveWriteWasOn;
	bit8 m_diskLoaded;
	bit32 m_currentHeadIndex;
	bit8 m_currentTrackNumber;
	char m_lastHeadStepDir;
	bit8 m_lastHeadStepPosition;
	bit8 m_shifterWriter_UD3; //74LS165 UD3
	bit16 m_shifterReader_UD2; //74LS164 UD2 Two flops make the shifter effectively 10 bits. The lower 8 bits are read by VIA2 port A.
	ICLK m_busDataUpdateClock;
	bit16 m_busByteReadyPreviousData;
	bit8 m_busByteReadySignal;
	bit8 m_frameCounter_UC3; //74LS191 UC3
	bit8 m_debugFrameCounter;
	bit8 m_clockDivider1_UE7_Reload;
	bit8 m_clockDivider1_UE7; //74LS193 UE7 16MHz
	bit8 m_clockDivider2_UF4; //74LS193 UF4

	bit8 m_writeStream;
	unsigned int m_totalclocks_UE7;
	unsigned int m_lastPulseTime;

	//bit8 *m_rawTrackData[G64_MAX_TRACKS];

	__int64 m_diskd64clk_xf;
};

struct SsViaCommon
{
	int ID;
	ICLK DevicesClock;
	
	bool bLatchA;
	bool bLatchB;

	bit8 ora;
	bit8 ira;
	bit8 orb;
	bit8 irb;
	bit8 ddra;
	bit8 ddrb;
	bit16u timer1_counter;
	bit16u timer2_counter;
	bit16u timer1_latch;
	bit16u timer2_latch;
	bit8 acr;
	bit8 pcr;
	bit8 ca1_in;
	bit8 ca1_in_prev;
	bit8 ca2_in;
	bit8 ca2_in_prev;
	bit8 cb1_in;
	bit8 cb1_in_prev;
	bit8 cb2_in;
	bit8 cb2_in_prev;
	bit8 ca2_out;
	bit8 cb2_out;
	bit8 shift;
	bit8 ifr;
	bit8 ier;
	bit8 serial_active;
	bit8 serial_mode;
	unsigned __int64 delay;
	unsigned __int64 feed;
	unsigned __int64 old_delay;
	unsigned __int64 old_feed;
	bit8 modulo;
	bit8 Interrupt;

	bit8 bPB7TimerMode;
	bit8 bPB7Toggle;
	bit8 bPB7TimerOut;

	bit8 no_change_count;
	bit16 dec_2;
	bit8 idle;
};

struct SsVia1
{
	SsViaCommon via;
};

struct SsVia2
{
	SsViaCommon via;
	bit8 oldDiskControl;
};

# pragma pack ()

class SaveState
{
public:
	static const char SIGNATURE[];
	static const char NAME[];
	static const int SIZE64K = 0x10000;

	template<class T>
	static HRESULT SaveSection(IStream *pfs, const T& section, SsLib::SectionType::SectionType sectionType);
};

template<class T>
HRESULT SaveState::SaveSection(IStream *pfs, const T& section, SsLib::SectionType::SectionType sectionType)
{
HRESULT hr;
ULONG bytesWritten;
SsSectionHeader sh;

	sh.size = sizeof(section);
	sh.id = sectionType;

	hr = pfs->Write(&sh, sizeof(sh), &bytesWritten);
	if (SUCCEEDED(hr))
	{
		hr = pfs->Write(&section, sizeof(section), &bytesWritten);
	}
	return hr;
}