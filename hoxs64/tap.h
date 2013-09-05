#ifndef __TAP_H__
#define __TAP_H__

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
	HRESULT LoadTAPFile(TCHAR *s);
	void UnloadTAP();

	RAWTAPE TapeHeader;
	bit32 *pData;
	DWORD tape_max_counter;
	static const int DEFAULTDELAY = 32768;
	static const int MAX_COUNTERS = 0x280000;

private:
	HRESULT ReadTapeData(HANDLE hfile,int version, bit32 *buffer, int bufferlen, int *count);
};


class Tape64 : public ITape, public TAP64
{
public:
	Tape64();
	bit32 tape_position;
	ICLK tape_pulse_length;
	ICLK CurrentClock;
	ICLK nextTapeTickClock;

	HRESULT InsertTAPFile(TCHAR *filename);

	virtual void SetMotorWrite(bool motor, bit8 write);
	virtual void PressPlay();
	virtual void PressStop();
	virtual void Rewind();
	virtual void Eject();
	void Tick(ICLK sysclock);

	void GetState(SsTape &state);
	void SetState(const SsTape &state);
	ITapeEvent *TapeEvent;
private:
	bool bMotorOn;
	bool bPlayDown;
	bool bEOT;
	bool bEOD;
};

#endif
