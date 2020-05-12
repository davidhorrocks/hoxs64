#pragma once
#define bitCountA0		0
#define bitCountA1		1
#define bitCountA2		2
#define bitCountA3		3
#define bitCountB0		4
#define bitCountB1		5
#define bitCountB2		6
#define bitCountB3		7
#define bitLoadA0		8
#define bitLoadA1		9
#define bitLoadA2		10
#define bitLoadB0		11
#define bitLoadB1		12
#define bitLoadB2		13
#define bitPB6Low0		14
#define bitPB6Low1		15
#define bitPB7Low0		16
#define bitPB7Low1		17
#define bitInterrupt0	18
#define bitInterrupt1	19
#define bitOneShotA0	20
#define bitOneShotB0	21

#define CountA0     0x00000001ULL
#define CountA1     0x00000002ULL
#define CountA2     0x00000004ULL
#define CountA3     0x00000008ULL
#define CountB0     0x00000010ULL
#define CountB1     0x00000020ULL
#define CountB2     0x00000040ULL
#define CountB3     0x00000080ULL
#define LoadA0      0x00000100ULL
#define LoadA1      0x00000200ULL
#define LoadA2      0x00000400ULL
#define LoadB0      0x00000800ULL
#define LoadB1      0x00001000ULL
#define LoadB2      0x00002000ULL
#define PB6Low0     0x00004000ULL
#define PB6Low1     0x00008000ULL
#define PB7Low0     0x00010000ULL
#define PB7Low1     0x00020000ULL
#define Interrupt0  0x00040000ULL
#define Interrupt1  0x00080000ULL
#define OneShotA0   0x00100000ULL
#define OneShotB0   0x00200000ULL
#define ClearIcr0   0x00400000ULL
#define ClearIcr1   0x00800000ULL
#define ClearIcr2   0x01000000ULL
#define ReadIcr0    0x02000000ULL
#define ReadIcr1    0x04000000ULL
#define ReadIcr2    0x08000000ULL
#define WriteIcr0   0x10000000ULL
#define WriteIcr1   0x20000000ULL
#define WriteIcr2   0x40000000ULL
#define SetIcr0     0x80000000ULL
#define SetIcr1       0x0000000100000000ULL
#define SetIcr2       0x0000000200000000ULL
#define SetCntFlip0   0x0000000400000000ULL
#define SetCntFlip1   0x0000000800000000ULL
#define SetCntFlip2   0x0000001000000000ULL
#define SetCntFlip3   0x0000002000000000ULL
#define SetCnt0       0x0000004000000000ULL
#define SetCnt1       0x0000008000000000ULL
#define SetCnt2       0x0000010000000000ULL
#define SetCnt3       0x0000020000000000ULL
#define DelayMask   ~(0x0000040000000000ULL | CountA0 | CountB0 | LoadA0 | LoadB0 | PB6Low0 | PB7Low0 | Interrupt0 | OneShotA0 | OneShotB0 | ClearIcr0 | ReadIcr0 | WriteIcr0 | SetIcr0 | SetCntFlip0 | SetCnt0)

#define CIA_MAX_TEMPERATURE_TIME (1000000 * 60)

class CIA : public IRegister
{
public:
	CIA();
	~CIA() = default;
	CIA(const CIA&) = delete;
	CIA& operator=(const CIA&) = delete;
	CIA(CIA&&) = delete;
	CIA& operator=(CIA&&) = delete;

	//IRegister
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	virtual bit8 ReadPortA()=0;
	virtual bit8 ReadPortB()=0;
	virtual void WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new)=0;
	virtual void WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new)=0;
	virtual void SetSystemInterrupt()=0;
	virtual void ClearSystemInterrupt()=0;
	virtual void ExecuteDevices(ICLK sysclock)=0;

	virtual void SetWakeUpClock();
	bit8 PortAOutput_Strong0s();
	bit8 PortBOutput_Strong0s();
	void incrementTOD();
	bit8 PortAOutput_Strong1s();
	bit8 PortBOutput_Strong1s();
	void Pulse(ICLK sysclock);
	void SetMode(HCFG::CIAMODE mode, bool bTimerBbug);
	void PreventClockOverflow();
	void SetTODWakeUpClock();
	void CheckTODAlarmCompare(ICLK sysclock);
	bool ReadCntPinLevel();
	bool ReadSpPinLevel();
	void CntChanged();
	bool SetSerialCntOut(bool value);
	void SetSerialSpOut(bool value);

	static long GetTenthsFromTimeToAlarm(const cia_tod &time, const cia_tod &alarm);
	static int prec_bitcount[256];
	static void init_bitcount();

	ICLK DevicesClock = {};
	int ID = 0;
	bit64 delay = 0;
	bit64 feed = 0;
	bit64 delay_aux_mask = 0;
	bit64 old_delay = 0;
	bit64 old_feed = 0;
	bool idle = false;
	bit16 dec_a = 0;
	bit16 dec_b = 0;
	bit32 no_change_count = 0;
	bool flag_change = false;
	bool f_flag_in = false;
	bool f_sp_in = false;
	bool f_sp_out = false;
	bool f_cnt_in = false;
	bool f_cnt_out = false;
	bit8 pra_out = 0;
	bit8 prb_out = 0;
	bit8 ddra = 0;
	bit8 ddrb = 0;
	bit16u ta_counter = {};
	bit16u tb_counter = {};
	bit16u ta_latch = {};
	bit16u tb_latch = {};
	ICLKS tod_clock_reload = 0;
	ICLKS tod_clock_rate = 0;
	ICLKS tod_tick = 0;
	volatile bit8 tod_alarm = 0;
	volatile bit8 tod_read_freeze = 0;
	volatile cia_tod tod_read_latch = {};
	volatile bit8 tod_write_freeze = {};
	volatile cia_tod tod_write_latch = {};
	volatile cia_tod tod = {};
	volatile cia_tod alarm = {};
	bit8 cra = 0;
	bit8 crb = 0;
	bit32 timera_output = 0;
	bit32 timerb_output = 0;
	bit8 icr = 0;
	bit8 icr_ack = 0;
	bit8 imr = 0;
	bit8 Interrupt = 0;
	bit8 serial_data_register = 0;
	bit8 serial_shift_buffer = 0;
	bit8 serial_int_count = 0;
	bool serial_data_write_pending = false;
	bool serial_data_write_loading = false;
	bool serial_shiftregister_loaded = false;
	bit32 serial_interrupt_delay = 0;
	bool serial_interrupt_on_direction_change = false;
	bool bEarlyIRQ = false;
	bool bTimerBbug = false;
	ICLK ClockReadICR = 0;
	unsigned char bPB67TimerMode = 0;
	unsigned char bPB67TimerOut = 0;
	unsigned char bPB67Toggle = 0;
	ICLK ClockNextWakeUpClock = 0;
	ICLK ClockNextTODWakeUpClock = 0;
	bool is_warm = false;
protected:
	void InitCommonReset(ICLK sysclock, bool poweronreset);
	void GetCommonState(SsCiaV2 &state);
	void SetCommonState(const SsCiaV2 &state);
	static void UpgradeStateV0ToV1(const SsCiaV0 &in, SsCiaV1 &out);
	static void UpgradeStateV1ToV2(const SsCiaV1 &in, SsCiaV2 &out);
	static void UpgradeStateV2ToV3(const SsCiaV2& in, SsCiaV2& out);

	random_device rd;
	mt19937 randengine;
};
