#ifndef __CIA1_H__
#define __CIA1_H__

class IAutoLoad
{
public:
	virtual void AutoLoadHandler(ICLK sysclock)=0;
};

class VIC6569;
class C64;
class Tape64;
class ISid;

class CIA1 : public CIA, public ICia1, public ErrorMsg
{
public:
	CIA1();
	HRESULT Init(CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, VIC6569 *vic, ISid *sid, Tape64 *tape64, CDX9 *dx, IAutoLoad* pAutoLoad);
	void InitReset(ICLK sysclock, bool poweronreset);
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadPortA();
	virtual bit8 ReadPortB();
	bit8 ReadPortA_NoUpdateKeyboard();
	bit8 ReadPortB_NoUpdateKeyboard();
	virtual void WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new);
	virtual void WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new);
	virtual void SetSystemInterrupt();
	virtual void ClearSystemInterrupt();
	virtual void ExecuteDevices(ICLK sysclock);
	virtual bit8 Get_PotAX();
	virtual bit8 Get_PotAY();
	virtual bit8 Get_PotBX();
	virtual bit8 Get_PotBY();
	void LightPen();
	bit8 keyboard_matrix[8];
	bit8 keyboard_rmatrix[8];
	bit8 joyport1,joyport2;
	bit8 potAx;
	bit8 potAy;
	bit8 potBx;
	bit8 potBy;
	bit8 out4066PotX;
	bit8 out4066PotY;

	void WriteDebuggerReadKeyboard();
	void ReadKeyboard();
	void ResetKeyboard();
	void SetKeyMatrixDown(bit8 row, bit8 col);
	ICLK NextScanDelta();
	void GetState(SsCia1V2 &state);
	void SetState(const SsCia1V2 &state);
	static void UpgradeStateV0ToV1(const SsCia1V0 &in, SsCia1V1 &out);
	static void UpgradeStateV1ToV2(const SsCia1V1 &in, SsCia1V2 &out);

	unsigned char c64KeyMap[256];// array of windows keyboard scan codes indexed by the C64Keys::C64Key enumeration
	ICLK nextKeyboardScanClock;
	virtual void SetWakeUpClock();

	CPU6510 *cpu;
	VIC6569 *vic;
	ISid *sid;
	Tape64 *tape64;
	IC64 *pIC64;
	IAutoLoad* pIAutoLoad;
	uniform_int_distribution<int> dist_pal_frame;
protected:
	bool m_bAltLatch;
private:
	bool ReadJoyAxis(int joyindex, struct joyconfig& joycfg, unsigned int& axis, bool& fire1, bool& fire2, unsigned char c64keyboard[]);
	static const unsigned int JOYDIR_UP = 8;
	static const unsigned int JOYDIR_DOWN = 4;
	static const unsigned int JOYDIR_LEFT = 2;
	static const unsigned int JOYDIR_RIGHT = 1;
	CAppStatus *appStatus;
	CDX9 *dx;
	DIJOYSTATE2  js;
	bool restore_was_up;
	bool F12_was_up;
	bool F11_was_up;
	ICLK keyboardNotAcquiredClock;
};

#endif
