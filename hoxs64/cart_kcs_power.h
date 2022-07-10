#pragma once

class Cart;

class CartKcsPower : public CartCommon
{
public:
	CartKcsPower(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

	void CartFreeze() override;
	void CheckForCartFreeze() override;

protected:
	virtual void UpdateIO() override;
private:
	void LatchShift();
};
