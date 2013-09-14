#pragma once

class Cart;

class CartOcean1 : public CartCommon
{
public:
	CartOcean1(CrtHeader &crtHeader, CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

protected:
	virtual void UpdateIO();
private:
};
