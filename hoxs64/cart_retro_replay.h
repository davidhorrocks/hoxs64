#pragma once

class Cart;

class CartRetroReplay : public CartCommon
{
public:
	CartRetroReplay(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

	void CartFreeze() override;
	void CheckForCartFreeze() override;
	bool IsCartIOActive(bit16 address, bool isWriting) override;
	bit8 ReadROML(bit16 address) override;
	bit8 ReadROMH(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;
	bit8 ReadUltimaxROMH(bit16 address) override;
	void WriteROML(bit16 address, bit8 data) override;
	void WriteROMH(bit16 address, bit8 data) override;
	void WriteUltimaxROML(bit16 address, bit8 data) override;
	void WriteUltimaxROMH(bit16 address, bit8 data) override;
protected:
	void UpdateIO() override;
private:

	bool& m_bDE01WriteDone;
};
