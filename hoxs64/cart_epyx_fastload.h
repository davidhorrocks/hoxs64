#pragma once

class Cart;

class CartEpyxFastLoad : public CartCommon
{
public:
	CartEpyxFastLoad(const CrtHeader &crtHeader, IC6510 *pCpu, bit8 *pC64RamMemory);
	void ExecuteCycle(ICLK sysclock);
	virtual void Reset(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);

	virtual bit8 ReadROML(bit16 address);
	virtual bit8 ReadUltimaxROML(bit16 address);

	virtual bit8 Get_EXROM();

protected:
	virtual void UpdateIO();
private:

	int m_iCapacitorCharge;
	bool m_bCapacitorCharged;

	void DischargeCapacitor();
};
