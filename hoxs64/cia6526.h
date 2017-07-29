#ifndef __CIA6526_H__
#define __CIA6526_H__

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
#define ReadIcr0    0x01000000ULL
#define ReadIcr1    0x02000000ULL
#define WriteIcr0   0x04000000ULL
#define WriteIcr1   0x08000000ULL
#define SetIcr0     0x10000000ULL
#define SetIcr1     0x20000000ULL

#define SetCntFlip0   0x0000000040000000ULL
#define SetCntFlip1   0x0000000080000000ULL
#define SetCntFlip2   0x0000000100000000ULL
#define SetCntFlip3   0x0000000200000000ULL
#define SetCnt0       0x0000000400000000ULL
#define SetCnt1       0x0000000800000000ULL
#define SetCnt2       0x0000001000000000ULL
#define SetCnt3       0x0000002000000000ULL
#define DelayMask   ~(0x0000004000000000ULL | CountA0 | CountB0 | LoadA0 | LoadB0 | PB6Low0 | PB7Low0 | Interrupt0 | OneShotA0 | OneShotB0 | ClearIcr0 | ReadIcr0 | WriteIcr0 | SetIcr0 | SetCntFlip0 | SetCnt0)


class CIA : public IRegister
{
public:
	CIA();

	void InitReset(ICLK sysclock, bool poweronreset);
	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	bit8 PortAOutput_Strong0s();
	bit8 PortBOutput_Strong0s();
	void incrementTOD();

	bit8 PortAOutput_Strong1s();
	bit8 PortBOutput_Strong1s();

	virtual bit8 ReadPortA()=0;
	virtual bit8 ReadPortB()=0;
	virtual void WritePortA()=0;
	virtual void WritePortB()=0;
	virtual void SetSystemInterrupt()=0;
	virtual void ClearSystemInterrupt()=0;

	ICLK DevicesClock;
	virtual void ExecuteDevices(ICLK sysclock)=0;

	void Pulse(ICLK sysclock);

	void SetMode(HCFG::CIAMODE mode, bool bTimerBbug);

	void PreventClockOverflow();
	int ID;
	bit64 delay;
	bit64 feed;
	bit64 delay_aux_mask;
	bit64 old_delay;
	bit64 old_feed;
	bool idle;
	bit16 dec_a;
	bit16 dec_b;
	bit32 no_change_count;
	bool flag_change;
	bool f_flag_in;
	bool f_sp_in;
	bool f_sp_out;
	bool f_cnt_in;
	bool f_cnt_out;
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
	volatile bit8 tod_alarm;
	volatile bit8 tod_read_freeze;	
	volatile cia_tod tod_read_latch;
	volatile bit8 tod_write_freeze;
	volatile cia_tod tod_write_latch;
	volatile cia_tod tod;
	volatile cia_tod alarm;
	bit8 cra;
	bit8 crb;
	bit32 timera_output;
	bit32 timerb_output;
	bit8 icr;
	bit8 icr_ack;
	bool fast_clear_pending_int;
	bit8 imr;
	bit8 Interrupt;
	bit8 serial_data_register;
	bit8 serial_shift_buffer;
	bit8 serial_int_count;
	bool serial_data_write_pending;
	bool serial_data_write_loading;
	bit32 serial_interrupt_delay;
	bool bEarlyIRQ;
	bool bTimerBbug;
	ICLK ClockReadICR;
	unsigned char bPB67TimerMode;
	unsigned char bPB67TimerOut;
	unsigned char bPB67Toggle;
	ICLK ClockNextWakeUpClock;
	ICLK ClockNextTODWakeUpClock;
	virtual void SetWakeUpClock();
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

protected:
	void GetState(SsCiaV2 &state);
	void SetState(const SsCiaV2 &state);
	static void UpgradeStateV0ToV1(const SsCiaV0 &in, SsCiaV1 &out);
	static void UpgradeStateV1ToV2(const SsCiaV1 &in, SsCiaV2 &out);
};


#endif
