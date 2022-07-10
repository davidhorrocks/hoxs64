#pragma once

class Cart;

class CartSimonsBasic : public CartCommon
{
public:
	CartSimonsBasic(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);
	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
protected:
	void UpdateIO() override;
private:
	bool& m_bSimonsBasic16K;
};
