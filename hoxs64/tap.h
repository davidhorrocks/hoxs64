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

	RAWTAPE *pTapeHeader;
	bit8 *pTapeData;
	TCHAR errorText[300];
	DWORD tape_length;
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
	ITapeEvent *TapeEvent;
private:
	bool bMotorOn;
	bool bPlayDown;
	bool bEOT;
	bool bEOD;
};

#endif
