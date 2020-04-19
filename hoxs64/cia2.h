#pragma once
#include "errormsg.h"

class DiskInterface;
class VIC6569;

class CIA2 : public CIA , public ErrorMsg
{
public:
	CIA2();
	~CIA2() = default;
	CIA2(const CIA2&) = delete;
	CIA2& operator=(const CIA2&) = delete;
	CIA2(CIA2&&) = delete;
	CIA2& operator=(CIA2&&) = delete;
	HRESULT Init(CAppStatus *appStatus, CPU6510 *cpu, VIC6569 *vic, DiskInterface *disk);

	void InitReset(ICLK sysclock, bool poweronreset);
	//IRegister
	void Reset(ICLK sysclock, bool poweronreset) override;

	bit8 ReadPortA() override;
	bit8 ReadPortB() override;
	void WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new) override;
	void WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new) override;
	void SetSystemInterrupt() override;
	void ClearSystemInterrupt() override;
	void ExecuteDevices(ICLK sysclock) override;
	void SetCurrentClock(ICLK sysclock) override;

	void GetState(SsCia2V2 &state);
	void SetState(const SsCia2V2 &state);
	static void UpgradeStateV0ToV1(const SsCia2V0 &in, SsCia2V1 &out);
	static void UpgradeStateV1ToV2(const SsCia2V1 &in, SsCia2V2 &out);

	CAppStatus* appStatus = nullptr;
	CPU6510 *cpu = nullptr;
	VIC6569 *vic = nullptr;
	DiskInterface *disk = nullptr;
	volatile bit8 c64_serialbus = 0;
	bit8 m_commandedVicBankIndex = 0;
private:
	uniform_int_distribution<int> dist_port_temperature;
};
