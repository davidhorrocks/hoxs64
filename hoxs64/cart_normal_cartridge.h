#pragma once

class Cart;

class CartNormalCartridge : public CartCommon
{
public:
	CartNormalCartridge(Cart *pCart, IC6510 *pCpu, bit8 *pC64RamMemory);

	//virtual void ExecuteCycle(ICLK sysclock);
	//virtual void Reset(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	//virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	//virtual bit8 ReadROML(bit16 address);
	//virtual bit8 ReadROMH(bit16 address);
	//virtual bit8 ReadUltimaxROML(bit16 address);
	//virtual bit8 ReadUltimaxROMH(bit16 address);

	//virtual void WriteROML(bit16 address, bit8 data);
	//virtual void WriteROMH(bit16 address, bit8 data);
	//virtual void WriteUltimaxROML(bit16 address, bit8 data);
	//virtual void WriteUltimaxROMH(bit16 address, bit8 data);

	//virtual bit8 MonReadROML(bit16 address);
	//virtual bit8 MonReadROMH(bit16 address);
	//virtual bit8 MonReadUltimaxROML(bit16 address);
	//virtual bit8 MonReadUltimaxROMH(bit16 address);

	//virtual void MonWriteROML(bit16 address, bit8 data);
	//virtual void MonWriteROMH(bit16 address, bit8 data);
	//virtual void MonWriteUltimaxROML(bit16 address, bit8 data);
	//virtual void MonWriteUltimaxROMH(bit16 address, bit8 data);

	//virtual void CartFreeze();
	//virtual void CartReset();
	//virtual void CheckForCartFreeze();

	//virtual void DetachCart();
protected:
	virtual void UpdateIO();
private:
};
