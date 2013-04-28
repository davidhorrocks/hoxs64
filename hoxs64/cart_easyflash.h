#pragma once

class Cart;
class CartEasyFlash;

class EasyFlashChip
{
public:
	enum EEasyFlashMode
	{
		Read,
		AutoSelect,
		ByteProgram,	
		ChipErase,
		SectorErase,
		SectorEraseSuspend,
		ChipEraseSuspend,
	};
	EasyFlashChip();
	~EasyFlashChip();
	static const int MAXBANKS = 64;
	void Init(CartEasyFlash *pCartEasyFlash, int chipNumber);
	void CleanUp();
	void Detach();
	void Reset(ICLK sysclock);
	bit8 ReadByte(bit16 address);
	void WriteByte(bit16 address, bit8 data);
	void CheckForPendingWrite(ICLK clock);
	bit8 MonReadByte(bit16 address);
	void MonWriteByte(bit16 address, bit8 data);

	void PreventClockOverflow();

private:
	CartEasyFlash *m_pCartEasyFlash;
	ICLK m_iLastCommandWriteClock;
	bit8 m_iCommandByte;
	bit8 m_iCommandCycle;
	bit8 m_iStatus;
	bit8 m_iByteWritten;
	bit16 m_iAddressWrite;
	bit8 m_iSectorWrite;
	EEasyFlashMode m_mode;
	std::vector<bit8> m_vecPendingSectorErase;
	std::vector<CrtChipAndData> m_vecBanks;
	int m_chipNumber;
	bit8 *m_pBlankData;
};


class CartEasyFlash : public CartCommon
{
public:
	CartEasyFlash(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory);
	~CartEasyFlash();
	virtual void Reset(ICLK sysclock);
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

protected:
	virtual void UpdateIO();
private:

	EasyFlashChip m_EasyFlashChipROML;
	EasyFlashChip m_EasyFlashChipROMH;

	friend EasyFlashChip;
};

