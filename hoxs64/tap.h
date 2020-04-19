#pragma once
#include "errormsg.h"

# pragma pack (1)
typedef struct tagRAWTAPE
{
	bit8 Signature[12];
	bit8 Version;
	bit8 For_future_use[3];
	bit32 data_length;
	bit8 data[1];
} RAWTAPE,*pRAWTAPE;
# pragma pack ()

class TAP64 : public ErrorMsg
{
public:
	TAP64();
	~TAP64();
	HRESULT LoadTAPFile(const TCHAR *s);
	void UnloadTAP();

	RAWTAPE TapeHeader;
	bit32 *pData;
	DWORD tape_max_counter;
	static const int DEFAULTDELAY = 32768;
	static const int MAX_COUNTERS = 0x280000;

private:
	HRESULT ReadTapeData(IStream *pstmtap,int version, bit32 *buffer, int bufferMaxPulses, int *pCountOfPulses);
};


class Tape64 : public ITape, public TAP64
{
public:
	Tape64();
	~Tape64() = default;
	Tape64(const Tape64&) = delete;
	Tape64& operator=(const Tape64&) = delete;
	Tape64(Tape64&&) = delete;
	Tape64& operator=(Tape64&&) = delete;

	bit32 tape_position = 0;
	ICLK tape_pulse_length = 0;
	ICLK CurrentClock = 0;
	ICLK nextTapeTickClock = 0;

	HRESULT InsertTAPFile(const TCHAR *filename);

	virtual void SetMotorWrite(bool motor, bit8 write);
	virtual void PressPlay();
	virtual void PressStop();
	virtual void Rewind();
	virtual void Eject();
	void Tick(ICLK sysclock);

	void GetState(SsTape &state);
	void SetState(const SsTape &state);
	ITapeEvent *TapeEvent = nullptr;
private:
	bool bMotorOn = false;
	bool bPlayDown = false;
	bool bEOT = false;
	bool bEOD = false;
};
