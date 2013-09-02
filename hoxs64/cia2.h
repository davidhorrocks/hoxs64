#ifndef __CIA2_H__
#define __CIA2_H__

class DiskInterface;
class VIC6569;

class CIA2 : public CIA , public ErrorMsg
{
public:

	CIA2();
	HRESULT Init(CConfig *cfg, CAppStatus *appStatus, CPU6510 *cpu, VIC6569 *vic, DiskInterface *disk);

	void InitReset(ICLK sysclock);
	//IRegister
	virtual void Reset(ICLK sysclock);

	bit8 ReadPortA();
	bit8 ReadPortB();
	void WritePortA();
	void WritePortB();
	void SetSystemInterrupt();
	void ClearSystemInterrupt();

	void ExecuteDevices(ICLK sysclock);

	void SetCurrentClock(ICLK sysclock);

	void GetState(SsCia2 &state);
	void SetState(const SsCia2 &state);

	CConfig *cfg;
	CAppStatus *appStatus;
	CPU6510 *cpu;
	VIC6569 *vic;
	DiskInterface *disk;
	volatile bit8 c64_serialbus;
	bit8 m_commandedVicBankIndex;

};


#endif
