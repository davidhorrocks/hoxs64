#pragma once

class Cart;

class CartOcean1 : public CartCommon
{
public:
	CartOcean1(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	bool IsCartIOActive(bit16 address, bool isWriting) override;
protected:
	virtual void UpdateIO() override;
private:
};
