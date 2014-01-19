#ifndef __VIA2_H__
#define __VIA2_H__


class VIA2 : public VIA
{
public:
	VIA2();

	HRESULT Init(int ID, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk);
	virtual void ExecuteDevices(ICLK sysclock);
	void SetCA2Output(bit8 value);
	void SetCB2Output(bit8 value);
	bit8 ReadPinsPortA();
	bit8 ReadPinsPortB();
	void SetPinsPortA(bit8);
	void SetPinsPortB(bit8);
	void SetSystemInterrupt();
	void ClearSystemInterrupt();

	void InitReset(ICLK sysclock);
	virtual void Reset(ICLK sysclock);

	void GetState(SsVia2 &state);
	void SetState(const SsVia2 &state);

protected:
	virtual void OnTransitionCA1Low();

public:
	bit8 oldDiskControl;
	DiskInterface *disk;
	CPUDisk *cpu;
	CAppStatus *appStatus;
};


#endif