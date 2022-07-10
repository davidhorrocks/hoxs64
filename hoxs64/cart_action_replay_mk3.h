#pragma once

class Cart;

class CartActionReplayMk3 : public CartActionReplay
{
public:
	CartActionReplayMk3(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

	void CartFreeze() override;
	void CheckForCartFreeze() override;

protected:
	virtual void UpdateIO();
private:
};
