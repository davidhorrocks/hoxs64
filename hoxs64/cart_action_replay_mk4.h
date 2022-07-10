#pragma once

class Cart;

class CartActionReplayMk4 : public CartActionReplay
{
public:
	CartActionReplayMk4(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;

	void CartFreeze() override;
	void CheckForCartFreeze() override;

protected:
	void UpdateIO() override;
private:
};
