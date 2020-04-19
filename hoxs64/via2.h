#pragma once

class VIA2 : public VIA
{
public:
	VIA2() = default;
	~VIA2() = default;
	VIA2(const VIA2&) = delete;
	VIA2& operator=(const VIA2&) = delete;
	VIA2(VIA2&&) = delete;
	VIA2& operator=(VIA2&&) = delete;

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
	virtual void Reset(ICLK sysclock, bool poweronreset);

	void GetState(SsVia2 &state);
	void SetState(const SsVia2 &state);

protected:
	virtual void OnTransitionCA1Low();

public:
	bit8 oldDiskControl = 0;
	DiskInterface *disk = nullptr;
	CPUDisk *cpu = nullptr;
	CAppStatus *appStatus = nullptr;
};

