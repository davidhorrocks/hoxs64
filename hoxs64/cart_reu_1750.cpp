#include "cart.h"

CartReu1750::CartReu1750(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
	, reg_status(registerArray[0x0])
	, reg_command(registerArray[0x1])
	, reg_c64BaseAddress(*(bit16u*)&registerArray[0x2])
	, reg_reuBaseAddress(*(bit16u*)&registerArray[0x4])
	, reg_reuBankPointer(registerArray[0x6])
	, reg_transferLength(*(bit16u*)&registerArray[0x7])
	, reg_interruptMask(registerArray[0x9])
	, reg_addressControl(registerArray[0xA])
	, shadow_c64BaseAddress(*(bit16u*)&registerArray[0x10])
	, shadow_reuBaseAddress(*(bit16u*)&registerArray[0x12])
	, shadow_reuBankPointer(registerArray[0x14])
	, shadow_transferLength(*(bit16u*)&registerArray[0x15])
	, transfer_started(m_statebool[0])
	, transfer_finished(m_statebool[1])
	, swap_state(m_statebool[2])
	, verify_error(m_statebool[3])
	, dma_on(m_statebool[4])
	, swap_started(m_statebool[5])
	, swap_byte_from_reu(m_statebyte[0])
	, swap_byte_from_c64(m_statebyte[1])
	, verify_byte_from_reu(m_statebyte[2])
	, verify_byte_from_c64(m_statebyte[3])
	, runcount(m_stateuint[0])
{
	dma_on = false;
	GAME = 1;
	EXROM = 1;
	DMA = 1;
	m_bREUcompatible = true;
	reu_extraAddressBits = 0;
	reu_extraAddressMask = 7;
}

HRESULT CartReu1750::InitCart(CartData& cartData)
{
	CartCommon::InitCart(cartData);
	unsigned int extraBitsCompare = MaxExtraBits;
	unsigned int extraMaskCompare = 0xff;
	unsigned int memorySizeCompare = MaxRAMSize;
	while (extraBitsCompare > 0 && (memorySizeCompare > m_amountOfExtraRAM || (extraBitsCompare > cartData.m_crtHeader.SubType && m_amountOfExtraRAM < MaxRAMSize)))
	{
		extraBitsCompare--;
		memorySizeCompare = memorySizeCompare >>= 1;
		extraMaskCompare = extraMaskCompare >>= 1;
	}

	this->reu_extraAddressBits = extraBitsCompare;
	this->reu_extraAddressMask = extraMaskCompare;
	return S_OK;
}


void CartReu1750::Reset(ICLK sysclock, bool poweronreset)
{
	CartCommon::Reset(sysclock, poweronreset);
	reg_status = 0x10;
	reg_command = 0x10;
	reg_c64BaseAddress.word = 0;
	reg_reuBaseAddress.word = 0;
	reg_reuBankPointer = ~reu_extraAddressMask;
	reg_transferLength.word = 0xFFFF;
	reg_interruptMask = 0x1F;
	reg_addressControl = 0x3F;
	shadow_c64BaseAddress.word = 0;
	shadow_reuBaseAddress.word = 0;
	shadow_reuBankPointer = ~reu_extraAddressMask;
	shadow_transferLength.word = 0xFFFF;
	transfer_started = false;
	transfer_finished = false;
	verify_error = false;
	dma_on = false;
	DMA = dma_on ? 0 : 1;
	if (poweronreset && !this->m_bPreserveRamOnReset)
	{
		unsigned int i;
		unsigned int* p = (unsigned int*)m_pCartData;
		for (i = 0; i < m_amountOfExtraRAM / sizeof(bit32); i++)
		{
			if (((i & 0x40) == 0) ^ ((i & 0x8000) == 0))
			{
				p[i] = 0x00FFFF00;
			}
			else
			{
				p[i] = 0xFF0000FF;
			}
		}
	}

	this->m_bPreserveRamOnReset = true;
}

bit8 CartReu1750::ReadRegister(bit16 address, ICLK sysclock)
{
	bit8 t;
	switch (address & 0x1F)
	{
	case 0x0:
		t = reg_status;
		if (m_bEffects)
		{
			reg_status = 0;
			m_pCpu->Clear_CRT_IRQ();
		}

		return t | 0x10;
	case 0x1:
		return reg_command;
		break;
	case 0x2:
		return reg_c64BaseAddress.byte.loByte;
		break;
	case 0x3:
		return reg_c64BaseAddress.byte.hiByte;
		break;
	case 0x4:
		return reg_reuBaseAddress.byte.loByte;
		break;
	case 0x5:
		return reg_reuBaseAddress.byte.hiByte;
		break;
	case 0x6:
		return reg_reuBankPointer | ~reu_extraAddressMask;
		break;
	case 0x7:
		return reg_transferLength.byte.loByte;
		break;
	case 0x8:
		return reg_transferLength.byte.hiByte;
		break;
	case 0x9:
		return reg_interruptMask | 0x1F;
		break;
	case 0xA:
		return reg_addressControl | 0x3F;
		break;
	default:
		return 0xFF;
	}
}

void CartReu1750::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	switch (address & 0x1F)
	{
	case 0x0:
		break;
	case 0x1:
		reg_command = data;
		if ((data & 0x90) == 0x90) // Execute and without FF00 trigger disabled. 1 means disabled.
		{
			if (!transfer_started)
			{
				StartTransfer();
			}
		}

		break;
	case 0x2:
		reg_c64BaseAddress.byte.loByte = data;
		shadow_c64BaseAddress.byte.loByte = data;
		reg_c64BaseAddress.byte.hiByte = shadow_c64BaseAddress.byte.hiByte;
		break;
	case 0x3:
		reg_c64BaseAddress.byte.hiByte = data;
		shadow_c64BaseAddress.byte.hiByte = data;
		reg_c64BaseAddress.byte.loByte = shadow_c64BaseAddress.byte.loByte;
		break;
	case 0x4:
		reg_reuBaseAddress.byte.loByte = data;
		shadow_reuBaseAddress.byte.loByte = data;
		reg_reuBaseAddress.byte.hiByte = shadow_reuBaseAddress.byte.hiByte;
		break;
	case 0x5:
		reg_reuBaseAddress.byte.hiByte = data;
		shadow_reuBaseAddress.byte.hiByte = data;
		reg_reuBaseAddress.byte.loByte = shadow_reuBaseAddress.byte.loByte;
		break;
	case 0x6:
		reg_reuBankPointer = data;
		shadow_reuBankPointer = data;
		break;
	case 0x7:
		reg_transferLength.byte.loByte = data;
		shadow_transferLength.byte.loByte = data;
		reg_transferLength.byte.hiByte = shadow_transferLength.byte.hiByte;
		break;
	case 0x8:
		reg_transferLength.byte.hiByte = data;
		shadow_transferLength.byte.hiByte = data;
		reg_transferLength.byte.loByte = shadow_transferLength.byte.loByte;
		break;
	case 0x9:
		reg_interruptMask = data;

		if ((reg_interruptMask & 0x80) != 0 && (reg_interruptMask & reg_status & 0x60) != 0)
		{
			reg_status |= 0x80;
			m_pCpu->Set_CRT_IRQ(CurrentClock);
		}
		else
		{ 
			reg_status &= 0x7F;
			m_pCpu->Clear_CRT_IRQ();
		}

		break;
	case 0xA:
		reg_addressControl = data;
		break;
	default:
		break;
	}
}

bool CartReu1750::SnoopWrite(bit16 address, bit8 data)
{
	/* From VICE's testprogs\REU\reutiming2\readme.txt
	* 
	* When the DMA is triggered by writing to $ff00 using a RMW instruction, the
	* first "dummy" write will start the DMA immediatly, the CPU will be
	* disconnected from the bus instantly and the second write cycle will go to
	* "nowhere", so it will not end up in the computers RAM.
	*/

	bool tx = transfer_started;
	if (address == 0xFF00)	
	{
		if ((reg_command & 0x90) == 0x80) // Execute and FF00 trigger on
		{
			if (!tx)
			{
				StartTransfer();
			}
		}

	}

	// If the transfer was already started, we prevent the CPU write.
	return !tx;
}

bool CartReu1750::IsREU()
{
	return true;
}

bool CartReu1750::IsCartIOActive(bit16 address, bool isWriting)
{
	if ((address & 0xFF00) == 0xDF00) // leave open address space 0xDExx
	{
		return !transfer_started;
	}
	else
	{
		return false;
	}
}

void CartReu1750::UpdateIO()
{
	DMA = dma_on ? 0 : 1;
	m_iSelectedBank = 0;
	m_bIsCartIOActive = true;
	BankRom();
}

void CartReu1750::StartTransfer()
{
	transfer_started = true;
	transfer_finished = false;
	verify_error = false;
	swap_state = false;
	reg_command |= 0x10; // FF00 trigger set to 1 means disabled.
	verify_byte_from_reu = 0;
	verify_byte_from_c64 = 0;
	dma_on = false;
	DMA = dma_on ? 0 : 1;
	swap_started = false;
	runcount = 0;
}

void CartReu1750::ExecuteCycle(ICLK sysclock)
{
	ICLK clocks;
	clocks = sysclock - CurrentClock;
	while ((ICLKS)clocks-- > 0)
	{
		CurrentClock++;
		if (!transfer_started)
		{	
			continue;
		}

		runcount++;
		switch (reg_command & 0x3)
		{
		case 0://00 - data is transferred from C64/C128 to REU
			RunC64ToREU();
			break;
		case 1://01 – data is transferred from REU to C64/C128
			RunREUToC64();
			break;
		case 2://10 – data is exchanged between C64/C128 and REU
			RunSwap();
			break;
		case 3://11 – data is verified between C64/C128 and REU
			RunVerify();
			break;
		default:
			break;
		}
	}
}

/************
* RunC64ToREU
* 
* REU/reutiming2
* e.prg   P
* e2.prg  P
* e3.prg  P
* f.prg   P
* f2.prg  P
* g.prg   P
* g2.prg  P
*************/
void CartReu1750::RunC64ToREU()
{
	bit8 data;

	// Check VIC's BA
	ICLK countBALow = m_pVic->Get_CountBALow();
	if ((countBALow > 0) && (countBALow != 1 || (m_pVic->SpriteDMATurningOn() & 1) == 0))
	{
		// If sprite0 DMA is what caused BA to go low then delay the release of the DMA line.
		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
			if (FinishTransfer())
			{
				return;
			}
		}
	}

	if (countBALow == 0)
	{
		if (!dma_on)
		{
			dma_on = true;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyLow(CurrentClock);
			if (FinishTransfer())
			{
				return;
			}
		}
	}

	if (!dma_on)
	{
		return;
	}

	if (FinishTransfer())
	{
		return;
	}

	// data is transferred from C64/C128 to REU
	data = ReadByteFromC64(reg_c64BaseAddress.word, countBALow == 1);
	WriteByteToREU(reg_reuBankPointer, reg_reuBaseAddress.word, data);

	// Update counters.
	UpdateTransferAddressAndCounter();
}

/************
* RunREUToC64
*
* REU/reutiming2
* a.prg   P
* a2.prg  P
* b.prg   P
* b2.prg  P
* b3.prg  P
* c.prg   ?
* c2.prg  P
* d.prg   P
* d2.prg  P
*************/
void CartReu1750::RunREUToC64()
{
	bit8 data;

	// Check VIC's BA
	ICLK countBALow = m_pVic->Get_CountBALow();

	/*
	* In order get to get badoublewite.prg to work then:
	* If BA goes low on the first cycle of a transfer then a DMA byte is written to the C64,
	* DMA will be released in the second half of this cycle,
	* and the memory pointers and counter will not change.
	*/
	if (countBALow == 0 || (countBALow == 1 && runcount == 1)) // badoublewite.prg
	{
		if (!dma_on)
		{
			dma_on = true;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyLow(CurrentClock);
			if (FinishTransfer())
			{
				return;
			}
		}
	}

	if (countBALow > 0 && reg_transferLength.word == 1 && !transfer_finished && runcount > 1)
	{
		/*
		* From VICE's testprogs\REU\reutiming2\readme.txt
		* 
		* When a REU DMA ends in the same cycle when a VIC DMA starts, it will take one
		* extra cycle
		*/

		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		}
	}

	if (!dma_on)
	{
		return;
	}

	if (FinishTransfer())
	{
		return;
	}

	// data is transferred from REU to C64/C128
	data = ReadByteFromREU(reg_reuBankPointer, reg_reuBaseAddress.word);
	WriteByteToC64(reg_c64BaseAddress.word, data);

	if (countBALow == 1 && runcount == 1)
	{
		// testprogs\REU\reutiming2\badoublewite.prg
		// We do not increment the counters. This may produce a second write to the same address in the next cycle.
		// Unfortunately, the VICE tests do not currently probe whether sprint0 DMA turning on bug can delay release of the DMA line in this condition.
		// It might be that if sprite0 DMA is what caused BA to go low then we should continue to increment the counters,
		// but this has not been tested.
		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		}

		return;
	}

	// Update counters.
	UpdateTransferAddressAndCounter();

	if ((countBALow > 0) && (countBALow != 1 || (m_pVic->SpriteDMATurningOn() & 1) == 0))
	{
		// If sprite0 DMA is what caused BA to go low then delay the release of the DMA line.
		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		}
	}
}

/************
* RunSwap
*
* a3.prg
* a4.prg  
* b4.prg  
* b5.prg  
* b6.prg  
* c3.prg  
* c4.prg  
* d3.prg  
* d4.prg  
* e4.prg  
* e5.prg  
* f3.prg P
* f4.prg P
* g3.prg P
* g4.prg P
*/
void CartReu1750::RunSwap()
{
	bool ba_rising = false;
	// Check VIC's BA
	ICLK countBALow = m_pVic->Get_CountBALow();
	if (countBALow == 0)
	{
		if (!dma_on)
		{
			dma_on = true;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyLow(CurrentClock);
			ba_rising = true;
			if (FinishTransfer())
			{
				return;
			}
		}
	}

	if (swap_started)
	{
		swap_state = !swap_state;
	}

	if (!dma_on)
	{
		return;
	}

	if (FinishTransfer())
	{
		return;
	}
	
	if (!swap_started)
	{
		// The swap state continues to be toggled even while DMA is paused.
		swap_state = !swap_state;
	}

	// data is exchanged between C64/C128 and REU
	if (swap_state)
	{
		swap_byte_from_reu = ReadByteFromREU(reg_reuBankPointer, reg_reuBaseAddress.word);
		swap_byte_from_c64 = ReadByteFromC64(reg_c64BaseAddress.word, countBALow == 1);
		swap_started = true;
	}
	else
	{
		if (countBALow > 0)
		{
			if (dma_on)
			{
				dma_on = false;
				DMA = dma_on ? 0 : 1;
				m_pCpu->SetCartridgeRdyHigh(CurrentClock);
			}

			return;
		}

		WriteByteToREU(reg_reuBankPointer, reg_reuBaseAddress.word, swap_byte_from_c64);
		WriteByteToC64(reg_c64BaseAddress.word, swap_byte_from_reu);
	}

	if (countBALow > 0)
	{
		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		}

		return;
	}


	// Update counters.
	UpdateTransferAddressAndCounter();
	if (ba_rising)
	{
		if (FinishTransfer())
		{
			return;
		}
	}
}

void CartReu1750::RunVerify()
{
	// Check VIC's BA
	ICLK countBALow = m_pVic->Get_CountBALow();
	if (countBALow == 0)
	{
		if (!dma_on)
		{
			dma_on = true;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyLow(CurrentClock);
			if (FinishTransfer())
			{
				return;
			}
		}
	}

	if (!dma_on)
	{
		return;
	}

	verify_error = verify_byte_from_c64 != verify_byte_from_reu;
	if (verify_error)
	{
		reg_status |= 0x20; // Verify error
	}

	if (FinishTransfer())
	{
		return;
	}

	//data is verified between C64/C128 and REU
	verify_byte_from_reu = ReadByteFromREU(reg_reuBankPointer, reg_reuBaseAddress.word);
	verify_byte_from_c64 = ReadByteFromC64(reg_c64BaseAddress.word, countBALow == 1);
	if (verify_error)
	{
		transfer_finished = true;
		return;
	}

	// Update counters.
	UpdateTransferAddressAndCounter();

	if ((countBALow > 0) && (countBALow != 1 || (m_pVic->SpriteDMATurningOn() & 1) == 0))
	{
		// If sprite0 DMA is what caused BA to go low then delay the release of the DMA line.
		if (dma_on)
		{
			dma_on = false;
			DMA = dma_on ? 0 : 1;
			m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		}
	}
}

void CartReu1750::UpdateTransferAddressAndCounter()
{
	if (!swap_state)
	{
		if (((reg_addressControl & 0x40) == 0))
		{
			reg_reuBaseAddress.word++;
			if (reg_reuBaseAddress.word == 0)
			{
				reg_reuBankPointer = (reg_reuBankPointer + 1) & reu_extraAddressMask;
			}
		}

		if ((reg_addressControl & 0x80) == 0)
		{
			reg_c64BaseAddress.word++;
		}

		if (reg_transferLength.word == 1)
		{
			reg_status = reg_status | 0x40; // Set End of Block
			transfer_finished = true;
		}
		else
		{
			--reg_transferLength.word;
		}
	}
}

bool CartReu1750::FinishTransfer()
{
	if (transfer_finished)
	{
		if (reg_transferLength.word == 1 && !verify_error)
		{
			reg_status = reg_status | 0x40; // Set End of Block
		}

		if ((reg_command & 0x20) != 0) // Autoload
		{
			reg_reuBankPointer = shadow_reuBankPointer;
			reg_reuBaseAddress = shadow_reuBaseAddress;
			reg_c64BaseAddress = shadow_c64BaseAddress;
			reg_transferLength = shadow_transferLength;
		}

		reg_command = (reg_command & 0x7F) | 0x10; // Clear Execute; Set FF00 trigger (1 is disabled)

		if ((reg_interruptMask & 0x80) != 0 && (reg_interruptMask & reg_status & 0x60) != 0)
		{
			reg_status |= 0x80;
			m_pCpu->Set_CRT_IRQ(CurrentClock);
		}

		dma_on = false;
		DMA = dma_on ? 0 : 1;
		transfer_started = false;
		m_pCpu->SetCartridgeRdyHigh(CurrentClock);
		return true;
	}
	else
	{
		return false;
	}
}


bit8 CartReu1750::ReadByteFromREU(bit8 bank, bit16 address)
{
	bank = bank & reu_extraAddressMask;
	return this->m_pCartData[(unsigned int)bank * 0x10000 + address];
}

bit8 CartReu1750::ReadByteFromC64(bit16 address, bool startingVicDMA)
{
	if (startingVicDMA && address >= 0xD000 && address <= 0xDFFF && m_pCpu->GetCurrentCpuMmuReadMemoryType(address) == MT_IO)
	{
		/*
		* It is reported that the C64C has a bug where it can read from RAM instead of IO
		* The older C64 reads from IO as expected.

		* From VICE's testprogs\REU\reutiming2\readme.txt
		* 
		* When reading from I/O, the first byte of a transfer that is started in the
		* same cycle as a VIC DMA would be may come from the RAM under the I/O instead.
		* This seems to happen (to be confirmed) when the "new" VICII is used and/or
		* a "new" motherboard is used (C64C) - on "breadbox" with "old" VICII we
		* apparently get the expected value from I/O (see the -m2 test variants).		
		*/
		return this->m_pC64RamMemory[address];
	}
	else
	{
		return m_pCpu->ReadDmaByte(address);
	}
}

void CartReu1750::WriteByteToREU(bit8 bank, bit16 address, bit8 data)
{
	bank = bank & reu_extraAddressMask;
	this->m_pCartData[(unsigned int)bank * 0x10000 + address] = data;
}

void CartReu1750::WriteByteToC64(bit16 address, bit8 data)
{
	m_pCpu->WriteDmaByte(address, data);
}
