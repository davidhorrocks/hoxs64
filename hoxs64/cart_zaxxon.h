#pragma once

class Cart;

class CartZaxxon : public CartCommon
{
public:
	CartZaxxon(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bit8 ReadROML(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;

protected:
	virtual void UpdateIO();
private:
};
