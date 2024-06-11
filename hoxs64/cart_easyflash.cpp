#include "cart.h"

CartEasyFlash::CartEasyFlash(const CrtHeader& crtHeader, IC6510* pCpu, IVic* pVic, bit8* pC64RamMemory)
	: CartCommon(crtHeader, pCpu, pVic, pC64RamMemory)
{

}

CartEasyFlash::~CartEasyFlash()
{
	m_EasyFlashChipROML.Detach();
	m_EasyFlashChipROMH.Detach();
}

HRESULT CartEasyFlash::InitCart(CartData& cartData)
{
HRESULT hr=E_FAIL;

	try
	{
		*((CartData*)this) = std::move(cartData);
		do
		{
			if (m_plstBank->size() < EasyFlashChip::MAXEASYFLASHBANKS)
			{
				m_plstBank->resize(EasyFlashChip::MAXEASYFLASHBANKS);
			}

			hr = m_EasyFlashChipROML.Init(this, 0);
			if (FAILED(hr))
			{
				break;
			}

			hr = m_EasyFlashChipROMH.Init(this, 1);
			if (FAILED(hr))
			{
				break;
			}
		} while (false);
	}
	catch (std::exception&)
	{
		hr = E_FAIL;
	}

	return hr;
}

void CartEasyFlash::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	reg1 = 0;
	reg2 = 0;
	m_EasyFlashChipROML.Reset(sysclock);
	m_EasyFlashChipROMH.Reset(sysclock);
	ConfigureMemoryMap();
}

bit8 CartEasyFlash::ReadRegister(bit16 address, ICLK sysclock)
{
bit16 addr;
	if (address >= 0xDF00 && address < 0xE000)
	{
		addr = address - 0xDF00;
		return m_pCartData[addr];
	}
	else if (address == 0xDE00 || address == 0xDE02)
	{
		return 0;
	}
	return 0;
}

void CartEasyFlash::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address == 0xDE00)
	{
		reg1 = data;
		ConfigureMemoryMap();
	}
	else if (address == 0xDE02)
	{
		reg2 = data;
		ConfigureMemoryMap();
	}
	else if (address >= 0xDF00 && address < 0xE000)
	{
		m_pCartData[address - 0xDF00] = data;
	}
}

bit8 CartEasyFlash::ReadROML(bit16 address)
{
	return this->m_EasyFlashChipROML.ReadByte(address);
}

bit8 CartEasyFlash::ReadROMH(bit16 address)
{
	return this->m_EasyFlashChipROMH.ReadByte(address);
}

bit8 CartEasyFlash::ReadUltimaxROML(bit16 address)
{
	return this->m_EasyFlashChipROML.ReadByte(address);
}

bit8 CartEasyFlash::ReadUltimaxROMH(bit16 address)
{
	return this->m_EasyFlashChipROMH.ReadByte(address);
}

void CartEasyFlash::WriteROML(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROML.WriteByte(address, data);
}

void CartEasyFlash::WriteROMH(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROMH.WriteByte(address, data);
}

void CartEasyFlash::WriteUltimaxROML(bit16 address, bit8 data)
{
	this->m_EasyFlashChipROML.WriteByte(address, data);
}

void CartEasyFlash::WriteUltimaxROMH(bit16 address, bit8 data)
{
	this->m_EasyFlashChipROMH.WriteByte(address, data);
}

bit8 CartEasyFlash::MonReadROML(bit16 address)
{
	return this->m_EasyFlashChipROML.MonReadByte(address);
}

bit8 CartEasyFlash::MonReadROMH(bit16 address)
{
	return this->m_EasyFlashChipROMH.MonReadByte(address);
}

bit8 CartEasyFlash::MonReadUltimaxROML(bit16 address)
{
	return this->m_EasyFlashChipROML.MonReadByte(address);
}

bit8 CartEasyFlash::MonReadUltimaxROMH(bit16 address)
{
	return this->m_EasyFlashChipROMH.MonReadByte(address);
}

void CartEasyFlash::MonWriteROML(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROML.MonWriteByte(address, data);
}

void CartEasyFlash::MonWriteROMH(bit16 address, bit8 data)
{
	m_pC64RamMemory[address] = data;
	this->m_EasyFlashChipROMH.MonWriteByte(address, data);
}

void CartEasyFlash::MonWriteUltimaxROML(bit16 address, bit8 data)
{
	this->m_EasyFlashChipROML.MonWriteByte(address, data);
}

void CartEasyFlash::MonWriteUltimaxROMH(bit16 address, bit8 data)
{
	this->m_EasyFlashChipROMH.MonWriteByte(address, data);
}

void CartEasyFlash::UpdateIO()
{
	m_iSelectedBank = reg1 & 0x3f;
	if (reg2 & 4)
	{
		GAME = (reg2 & 0x1) == 0;
		EXROM = (reg2 & 0x2) == 0;
	}
	else
	{
		GAME = 0;
		EXROM = (reg2 & 0x2) == 0;
	}
	m_bIsCartIOActive = true;
	BankRom();
}

void CartEasyFlash::PreventClockOverflow()
{
	m_EasyFlashChipROML.PreventClockOverflow();
	m_EasyFlashChipROMH.PreventClockOverflow();
}

ICLK CartEasyFlash::GetCurrentClock()
{
	return this->m_pCpu->Get6510CurrentClock();
}

void CartEasyFlash::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - this->m_pCpu->Get6510CurrentClock();
	m_EasyFlashChipROML.SetCurrentClock(sysclock);
	m_EasyFlashChipROMH.SetCurrentClock(sysclock);
}

unsigned int CartEasyFlash::GetStateBytes(int version, void *pstate)
{
int k, c;
bit8 *p = (bit8 *)pstate;

	c = 0;
	k = CartCommon::GetStateBytes(version, p);
	if (p)
	{
		p += k;
	}

	c += k;
	k = m_EasyFlashChipROML.GetStateBytes(p);
	if (p)
	{
		p += k;
	}

	c += k;
	k = m_EasyFlashChipROMH.GetStateBytes(p);
	c += k;
	return c;
}

HRESULT CartEasyFlash::SetStateBytes(int version, void* pstate, unsigned int size)
{
	HRESULT hr;
	unsigned int k;

	int totalsize = GetStateBytes(LatestVersion, NULL);
	if (totalsize == 0 || !pstate || size != totalsize)
	{
		return E_FAIL;
	}

	bit8* p = (bit8*)pstate;
	k = CartCommon::GetStateBytes(LatestVersion, NULL);
	hr = CartCommon::SetStateBytes(version, p, k);
	if (FAILED(hr))
	{
		return hr;
	}

	p += k;
	k = m_EasyFlashChipROML.GetStateBytes(NULL);
	hr = m_EasyFlashChipROML.SetStateBytes(p, k);
	if (FAILED(hr))
	{
		return hr;
	}

	p += k;
	k = m_EasyFlashChipROMH.GetStateBytes(NULL);
	hr = m_EasyFlashChipROMH.SetStateBytes(p, k);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}
