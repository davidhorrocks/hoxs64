#pragma once

class Cart;

class CartNormalCartridge : public CartCommon
{
public:
	CartNormalCartridge(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
protected:
	void UpdateIO() override;
private:
};
