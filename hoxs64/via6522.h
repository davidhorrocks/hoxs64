#pragma once
#define VIA_SER_DISABLED	(0x0)
#define VIA_SER_IN_T2		(0x1)
#define VIA_SER_IN_02		(0x2)
#define VIA_SER_IN_CB1		(0x3)
#define VIA_SER_OUT_T2_CIRC	(0x4)
#define VIA_SER_OUT_T2		(0x5)
#define VIA_SER_OUT_02		(0x6)
#define VIA_SER_OUT_CB1		(0x7)

#define VIA_INT_CA2 1
#define VIA_INT_CA1	2
#define VIA_INT_SER	4
#define VIA_INT_CB2	8
#define VIA_INT_CB1	16
#define VIA_INT_T2	32
#define VIA_INT_T1	64
#define VIA_INT_IRQ	128


#define VIACountA0         0x000000001ULL
#define VIACountA1         0x000000002ULL
#define VIACountA2         0x000000004ULL
#define VIACountA3         0x000000008ULL
#define VIACountB0         0x000000010ULL
#define VIACountB1         0x000000020ULL
#define VIACountB2         0x000000040ULL
#define VIACountB3         0x000000080ULL
#define VIALoadA0          0x000000100ULL
#define VIALoadA1          0x000000200ULL
#define VIALoadA2          0x000000400ULL
#define VIALoadB0          0x000000800ULL
#define VIALoadB1          0x000001000ULL
#define VIALoadB2          0x000002000ULL
#define VIACA2Low0         0x000004000ULL
#define VIACA2Low1         0x000008000ULL
#define VIACB2Low0         0x000010000ULL
#define VIACB2Low1         0x000020000ULL
#define VIAPB7Low0         0x000040000ULL
#define VIAPB7Low1         0x000080000ULL
#define VIAInterrupt0      0x000100000ULL
#define VIAInterrupt1      0x000200000ULL
#define VIAOneShotA0       0x000400000ULL
#define VIAOneShotB0       0x000800000ULL
#define VIAPostOneShotA0   0x001000000ULL
#define VIAPostOneShotB0   0x002000000ULL
#define VIACA1Trans0  (1ULL << 26)
#define VIACA1Trans1  (1ULL << 27)
#define VIACA2Trans0  (1ULL << 28)
#define VIACA2Trans1  (1ULL << 29)
#define VIACB1Trans0  (1ULL << 30)
#define VIACB1Trans1  (1ULL << 31)
#define VIACB2Trans0  (1ULL << 32)
#define VIACB2Trans1  (1ULL << 33)
#define VIAShift0     (1ULL << 34)
#define VIAShift1     (1ULL << 35)
#define VIAShift2     (1ULL << 36)
#define VIAShift3     (1ULL << 37)
#define VIAShift4     (1ULL << 38)
#define VIAMaskMSB    (1ULL << 39)

#define VIADelayMask ~(VIAMaskMSB | VIACountA0 | VIACountB0 | VIALoadA0 | VIALoadB0 | VIACA2Low0 | VIACB2Low0 | \
	VIAPB7Low0 | VIAInterrupt0 | VIAOneShotA0 | VIAOneShotB0 | VIAPostOneShotA0 | VIAPostOneShotB0 | \
	VIACA1Trans0 | VIACA2Trans0 | VIACB1Trans0 | VIACB2Trans0 | VIAShift0)

#define VIATrans1 (VIACA1Trans1 | VIACA2Trans1 | VIACB1Trans1 | VIACB2Trans1)

#define VIA_NO_CHANGE_TO_IDLE 4
#define VIA_MINIMUM_STAY_IDLE 3
#define VIA_MINIMUM_GO_IDLE_TIME 5

class VIA : public IRegister
{
public:
	virtual void SetCA2Output(bool)=0;
	virtual void SetCB1Output(bool)=0;
	virtual void SetCB2Output(bool)=0;
	virtual bit8 ReadPinsPortA()=0;
	virtual bit8 ReadPinsPortB()=0;
	virtual void SetPinsPortA(bit8 newPin)=0;
	virtual void SetPinsPortB(bit8 newPin)=0;
	virtual void SetSystemInterrupt()=0;
	virtual void ClearSystemInterrupt()=0;
	void InitReset(ICLK sysclock, bool poweronreset);

	//IRegister
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock) override;
	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	virtual void ExecuteDevices(ICLK sysclock) = 0;

	void GetState(SsViaCommon &state);
	void SetState(const SsViaCommon &state);
	bit8 PortAOutput();
	bit8 PortBOutput();
	void SetCA1Input(bool, int phase);
	void SetCA2Input(bool, int phase);
	void SetCB1Input(bool, int phase);
	void SetCB2Input(bool, int phase);
	void WakeUp();

protected:
	virtual void OnTransitionCA1Low();
	void LocalCA2Output(bool);
	void LocalCB1Output(bool);
	void LocalCB2Output(bool);

public:
	int ID = 0;
	ICLK DevicesClock = 0;
	bool bLatchA = false;
	bool bLatchB = false;
	bit8 ora = 0;
	bit8 ira = 0;
	bit8 orb = 0;
	bit8 irb = 0;
	bit8 ddra = 0;
	bit8 ddrb = 0;
	bit16u timer1_counter = {};
	bit16u timer2_counter = {};
	bit16u timer1_latch = {};
	bit16u timer2_latch = {};
	bit8 acr = 0;
	bit8 pcr = 0;
	bool ca1_in = false;
	bool ca1_in_prev = false;
	bool ca2_in = false;
	bool ca2_in_prev = false;
	bool cb1_in = false;
	bool cb1_in_prev = false;
	bool cb2_in = false;
	bool cb2_in_prev = false;
	bool ca2_out = false;
	bool cb1_out = false;
	bool cb2_out = false;
	bit8 ifr = 0;
	bit8 ier = 0;
	bit8 shiftRegisterMode = 0;
	bool shiftClockLevel = false;
	bit8 shiftCounter = 0;
	bit8 shiftRegisterData = 0;
	unsigned __int64 delay = 0;
	unsigned __int64 feed = 0;
	unsigned __int64 old_delay = 0;
	unsigned __int64 old_feed = 0;
	bit8 Interrupt = 0;
	bit8 bPB7TimerMode = 0;
	bit8 bPB7Toggle = 0;
	bit8 bPB7TimerOut = 0;
	bit8 no_change_count = 0;
	bit16 dec_2 = 0;
	bit8 idle = 0;

private:
	void ActiveTransitionCA1();
	void ActiveTransitionCA2();
	void ActiveTransitionCB1();
	void ActiveTransitionCB2();	
	void TransitionCA1();
	void TransitionCA2();
	void TransitionCB1();
	void TransitionCB2();
};
