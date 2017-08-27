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

class CIA1 : public CIA , public ErrorMsg
{
public:
	CIA1();
	HRESULT Init(CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, VIC6569 *vic, Tape64 *tape64, CDX9 *dx);
	void InitReset(ICLK sysclock, bool poweronreset);
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadPortA();
	virtual bit8 ReadPortB();
	bit8 ReadPortB_NoUpdateKeyboard();
	virtual void WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new);
	virtual void WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new);
	virtual void SetSystemInterrupt();
	virtual void ClearSystemInterrupt();
	virtual void ExecuteDevices(ICLK sysclock);

	void LightPen();
	bit8 keyboard_matrix[8];
	bit8 keyboard_rmatrix[8];
	bit8 joyport1,joyport2;

	void ReadKeyboard();
	void ResetKeyboard();
	void SetKeyMatrixDown(bit8 row, bit8 col);
	unsigned int NextScanDelta();
	void GetState(SsCia1V2 &state);
	void SetState(const SsCia1V2 &state);
	static void UpgradeStateV0ToV1(const SsCia1V0 &in, SsCia1V1 &out);
	static void UpgradeStateV1ToV2(const SsCia1V1 &in, SsCia1V2 &out);

	unsigned char c64KeyMap[256];
	ICLK nextKeyboardScanClock;
	virtual void SetWakeUpClock();

	CPU6510 *cpu;
	VIC6569 *vic;
	Tape64 *tape64;
	IC64 *pIC64;
	std::uniform_int_distribution<int> dist_pal_frame;
protected:
	bool m_bAltLatch;
private:
	bool ReadJoyAxis(int joyindex, struct joyconfig& joycfg, unsigned int& axis, bool& fire);
	static const unsigned int JOYDIR_UP = 8;
	static const unsigned int JOYDIR_DOWN = 4;
	static const unsigned int JOYDIR_LEFT = 2;
	static const unsigned int JOYDIR_RIGHT = 1;
	CAppStatus *appStatus;
	CDX9 *dx;
	bool restore_was_up;
	bool F12_was_up;
	bool F11_was_up;
};

#endif
