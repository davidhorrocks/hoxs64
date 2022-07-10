#pragma once

class Cart;

class CartEpyxFastLoad : public CartCommon
{
public:
	CartEpyxFastLoad(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);
	void ExecuteCycle(ICLK sysclock) override;
	void Reset(ICLK sysclock, bool poweronreset) override;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	bit8 ReadROML(bit16 address) override;
	bit8 ReadUltimaxROML(bit16 address) override;
	bit8 Get_EXROM() override;
protected:
	void UpdateIO() override;
private:

	int m_iCapacitorCharge;
	bool m_bCapacitorCharged;

	void DischargeCapacitor();
};
