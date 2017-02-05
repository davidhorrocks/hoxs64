#ifndef __VIA1_H__
#define __VIA1_H__

class CPUDisk;
class DiskInterface;

class VIA1 : public VIA
{
public:

	VIA1();

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
	void InitReset(ICLK sysclock, bool poweronreset);
	void Reset(ICLK sysclock, bool poweronreset);

	void GetState(SsVia1 &state);
	void SetState(const SsVia1 &state);

	CPUDisk *cpu;
	DiskInterface *disk;
	CAppStatus *appStatus;
};

#endif