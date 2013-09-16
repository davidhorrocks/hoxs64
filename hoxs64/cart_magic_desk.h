#pragma once

class Cart;

class CartMagicDesk : public CartCommon
{
public:
	CartMagicDesk(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

protected:
	virtual void UpdateIO();
private:
};
