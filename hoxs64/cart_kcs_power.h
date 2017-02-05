#pragma once

class Cart;

class CartKcsPower : public CartCommon
{
public:
	CartKcsPower(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

	virtual void CartFreeze();
	virtual void CheckForCartFreeze();

protected:
	virtual void UpdateIO();
private:
	void LatchShift();
};
