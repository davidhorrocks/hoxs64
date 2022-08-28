#pragma once
#include "errormsg.h"

class IAutoLoad
{
public:
	virtual void AutoLoadHandler(ICLK sysclock)=0;
};


class Tape64;
class ISid;

class CIA1 : public CIA, public ICia1, public ErrorMsg
{
public:
	CIA1();
	~CIA1() = default;
	CIA1(const CIA1&) = delete;
	CIA1& operator=(const CIA1&) = delete;
	CIA1(CIA1&&) = delete;
	CIA1& operator=(CIA1&&) = delete;	
	HRESULT Init(CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, IVic *vic, ISid *sid, Tape64 *tape64, CDX9 *dx, IAutoLoad* pAutoLoad);
	void UpdateKeyMap();
	void InitReset(ICLK sysclock, bool poweronreset);
	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadPortA() override;
	bit8 ReadPortB() override;
	bit8 ReadPortA_NoUpdateKeyboard();
	bit8 ReadPortB_NoUpdateKeyboard();
	void WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new) override;
	void WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new) override;
	void SetSystemInterrupt() override;
	void ClearSystemInterrupt() override;
	void ExecuteDevices(ICLK sysclock) override;
	void EnableInput(bool enabled) override;
	bit8 Get_PotAX() override;
	bit8 Get_PotAY() override;
	bit8 Get_PotBX() override;
	bit8 Get_PotBY() override;
	void LightPen();
	void SetWakeUpClock() override;
	bit8 keyboard_matrix[8] = {};
	bit8 keyboard_rmatrix[8] = {};
	bit8 joyport1 = 0;
	bit8 joyport2 = 0;
	bit8 potAx = 0;
	bit8 potAy = 0;
	bit8 potBx = 0;
	bit8 potBy = 0;
	bit8 out4066PotX = 0;
	bit8 out4066PotY = 0;

	void WriteDebuggerReadKeyboard();
	void ReadKeyboard();
	void ResetKeyboard();
	void SetKeyMatrixDown(bit8 row, bit8 col);
	void SetKeyMatrixCodeDown(C64MatrixCodes::C64MatrixCode code);
	ICLK NextScanDelta();
	void GetState(SsCia1V2 &state);
	void SetState(const SsCia1V2 &state);
	static void UpgradeStateV0ToV1(const SsCia1V0 &in, SsCia1V1 &out);
	static void UpgradeStateV1ToV2(const SsCia1V1 &in, SsCia1V2 &out);
	static void UpgradeStateV2ToV3(const SsCia1V2& in, SsCia1V2& out);

	unsigned char c64KeyMap[256] = {};// array of windows keyboard scan codes indexed by the C64Keys::C64Key enumeration
	ICLK nextKeyboardScanClock = 0;

	CPU6510 *cpu = nullptr;
	IVic* vic = nullptr;
	ISid *sid = nullptr;
	Tape64 *tape64 = nullptr;
	IC64 *pIC64 = nullptr;
	IAutoLoad* pIAutoLoad = nullptr;
	uniform_int_distribution<int> dist_pal_frame;
protected:
	bool m_bAltLatch = 0;
private:
	bool ReadJoyAxis(int joyindex, struct joyconfig& joycfg, unsigned int& axis, bool& fire1, bool& fire2, unsigned char c64keyboard[]);

	static const unsigned int JOYDIR_UP = 8;
	static const unsigned int JOYDIR_DOWN = 4;
	static const unsigned int JOYDIR_LEFT = 2;
	static const unsigned int JOYDIR_RIGHT = 1;
	bool enableInput = true;

	CAppStatus *appStatus = nullptr;
	CDX9 *dx = nullptr;
	DIJOYSTATE2  js = {};
	bool restore_was_up = false;
	bool F12_was_up = false;
	bool F11_was_up = false;
	ICLK keyboardNotAcquiredClock = 0;
};
