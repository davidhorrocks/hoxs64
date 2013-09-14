#pragma once

class Cart;

class CartSimonsBasic : public CartCommon
{
public:
	CartSimonsBasic(CrtHeader &crtHeader, CrtBankList *plstBank, bit8 *pCartData, bit8 *pZeroBankData, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual void Reset(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

protected:
	virtual void UpdateIO();
private:
	bool m_bSimonsBasic16K;
};
