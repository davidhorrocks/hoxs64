#pragma once

class Cart;

class CartActionReplayMk2 : public CartCommon
{
public:
	 CartActionReplayMk2(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);

	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);

	virtual void CartFreeze();
	virtual void CheckForCartFreeze();

protected:
	virtual void UpdateIO();
private:
	bool m_bActionReplayMk2Rom;
	int m_iActionReplayMk2EnableRomCounter;
	int m_iActionReplayMk2DisableRomCounter;
	ICLK m_clockLastDE00Write;
	ICLK m_clockLastDF40Read;
};
