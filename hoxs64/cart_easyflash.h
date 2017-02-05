#pragma once

class Cart;
class CartEasyFlash;

class EasyFlashChip
{
public:
	enum EEasyFlashMode
	{
		Read = 0,
		AutoSelect = 1,
		ByteProgram = 2,	
		ChipErase = 3,
		SectorErase = 4,
		SectorEraseSuspend = 5,
		ChipEraseSuspend = 6,
	};
	EasyFlashChip();
	~EasyFlashChip();
	static const int MAXEASYFLASHBANKS = 64;
	HRESULT Init(CartEasyFlash *pCartEasyFlash, int chipNumber);
	void CleanUp();
	void Detach();
	void Reset(ICLK sysclock);
	bit8 ReadByte(bit16 address);
	void WriteByte(bit16 address, bit8 data);
	void CheckForPendingWrite(ICLK clock);
	bit8 MonReadByte(bit16 address);
	void MonWriteByte(bit16 address, bit8 data);

	void PreventClockOverflow();
	void SetCurrentClock(ICLK sysclock);

protected:
	struct SsState
	{
		bit32 size;
		bit32 m_iLastCommandWriteClock;
		bit8 m_iCommandByte;
		bit8 m_iCommandCycle;
		bit8 m_iStatus;
		bit8 m_iByteWritten;
		//bit16 m_iAddressWrite;
		//bit8 m_iSectorWrite;
		bit8 m_mode;
		//std::vector<bit8> m_vecPendingSectorErase;
		//CrtBankList *m_plstBank;
		//int m_chipNumber;
		//bit8 *m_pBlankData;
	};
	virtual unsigned int GetStateBytes(void *pstate);
	virtual HRESULT SetStateBytes(void *pstate, unsigned int size);

private:
	CartEasyFlash *m_pCartEasyFlash;
	ICLK m_iLastCommandWriteClock;
	bit8 m_iCommandByte;
	bit8 m_iCommandCycle;
	bit8 m_iStatus;
	bit8 m_iByteWritten;
	//bit16 m_iAddressWrite;
	//bit8 m_iSectorWrite;
	EEasyFlashMode m_mode;
	std::vector<bit8> m_vecPendingSectorErase;
	CrtBankList *m_plstBank;
	int m_chipNumber;
	bit8 *m_pBlankData;

	friend CartEasyFlash;
};


class CartEasyFlash : public CartCommon
{
public:
	CartEasyFlash(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);
	~CartEasyFlash();
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

	virtual bit8 ReadROML(bit16 address);
	virtual bit8 ReadROMH(bit16 address);
	virtual bit8 ReadUltimaxROML(bit16 address);
	virtual bit8 ReadUltimaxROMH(bit16 address);

	virtual void WriteROML(bit16 address, bit8 data);
	virtual void WriteROMH(bit16 address, bit8 data);
	virtual void WriteUltimaxROML(bit16 address, bit8 data);
	virtual void WriteUltimaxROMH(bit16 address, bit8 data);

	virtual bit8 MonReadROML(bit16 address);
	virtual bit8 MonReadROMH(bit16 address);
	virtual bit8 MonReadUltimaxROML(bit16 address);
	virtual bit8 MonReadUltimaxROMH(bit16 address);

	virtual void MonWriteROML(bit16 address, bit8 data);
	virtual void MonWriteROMH(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROML(bit16 address, bit8 data);
	virtual void MonWriteUltimaxROMH(bit16 address, bit8 data);

	virtual void PreventClockOverflow();

	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	virtual HRESULT InitCart(CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData);

	virtual unsigned int GetStateBytes(void *pstate);
	virtual HRESULT SetStateBytes(void *pstate, unsigned int size);

protected:
	virtual void UpdateIO();
private:

	EasyFlashChip m_EasyFlashChipROML;
	EasyFlashChip m_EasyFlashChipROMH;

	friend EasyFlashChip;
};

