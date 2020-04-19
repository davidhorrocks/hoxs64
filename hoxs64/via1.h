#pragma once

class CPUDisk;
class DiskInterface;

class VIA1 : public VIA
{
public:

	VIA1() = default;
	~VIA1() = default;
	VIA1(const VIA1&) = delete;
	VIA1& operator=(const VIA1&) = delete;
	VIA1(VIA1&&) = delete;
	VIA1& operator=(VIA1&&) = delete;

	HRESULT Init(int ID, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk);
	virtual void ExecuteDevices(ICLK sysclock);
	void SetCA2Output(bool value);
	void SetCB1Output(bool value);
	void SetCB2Output(bool value);
	bit8 ReadPinsPortA();
	bit8 ReadPinsPortB();
	void SetPinsPortA(bit8);
	void SetPinsPortB(bit8);
	void SetSystemInterrupt();
	void ClearSystemInterrupt();
	void InitReset(ICLK sysclock, bool poweronreset);
	void Reset(ICLK sysclock, bool poweronreset);

	void GetState(SsVia1 &state);
	void SetState(const SsVia1 &state);

	CPUDisk *cpu = nullptr;
	DiskInterface *disk = nullptr;
	CAppStatus *appStatus = nullptr;
};
