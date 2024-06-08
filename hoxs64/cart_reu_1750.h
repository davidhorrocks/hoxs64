#pragma once

class Cart;

class CartReu1750 : public CartCommon
{
public:
	CartReu1750(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory);

	static const unsigned int MaxExtraBits = 5;
	static const unsigned int MaxRAMSize = 16 * 1024 * 1024;
	static const unsigned int DefaultRAMSize = 512 * 1024;
	bit8 ReadRegister(bit16 address, ICLK sysclock) override;
	void WriteRegister(bit16 address, ICLK sysclock, bit8 data) override;
	void Reset(ICLK sysclock, bool poweronreset) override;
	void ExecuteCycle(ICLK sysclock) override;
	bool IsREU() override;
	bool SnoopWrite(bit16 address, bit8 data) override;
	bit8 ReadByteFromREU(bit8 bank, bit16 address);
	void WriteByteToREU(bit8 bank, bit16 address, bit8 data);
	bit8 ReadByteFromC64(bit16 address, bool startingVicDMA);
	void WriteByteToC64(bit16 address, bit8 data);
	HRESULT InitCart(CartData& cartData) override;
protected:
	void UpdateIO() override;
	bool IsCartIOActive(bit16 address, bool isWriting) override;	
private:
	void StartTransfer();
	void RunC64ToREU();
	void RunREUToC64();
	void RunSwap();
	void RunVerify();
	void UpdateTransferAddressAndCounter();
	bool FinishTransfer();

	bit8& reg_status;             //00
	bit8& reg_command;            //01
	bit16u& reg_c64BaseAddress;  //02 - 03 
	bit16u& reg_reuBaseAddress;  //04 - 05
	bit8& reg_reuBankPointer;     //06
	bit16u& reg_transferLength;  //07 - 08
	bit8& reg_interruptMask;      //09
	bit8& reg_addressControl;     //0A

	bit16u& shadow_c64BaseAddress;
	bit16u& shadow_reuBaseAddress;
	bit8& shadow_reuBankPointer;
	bit16u& shadow_transferLength;

	bool& transfer_started;
	bool& swap_state;
	bool& verify_error;
	bool& transfer_finished;
	bool& dma_on;
	bool& swap_started;
	bit8& swap_byte_from_reu;
	bit8& swap_byte_from_c64;
	bit8& verify_byte_from_reu;
	bit8& verify_byte_from_c64;
	bit32& runcount;
	unsigned int reu_extraAddressBits;
	bit8 reu_extraAddressMask;
};
