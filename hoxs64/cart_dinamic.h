#pragma once

class Cart;

class CartDinamic : public CartCommon
{
public:
	CartDinamic(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

protected:
	void UpdateIO() override;
private:
};
