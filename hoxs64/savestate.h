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
			C64ColourRam = 1,
			C64Cpu = 2,
			C64Cia1V0 = 3,
			C64Cia2V0 = 4,
			C64Vic = 5,
			C64Sid = 6,
			C64KernelRom = 7,
			C64BasicRom = 8,
			C64CharRom = 9,
			C64Tape = 10,
			C64TapeData = 11,
			DriveRam = 12,
			DriveCpu = 13,
			DriveVia1 = 14,
			DriveVia2 = 15,
			DriveControllerV0 = 16,
			DriveDiskImageV0 = 17,
			DriveRom = 18,
			DriveTrackData = 19,
			Cart = 20,
			DriveControllerV1 = 21,
			DriveDiskImageV1 = 22,
			C64Cia1V1 = 23,
			C64Cia2V1 = 24,
			DriveControllerV2 = 25,
			C64SidV1 = 26,
			C64Cia1V2 = 27,
			C64Cia2V2 = 28,
			C64SidV2 = 29,
			C64SidV3 = 30
		};
	}
};

struct SsDataChunkHeader
{
	bit32 byteCount;
	bit32 compressionType;
};

struct SsHeader
{
	char Signature[0x1C];
	bit32 Version;
	bit32 HeaderSize;
	char EmulatorName[0x20];
	bit32 reserved1;
	bit32 reserved2;
	bit32 reserved3;
	bit32 reserved4;
};

struct SsSectionHeader
{
	bit32 size;
	bit32 id;
	bit32 version;
	bit32 reserved1;
	bit32 reserved2;
};

struct SsTrackHeader
{
	bit32 size;
	bit32 number;
	bit32 version;
	bit32 gap_count;
	bit32 reserved1;
};

struct SsEmulation
{
	bit8 Cia1Model;
	bit8 Cia2Model;
	bit8 SidEmuationEnabled;
	bit8 DriveEmuationEnabled;
	bit8 DriveDiskInserted;
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
	bit32 m_fade7clock;
	bit32 m_fade6clock;
	bit8 IRQ_VIC;	
	bit8 IRQ_CIA;
	bit8 IRQ_CRT;
	bit8 NMI_CIA;
	bit8 NMI_CRT;
	bit8 cpu_io_data;
	bit8 cpu_io_ddr;
	bit8 cpu_io_output;
	bit8 cpu_io_readoutput;
	bit8 LORAM;
	bit8 HIRAM;
	bit8 CHAREN;
	bit8 CASSETTE_WRITE;
	bit8 CASSETTE_MOTOR;
	bit8 CASSETTE_SENSE;
};

struct SsCpuDisk
{
	SsCpuCommon common;
	bit8 IRQ_VIA1;
	bit8 IRQ_VIA2;
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

struct SsVic6569
{
	bit32 CurrentClock;
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

struct SsCiaV0
{
	bit32 CurrentClock;
	bit32 DevicesClock;
	bit32 ClockNextWakeUpClock;
	bit32 ClockNextTODWakeUpClock;
	bit32 delay;
	bit32 feed;
	bit32 old_delay;
	bit32 old_feed;
	bit32 idle;
	bit16 dec_a;
	bit16 dec_b;
	bit32 no_change_count;
	bit32 flag_change;
	bit32 serial_interrupt_delay;	
	bit32 delay_aux_mask;
	bit8 serial_shift_buffer;
	bit8 serial_data_write_pending;
	bit8 serial_data_write_loading;
	bit8 serial_other;
	bit32 int32_buffer0;
	bit32 int32_buffer1;
	bit32 int32_buffer2;
	bit8 f_sp_in;
	bit8 f_sp_out;
	bit8 f_cnt_in;
	bit8 f_cnt_out;
	bit8 pra_out;
	bit8 prb_out;
	bit8 ddra;
	bit8 ddrb;
	bit16u ta_counter;
	bit16u tb_counter;
	bit16u ta_latch;
	bit16u tb_latch;
	bit32s tod_clock_reload;
	bit32s tod_clock_rate;
	bit32s tod_tick;
	bit32s tod_clock_compare_band;
	bit8 tod_alarm;
	bit8 tod_read_freeze;	
	cia_tod tod_read_latch;
	bit8 tod_write_freeze;
	cia_tod tod_write_latch;
	cia_tod tod;
	cia_tod alarm;
	bit8 serial_data_register;
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

struct SsCiaV1 : SsCiaV0
{
	bit8 icr_ack;
};

struct SsCiaV2
{
	bit32 CurrentClock;
	bit32 DevicesClock;
	bit32 ClockNextWakeUpClock;
	bit32 ClockNextTODWakeUpClock;
	bit64 delay;
	bit64 feed;
	bit64 old_delay;
	bit64 old_feed;
	bit8 idle;
	bit16 dec_a;
	bit16 dec_b;
	bit32 no_change_count;
	bit32 flag_change;
	bit32 serial_interrupt_delay;	
	bit64 delay_aux_mask;
	bit8 serial_shift_buffer;
	bit8 serial_data_write_pending;
	bit8 serial_data_write_loading;
	bit8 serial_other;
	bit32 int32_buffer0;
	bit32 int32_buffer1;
	bit32 int32_buffer2;
	bit8 f_sp_in;
	bit8 f_sp_out;
	bit8 f_cnt_in;
	bit8 f_cnt_out;
	bit8 pra_out;
	bit8 prb_out;
	bit8 ddra;
	bit8 ddrb;
	bit16u ta_counter;
	bit16u tb_counter;
	bit16u ta_latch;
	bit16u tb_latch;
	bit32s tod_clock_reload;
	bit32s tod_clock_rate;
	bit32s tod_tick;
	bit32s tod_clock_compare_band;
	bit8 tod_alarm;
	bit8 tod_read_freeze;	
	cia_tod tod_read_latch;
	bit8 tod_write_freeze;
	cia_tod tod_write_latch;
	cia_tod tod;
	cia_tod alarm;
	bit8 serial_data_register;
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
	bit8 icr_ack;
};

struct SsCia1V0
{
	SsCiaV0 cia;
	bit32 nextKeyboardScanClock;
};

struct SsCia1V1
{
	SsCiaV1 cia;
	bit32 nextKeyboardScanClock;
};

struct SsCia1V2
{
	SsCiaV2 cia;
	bit32 nextKeyboardScanClock;
};

struct SsCia2V0
{
	SsCiaV0 cia;
	bit8 c64_serialbus;
	bit8 m_commandedVicBankIndex;
};

struct SsCia2V1
{
	SsCiaV1 cia;
	bit8 c64_serialbus;
	bit8 m_commandedVicBankIndex;
};

struct SsCia2V2
{
	SsCiaV2 cia;
	bit8 c64_serialbus;
	bit8 m_commandedVicBankIndex;
};

struct SsSidVoice
{
	bit32 counter;
	bit32 frequency;
	bit16 volume;
	bit32 sampleHoldDelay;
	bit8 envmode;
	bit8 wavetype;
	bit8 sync;
	bit8 ring_mod;
	bit8 sustain_level;
	bit8 zeroTheCounter;
	bit8 exponential_counter_period;
	bit32 attack_value;
	bit32 decay_value;
	bit32 release_value;
	bit32 pulse_width_counter;
	bit32 pulse_width_reg;
	bit16 sampleHold;
	bit16 lastSample;
	double fVolSample;
	bit8 gate;
	bit8 test;
	bit32 sidShiftRegister;
	bit8 keep_zero_volume;
	bit16 envelope_counter;
	bit16 envelope_compare;
	bit8 exponential_counter;
	bit8 control;
	bit32s shifterTestCounter;	
};

struct SsSidVoiceV1 : SsSidVoice
{
	bit32 phaseOfShiftRegister;
	bit16 noiseFeedbackSample1;
};

struct SsSidVoiceV2 : SsSidVoiceV1
{
	bit8 nextvolume;
	bit8 samplevolume;
	bit8 next_envmode;
	bit8 envmode_changing_delay;
	bit8 envelope_count_delay;
	bit8 exponential_count_delay;
	bit8 next_exponential_counter_period;
	bit8 gotNextVolume;
	bit8 reset_envelope_counter;
	bit8 reset_exponential_counter;
	bit8 envelope_tick;
};

struct SsSidVoiceV3 : SsSidVoiceV2
{
	bit8 latched_envmode;
	bit8 want_latched_envmode;
	bit32 countertest;
	bit32 sidShiftRegisterFill;
	bit16 noiseFeedbackSample1;
	bit16 noiseFeedbackSample2;
	bit16 noiseFeedbackMask1;
	bit16 noiseFeedbackMask0;
	bit8 zeroTheShiftRegister;
};

struct SsSid
{
	bit32 CurrentClock;
	SsSidVoice voice1;
	SsSidVoice voice2;
	SsSidVoice voice3;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	bit8 sidBlock_Voice3;
	bit8 sidInternalBusByte;
	bit32 sidReadDelay;
};

struct SsSidV1
{
	bit32 CurrentClock;
	SsSidVoiceV1 voice1;
	SsSidVoiceV1 voice2;
	SsSidVoiceV1 voice3;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	bit8 sidBlock_Voice3;
	bit8 sidInternalBusByte;
	bit32 sidReadDelay;
};

struct SsSidV2
{
	bit32 CurrentClock;
	SsSidVoiceV2 voice1;
	SsSidVoiceV2 voice2;
	SsSidVoiceV2 voice3;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	bit8 sidBlock_Voice3;
	bit8 sidInternalBusByte;
	bit32 sidReadDelay;
};

struct SsSidV3
{
	bit32 CurrentClock;
	SsSidVoiceV3 voice1;
	SsSidVoiceV3 voice2;
	SsSidVoiceV3 voice3;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	bit8 sidBlock_Voice3;
	bit8 sidInternalBusByte;
	bit32 sidReadDelay;
};

struct SsDiskInterfaceV0
{
	bit32 CurrentClock;
	bit8 m_d64_serialbus;
	bit8 m_d64_dipswitch;
	bit8 m_d64_protectOff;
	bit8 m_d64_sync;
	bit8 m_d64_forcesync;
	bit8 m_d64_soe_enable;
	bit8 m_d64_write_enable;
	bit8 m_d64_diskchange_counter;
	bit8 m_d64TrackCount;
	bit8 m_c64_serialbus_diskview;
	bit8 m_c64_serialbus_diskview_next;
	bit32s m_diskChangeCounter;
	bit32 CurrentPALClock;
	bit32 m_changing_c64_serialbus_diskview_diskclock;
	bit32 m_driveWriteChangeClock;
	bit32 m_motorOffClock;
	bit32 m_headStepClock;
	bit32 m_pendingclocks;
	bit32 m_DiskThreadCommandedPALClock;
	bit32 m_DiskThreadCurrentPALClock;
	bit8 m_d64_diskwritebyte;
	bit8 m_bDiskMotorOn;
	bit8 m_bDriveLedOn;
	bit8 m_bDriveWriteWasOn;
	bit8 m_diskLoaded;
	bit32 m_currentHeadIndex;
	bit8 m_currentTrackNumber;
	bit8s m_lastHeadStepDir;
	bit8 m_lastHeadStepPosition;
	bit8 m_shifterWriter_UD3; //74LS165 UD3
	bit16 m_shifterReader_UD2; //74LS164 UD2 Two flops make the shifter effectively 10 bits. The lower 8 bits are read by VIA2 port A.
	bit32 m_busDataUpdateClock;
	bit16 m_busByteReadyPreviousData;
	bit8 m_busByteReadySignal;
	bit8 m_frameCounter_UC3; //74LS191 UC3
	bit8 m_debugFrameCounter;
	bit8 m_clockDivider1_UE7_Reload;
	bit8 m_clockDivider1_UE7; //74LS193 UE7 16MHz
	bit8 m_clockDivider2_UF4; //74LS193 UF4
	bit8 m_writeStream;
	bit32 m_totalclocks_UE7;
	bit32 m_lastPulseTime;
	__int64 m_diskd64clk_xf;
};

struct SsDiskInterfaceV1 : SsDiskInterfaceV0
{
	bit8 m_bPendingPulse;
	bit8 m_bPulseState;
	bit8 m_bLastPulseState;
	bit32s m_nextP64PulsePosition;
	bit32 m_headStepClockUp;
};

struct SsDiskInterfaceV2 : SsDiskInterfaceV1
{
	bit8 m_previousTrackNumber;
	bit32 m_lastWeakPulseTime;
	bit32 m_counterStartPulseFilter;
};

struct SsViaCommon
{
	bit32s ID;
	bit32 CurrentClock;
	bit32 DevicesClock;
	
	bit8 bLatchA;
	bit8 bLatchB;

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

struct SsTape
{
	bit32 CurrentClock;
	bit32 tape_max_counter;
	bit32 tape_position;
	bit32 tape_pulse_length;
	bit32 nextTapeTickClock;
	bit32 bMotorOn;
	bit8 bPlayDown;
	bit8 bRecordDown;
	bit8 bEOT;
	bit8 bEOD;
};

struct SsTapeData
{
	bit32 tape_max_counter;
};

# pragma pack ()

class SaveState
{
public:
    static const int VERSION = 6;
	static const char SIGNATURE[];
	static const char NAME[];
	static const int SIZE64K = 0x10000;
	static const int SIZECOLOURAM = 0x400;
	static const int SIZEC64KERNEL = 0x2000;
	static const int SIZEC64BASIC = 0x2000;
	static const int SIZEC64CHARGEN = 0x1000;
	static const int SIZEDRIVERAM = 0x0800;
	static const int SIZEDRIVEROM = 0x4000;

	template<class T>
	static HRESULT SaveSection(IStream *pfs, const T& section, SsLib::SectionType::SectionType sectionType);
};

template<class T>
HRESULT SaveState::SaveSection(IStream *pfs, const T& section, SsLib::SectionType::SectionType sectionType)
{
HRESULT hr;
ULONG bytesWritten;
SsSectionHeader sh;

	ZeroMemory(&sh, sizeof(sh));
	sh.size = sizeof(section) + sizeof(sh);
	sh.id = sectionType;
	sh.version = 0;

	hr = pfs->Write(&sh, sizeof(sh), &bytesWritten);
	if (SUCCEEDED(hr))
	{
		hr = pfs->Write(&section, sizeof(section), &bytesWritten);
	}
	return hr;
}