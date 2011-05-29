#ifndef __VIA6522_H__
#define __VIA6522_H__

#define VIA_SER_IDLE		0x1
#define VIA_SER_DISABLED	0
#define VIA_SER_IN_T2		0x4
#define VIA_SER_IN_02		0x8
#define VIA_SER_IN_CB1		0xc0
#define VIA_SER_OUT_T2_CIRC	0x10
#define VIA_SER_OUT_T2		0x14
#define VIA_SER_OUT_02		0x18
#define VIA_SER_OUT_CB1		0x1c

#define VIA_INT_CA2 1
#define VIA_INT_CA1	2
#define VIA_INT_SER	4
#define VIA_INT_CB2	8
#define VIA_INT_CB1	16
#define VIA_INT_T2	32
#define VIA_INT_T1	64
#define VIA_INT_IRQ	128


#define VIACountA0         0x00000001
#define VIACountA1         0x00000002
#define VIACountA2         0x00000004
#define VIACountA3         0x00000008
#define VIACountB0         0x00000010
#define VIACountB1         0x00000020
#define VIACountB2         0x00000040
#define VIACountB3         0x00000080
#define VIALoadA0          0x00000100
#define VIALoadA1          0x00000200
#define VIALoadA2          0x00000400
#define VIALoadB0          0x00000800
#define VIALoadB1          0x00001000
#define VIALoadB2          0x00002000
#define VIACA2Low0         0x00004000
#define VIACA2Low1         0x00008000
#define VIACB2Low0         0x00010000
#define VIACB2Low1         0x00020000
#define VIAPB7Low0         0x00040000
#define VIAPB7Low1         0x00080000
#define VIAInterrupt0      0x00100000
#define VIAInterrupt1      0x00200000
#define VIAOneShotA0       0x00400000
#define VIAOneShotB0       0x00800000
#define VIAPostOneShotA0   0x01000000
#define VIAPostOneShotB0   0x02000000
#define VIADelayMask     ~(0x04000000 | VIACountA0 | VIACountB0 | VIALoadA0 | VIALoadB0 | VIACA2Low0 | VIACB2Low0 | VIAPB7Low0 | VIAInterrupt0 | VIAOneShotA0 | VIAOneShotB0 | VIAPostOneShotA0 | VIAPostOneShotB0)

#define VIA_NO_CHANGE_TO_IDLE 4
#define VIA_MINIMUM_STAY_IDLE 3
#define VIA_MINIMUM_GO_IDLE_TIME 5

class VIA : public IRegister
{
public:
	virtual void SetCA2Output(bit8)=0;
	virtual void SetCB2Output(bit8)=0;
	virtual bit8 ReadPinsPortA()=0;
	virtual bit8 ReadPinsPortB()=0;
	virtual void SetPinsPortA(bit8 newPin)=0;
	virtual void SetPinsPortB(bit8 newPin)=0;
	virtual void SetSystemInterrupt()=0;
	virtual void ClearSystemInterrupt()=0;

	//IRegister
	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	ICLK DevicesClock;
	virtual void ExecuteDevices(ICLK sysclock)=0;

	bit8 PortAOutput();
	bit8 PortBOutput();
	void SetCA1Input(bit8);
	void SetCA2Input(bit8);
	void SetCB1Input(bit8);
	void SetCB2Input(bit8);
	void WakeUp();
	void SetPortA();
	void SetPortB();


	void ActiveTransitionCA1();
	void ActiveTransitionCA2();
	void ActiveTransitionCB1();
	void ActiveTransitionCB2();
	
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
	bit8 ca2_in;
	bit8 cb1_in;
	bit8 cb2_in;
	bit8 ca2_out;
	bit8 cb2_out;
	bit8 shift;
	bit8 ifr;
	bit8 ier;
	//bit8 sequence_t1;
	//bit8 sequence_t2;
	bit8 serial_active;
	bit8 serial_mode;
	bit32 delay;
	bit32 feed;
	bit32 old_delay;
	bit32 old_feed;
	bit8 modulo;
	bit8 Interrupt;

	bit8 bPB7TimerMode;
	bit8 bPB7Toggle;
	bit8 bPB7TimerOut;

	bit8 no_change_count;
	bit16 dec_2;
	bit8 idle;
};


#endif

