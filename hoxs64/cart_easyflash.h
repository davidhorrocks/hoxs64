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
	virtual ~EasyFlashChip();
	EasyFlashChip(const EasyFlashChip&) = delete;
	EasyFlashChip& operator=(const EasyFlashChip&) = delete;
	EasyFlashChip(EasyFlashChip&&) = delete;
	EasyFlashChip& operator=(EasyFlashChip&&) = delete;

	static const int MAXEASYFLASHBANKS = 64;
	HRESULT Init(CartEasyFlash *pCartEasyFlash, int chipNumber);
	void CleanUp() noexcept;
	void Detach() noexcept;
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
		bit8 m_mode;
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
	EEasyFlashMode m_mode;
	vector<bit8> m_vecPendingSectorErase;
	CrtBankList *m_plstBank;
	int m_chipNumber;
	bit8 *m_pBlankData;

	friend CartEasyFlash;
};


class CartEasyFlash : public CartCommon
{
public:
	CartEasyFlash(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);
	virtual ~CartEasyFlash();

	CartEasyFlash() = delete;
	CartEasyFlash(const CartEasyFlash&) = delete;
	CartEasyFlash& operator=(const CartEasyFlash&) = delete;
	CartEasyFlash(CartEasyFlash&&) = delete;
	CartEasyFlash& operator=(CartEasyFlash&&) = delete;

	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

	bit8 ReadROML(bit16 address) override;
	bit8 ReadROMH(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;
	bit8 ReadUltimaxROMH(bit16 address) override;

	void WriteROML(bit16 address, bit8 data) override;
	void WriteROMH(bit16 address, bit8 data) override;
	void WriteUltimaxROML(bit16 address, bit8 data) override;
	void WriteUltimaxROMH(bit16 address, bit8 data) override;

	bit8 MonReadROML(bit16 address) override;
	bit8 MonReadROMH(bit16 address) override;
	bit8 MonReadUltimaxROML(bit16 address) override;
	bit8 MonReadUltimaxROMH(bit16 address) override;

	void MonWriteROML(bit16 address, bit8 data) override;
	void MonWriteROMH(bit16 address, bit8 data) override;
	void MonWriteUltimaxROML(bit16 address, bit8 data) override;
	void MonWriteUltimaxROMH(bit16 address, bit8 data) override;

	void PreventClockOverflow() override;

	ICLK GetCurrentClock() override;
	void SetCurrentClock(ICLK sysclock) override;

	HRESULT InitCart(unsigned int amountOfExtraRAM, bit8* pCartData, CrtBankList* plstBank, bit8* pZeroBankData) override;

	unsigned int GetStateBytes(int version, void* pstate) override;
	HRESULT SetStateBytes(int version, void* pstate, unsigned int size) override;

protected:
	void UpdateIO() override;
private:

	EasyFlashChip m_EasyFlashChipROML;
	EasyFlashChip m_EasyFlashChipROMH;

	friend EasyFlashChip;
};

