#ifndef __VIA2_H__
#define __VIA2_H__


class VIA2 : public VIA
{
public:
	VIA2();

	HRESULT Init(CConfig *cfg, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk);
	virtual void ExecuteDevices(ICLK sysclock);
	void SetCA2Output(bit8 value);
	void SetCB2Output(bit8 value);
	bit8 ReadPinsPortA();
	bit8 ReadPinsPortB();
	void SetPinsPortA(bit8);
	void SetPinsPortB(bit8);
	void SetSystemInterrupt();
	void ClearSystemInterrupt();

	virtual void Reset(ICLK sysclock);
	bit8 oldDiskControl;

	DiskInterface *disk;
	CPUDisk *cpu;
	CConfig *cfg;
	CAppStatus *appStatus;
};


#endif