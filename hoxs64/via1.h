#ifndef __VIA1_H__
#define __VIA1_H__

class CPUDisk;
class DiskInterface;

class VIA1 : public VIA
{
public:

	VIA1();

	HRESULT Init(int ID, CConfig *cfg, CAppStatus *appStatus, CPUDisk *cpu, DiskInterface *disk);
	virtual void ExecuteDevices(ICLK sysclock);
	void SetCA2Output(bit8 value);
	void SetCB2Output(bit8 value);
	bit8 ReadPinsPortA();
	bit8 ReadPinsPortB();
	void SetPinsPortA(bit8);
	void SetPinsPortB(bit8);
	void SetSystemInterrupt();
	void ClearSystemInterrupt();
	void Reset(ICLK sysclock);

	CPUDisk *cpu;
	DiskInterface *disk;
	CConfig *cfg;
	CAppStatus *appStatus;
};

#endif