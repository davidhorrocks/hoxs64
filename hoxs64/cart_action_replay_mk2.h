#pragma once

class Cart;

class CartActionReplayMk2 : public CartActionReplay
{
public:
	CartActionReplayMk2(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock)override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data)override;

	void CartFreeze() override;
	void CheckForCartFreeze() override;

	HRESULT LoadState(IStream* pfs, int version) override;
	HRESULT SaveState(IStream* pfs) override;
protected:
	virtual void UpdateIO() override;
private:
	bool m_bActionReplayMk2Rom;
	int m_iActionReplayMk2EnableRomCounter;
	int m_iActionReplayMk2DisableRomCounter;
	ICLK m_clockLastDE00Write;
	ICLK m_clockLastDF40Read;
};
