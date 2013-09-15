#pragma once

class Cart;

class CartZaxxon : public CartCommon
{
public:
	CartZaxxon(IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadROML(bit16 address);
	virtual bit8 ReadUltimaxROML(bit16 address);

protected:
	virtual void UpdateIO();
private:
};
