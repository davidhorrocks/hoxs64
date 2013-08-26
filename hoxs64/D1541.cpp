#include <windows.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "c6502.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "d64.h"
#include "d1541.h"
#include "diskinterface.h"

CPUDisk::CPUDisk()
{
	pIC64Event = 0;
	pMappedRAM=0;
	pMappedROM=0;
	via1=0;
	via2=0;
}

CPUDisk::~CPUDisk()
{
}

void CPUDisk::InitReset(ICLK sysclock)
{
	CPU6502::InitReset(sysclock);
	IRQ_VIA1=0;	
	IRQ_VIA2=0;
	NMI_TRIGGER=0;
}

void CPUDisk::Reset(ICLK sysclock)
{
	InitReset(sysclock);
	CPU6502::Reset(sysclock);
}


HRESULT CPUDisk::Init(IC64Event *pIC64Event, int ID, IRegister *via1, IRegister *via2, DiskInterface *disk, bit8 *pMappedRAM, bit8 *pMappedROM, IBreakpointManager *pIBreakpointManager)
{
HRESULT hr;
	ClearError();
	hr = CPU6502::Init(ID, pIBreakpointManager);
	if (FAILED(hr))
		return hr;
	this->disk = disk;

	this->pIC64Event = pIC64Event;
	this->via1 = via1;
	this->via2 = via2;
	this->disk = disk;
	this->pMappedRAM = pMappedRAM;
	this->pMappedROM = pMappedROM;
	return S_OK;
}

void CPUDisk::SyncChips()
{
	via1->ExecuteCycle(CurrentClock);
	via2->ExecuteCycle(CurrentClock);
}

void CPUDisk::check_interrupts1()
{
	via1->ExecuteCycle(CurrentClock);
	via2->ExecuteCycle(CurrentClock);
	CPU6502::check_interrupts1();
}

void CPUDisk::check_interrupts0()
{
	via1->ExecuteCycle(CurrentClock);
	via2->ExecuteCycle(CurrentClock);
	CPU6502::check_interrupts0();
}

void CPUDisk::SyncVFlag()
{
	//disk->SpinDisk(CurrentClock);
	via2->ExecuteCycle(CurrentClock);
	if (SOTrigger &&  ((ICLKS)(CurrentClock - SOTriggerClock)>0))
	{
		SOTrigger = false;
		fOVERFLOW = 1;
	}
}

bit8 CPUDisk::ReadByte(bit16 address)
{	
	if (address >= 0x8000)
	{
		return pMappedROM[address | 0xC000];
	}
	else
	{
		address = address & 0x1fff;
		if (address < 0x0800)
		{
			return pMappedRAM[address];
		}
		else if (address >= 0x1c00)
		{
			return via2->ReadRegister(address, CurrentClock);
		}
		else if (address >= 0x1800)
		{
			return via1->ReadRegister(address, CurrentClock);
		}
		else
			return address >> 8;
	}
}

bit8 CPUDisk::MonReadByte(bit16 address, int memorymap)
{
	if (address >= 0x8000)
	{
		return pMappedROM[address | 0xC000];
	}
	else
	{
		address = address & 0x1fff;
		if (address < 0x0800)
		{
			return pMappedRAM[address];
		}
		else if (address >= 0x1c00)
		{
			return via2->ReadRegister_no_affect(address, CurrentClock);
		}
		else if (address >= 0x1800)
		{
			return via1->ReadRegister_no_affect(address, CurrentClock);
		}
		else
			return address >> 8;
	}
}

void CPUDisk::WriteByte(bit16 address, bit8 data)
{
	if (address < 0x8000)
	{
		address = address & 0x1fff;
		if (address < 0x0800)
		{
			pMappedRAM[address] = data;
		}
		else if (address >= 0x1c00)
		{
			return via2->WriteRegister(address, CurrentClock, data);
		}
		else if (address >= 0x1800)
		{
			return via1->WriteRegister(address, CurrentClock, data);
		}
	}
}

void CPUDisk::MonWriteByte(bit16 address, bit8 data, int memorymap)
{
	if (address < 0x8000)
	{
		address = address & 0x1fff;
		if (address < 0x0800)
		{
			pMappedRAM[address] = data;
		}
		else if (address >= 0x1c00)
		{
			return via2->WriteRegister(address, CurrentClock, data);
		}
		else if (address >= 0x1800)
		{
			return via1->WriteRegister(address, CurrentClock, data);
		}
	}
}

int CPUDisk::GetCurrentCpuMmuMemoryMap()
{
	return 0;
}

MEM_TYPE CPUDisk::GetCpuMmuReadMemoryType(bit16 address, int memorymap)
{
	if (address < 0x8000)
	{
		address = address & 0x1fff;
		if (address < 0x0800)
		{
			return MT_RAM;
		}
		else if (address >= 0x1c00)
		{
			return MT_IO;
		}
		else if (address >= 0x1800)
		{
			return MT_IO;
		}
	}
	return MT_NOTCONNECTED;
}

MEM_TYPE CPUDisk::GetCpuMmuWriteMemoryType(bit16 address, int memorymap)
{
	return GetCpuMmuReadMemoryType(address, memorymap);
}

//985248 PAL
//1022727 NTSC

void CPUDisk::Set_VIA1_IRQ(ICLK sysclock)
{
	SetIRQ(sysclock);
	IRQ_VIA1 = 1;
}
void CPUDisk::Clear_VIA1_IRQ()
{
	if (IRQ_VIA2==0)
		ClearSlowIRQ();
	IRQ_VIA1 = 0;
}
void CPUDisk::Set_VIA2_IRQ(ICLK sysclock)
{
	SetIRQ(sysclock);
	IRQ_VIA2 = 1;
}
void CPUDisk::Clear_VIA2_IRQ()
{
	if (IRQ_VIA1==0)
		ClearSlowIRQ();
	IRQ_VIA2 = 0;
}

bit8 CPUDisk::ReadRegister(bit16 address, ICLK sysclock)
{
	return 0;
}

void CPUDisk::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
}

bit8 CPUDisk::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	return 0;
}

ICLK CPUDisk::GetCurrentClock()
{
	return CurrentClock;
}

void CPUDisk::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CPU6502::SetCurrentClock(sysclock);
}
