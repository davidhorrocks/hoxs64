#include <windows.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "defines.h"
#include "tchar.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "hexconv.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "vic6569.h"
#include "tap.h"
#include "c64keys.h"
#include "cia1.h"
#include "cia2.h"

CPU6510::CPU6510()
{
	pIC64Event = NULL;
	m_piColourRAM = NULL;
	m_ppMemory_map_read = NULL;
	m_ppMemory_map_write = NULL;
	cia1 = NULL;
	cia2 = NULL;
	vic = NULL;
	sid = NULL;
	ram = NULL;
	tape = NULL;
	m_bIsWriteCycle = false;
}

CPU6510::~CPU6510()
{
}

void CPU6510::Reset(ICLK sysclock)
{
	m_fade7clock = sysclock;
	m_fade6clock = sysclock;
	m_bIsWriteCycle = false;
	IRQ_VIC=0;	
	IRQ_CIA=0;
	NMI_CIA=0;
	NMI_TRIGGER=0;

	LORAM=0;
	HIRAM=0;
	CHAREN=0;
	CASSETTE_WRITE=0;
	CASSETTE_MOTOR=0;
	CASSETTE_SENSE=1;
	cpu_io_ddr=0;
	cpu_io_data=0;
	cpu_io_output=0;
	write_cpu_io_ddr(0, sysclock);
	write_cpu_io_data(0);
	CPU6502::Reset(sysclock);
}

HRESULT CPU6510::Init(IC64Event *pIC64Event, int ID, IRegister *cia1, IRegister *cia2, IRegister *vic, IRegister *sid, RAM64 *ram, ITape *tape)
{
HRESULT hr;
	ClearError();
	hr = CPU6502::Init(ID);
	if (FAILED(hr))
		return hr;

	this->pIC64Event = pIC64Event;
	this->cia1 = cia1;
	this->cia2 = cia2;
	this->vic = vic;
	this->sid = sid;
	this->ram = ram;
	this->tape = tape;

	pCia1 = static_cast<CIA1 *>(cia1);
	pCia2 = static_cast<CIA2 *>(cia2);
	pVic = static_cast<VIC6569 *>(vic);

	if (ram->miIO == NULL)
		return SetError(E_FAIL, TEXT("Please call ram->Init() before calling cpu6510->Init()"));

	ram->ConfigureMMU(0, &m_ppMemory_map_read, &m_ppMemory_map_write);

	m_piColourRAM = ram->miIO;
	return S_OK;
}

bit8 CPU6510::ReadByte(bit16 address)
{
bit8 *t;
	if (address < 0xA000 && address > 1)
		return m_ppMemory_map_read[0][address];
	t=m_ppMemory_map_read[address >> 12];
	if (t)
		if (address>1)
			return t[address];
		else 
		{
			vic->ExecuteCycle(CurrentClock);
			return ReadRegister(address, CurrentClock);
		}
	else
	{
		switch (address >> 8)
		{
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
			return vic->ReadRegister(address, CurrentClock);
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
			vic->ExecuteCycle(CurrentClock);
			return sid->ReadRegister(address, CurrentClock);
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
			return m_piColourRAM[address] | (vic->ReadRegister(0xDE00, CurrentClock) & 0xF0);
		case 0xDC:
			vic->ExecuteCycle(CurrentClock);
			return cia1->ReadRegister(address, CurrentClock);
		case 0xDD:
			vic->ExecuteCycle(CurrentClock);
			return cia2->ReadRegister(address, CurrentClock);
		default:
			return vic->ReadRegister(address, CurrentClock);
		}
	}
}

#define USECPU6510WRITEOPTIMISATION (0)

void CPU6510::WriteByte(bit16 address, bit8 data)
{
bit8 *t;

#if USECPU6510WRITEOPTIMISATION == 1 
	if ((ICLKS)(pVic->nextWakeUpClock - CurrentClock) <=0 || ((address >> 14) == pVic->vicMemoryBankIndex))
	{
		m_bIsWriteCycle = true;
		vic->ExecuteCycle(CurrentClock);
		m_bIsWriteCycle = false;
	}
#else
		m_bIsWriteCycle = true;
		vic->ExecuteCycle(CurrentClock);
		m_bIsWriteCycle = false;
#endif
	if (address>1)
	{
		t=m_ppMemory_map_write[address >> 12];
		if (t)
			t[address]=data;
		else
		{
			switch (address >> 8)
			{
			case 0xD0:
			case 0xD1:
			case 0xD2:
			case 0xD3:
				m_bIsWriteCycle = true;
				vic->WriteRegister(address, CurrentClock, data);
				m_bIsWriteCycle = false;
				break;
			case 0xD4:
			case 0xD5:
			case 0xD6:
			case 0xD7:
				sid->WriteRegister(address, CurrentClock, data);
				break;
			case 0xD8:
			case 0xD9:
			case 0xDA:
			case 0xDB:
#if USECPU6510WRITEOPTIMISATION == 1 
				m_bIsWriteCycle = true;
				vic->ExecuteCycle(CurrentClock);
				m_bIsWriteCycle = false;
#endif
				m_piColourRAM[address] = (data & 0x0f);
				break;
			case 0xDC:
				cia1->WriteRegister(address, CurrentClock, data);
				break;
			case 0xDD:
#if USECPU6510WRITEOPTIMISATION == 1 
				if (address == 0xDD00)
				{
					//Possible vic memory bank change
					m_bIsWriteCycle = true;
					vic->ExecuteCycle(CurrentClock);
					m_bIsWriteCycle = false;
				}
#endif
				cia2->WriteRegister(address, CurrentClock, data);
				break;
			}
		}
	}
	else
	{
#if USECPU6510WRITEOPTIMISATION == 1 
		//de00_byte must be up to date.
		m_bIsWriteCycle = true;
		vic->ExecuteCycle(CurrentClock);
		m_bIsWriteCycle = false;
#endif
		WriteRegister(address, CurrentClock, data);
		ram->miMemory[address & 1] = pVic->de00_byte;
	}
}

bit8 CPU6510::MonReadByte(bit16 address, int memorymap)
{
bit8 *t;
	if (memorymap < 0)//Use the MMU
		t=m_ppMemory_map_read[address >> 12];
	else
		t= ram->MMU_read[memorymap & 0x1f][address >> 12];

	if (t)
		if (address>1)
			return t[address];
		else 
			return ReadRegister_no_affect(address, CurrentClock);
	else
	{
		switch (address >> 8)
		{
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
			return vic->ReadRegister_no_affect(address, CurrentClock);
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
			return sid->ReadRegister_no_affect(address, CurrentClock);
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
			return m_piColourRAM[address] | (vic->ReadRegister(address, CurrentClock) & 0xF0);
		case 0xDC:
			return cia1->ReadRegister_no_affect(address, CurrentClock);
		case 0xDD:
			return cia2->ReadRegister_no_affect(address, CurrentClock);
		default:
			return vic->ReadRegister(address, CurrentClock);
		}
	}
}

void CPU6510::MonWriteByte(bit16 address, bit8 data, int memorymap)
{
}

void CPU6510::GetCpuState(CPUState& state)
{
	CPU6502::GetCpuState(state);
	state.PortDdr = cpu_io_ddr;
	state.PortDataStored = cpu_io_data;
}

void CPU6510::SyncChips()
{
	ICLK& curClock = CurrentClock;
	if ((ICLKS)(pVic->ClockNextWakeUpClock - curClock) <=0)
		vic->ExecuteCycle(curClock);
	if ((ICLKS)(pCia1->ClockNextWakeUpClock - curClock) <=0)
		cia1->ExecuteCycle(curClock);
	if ((ICLKS)(pCia2->ClockNextWakeUpClock - curClock) <=0)
		cia2->ExecuteCycle(curClock);
}

void CPU6510::AddClockDelay()
{
	++CurrentClock;
	++m_CurrentOpcodeClock;
}

void CPU6510::check_interrupts1()
{
	ICLK& curClock = CurrentClock;
	if ((ICLKS)(pVic->ClockNextWakeUpClock - curClock) <=0)
		vic->ExecuteCycle(curClock);
	if ((ICLKS)(pCia1->ClockNextWakeUpClock - curClock) <=0)
		cia1->ExecuteCycle(curClock);
	if ((ICLKS)(pCia2->ClockNextWakeUpClock - curClock) <=0)
		cia2->ExecuteCycle(curClock);
	CPU6502::check_interrupts1();
}

void CPU6510::check_interrupts0()
{
ICLK& curClock = CurrentClock;
	vic->ExecuteCycle(curClock);
	cia1->ExecuteCycle(curClock);
	cia2->ExecuteCycle(curClock);
	CPU6502::check_interrupts0();
}


bit8 CPU6510::ReadRegister(bit16 address, ICLK sysclock)
{
	if (address==0)
		return cpu_io_ddr;
	else
	{
		CheckPortFade(sysclock);
		return cpu_io_readoutput;
	}
}

void CPU6510::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (address==0)
	{
		write_cpu_io_ddr(data, sysclock);
	}
	else
	{
		write_cpu_io_data(data);
	}
	static_cast<CIA1 *>(cia1)->SetWakeUpClock();
}

bit8 CPU6510::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	if (address==0)
		return cpu_io_ddr;
	else
	{
		CheckPortFade(sysclock);
		return cpu_io_readoutput;
	}
}

void CPU6510::CheckPortFade(ICLK sysclock)
{
bit8 f;
bool bFaded = false;

	//find pins that have 1s that are circulating in input mode
	f = ~cpu_io_ddr & cpu_io_output;
	if (f & 128)
	{
		if ((ICLKS)(sysclock - m_fade7clock) > 246312)
		{
			cpu_io_output &= 0x7f;
			bFaded = true;
		}
	}
	if (f & 64)
	{
		if ((ICLKS)(sysclock - m_fade6clock) > 246312)
		{
			cpu_io_output &= 0xbf;
			bFaded = true;
		}
	}
	if (bFaded)
		cpu_port();
}

void CPU6510::write_cpu_io_data(bit8 data)
{
	cpu_io_data = data;
	cpu_port();
}

void CPU6510::write_cpu_io_ddr(bit8 data, ICLK sysclock)
{
bit8 f;

	//find which pins have just changed to input mode
	f = (cpu_io_ddr ^ data) & cpu_io_ddr;
	if (f & 128)
	{
		m_fade7clock = sysclock;
	}
	if (f & 64)
	{
		m_fade6clock = sysclock;
	}

	cpu_io_ddr=data;
	cpu_port();
}

//       7 |        6 |     5 |     4 |     3 |       2 |     1 |      0
//NOT USED | NOT USED | MOTOR | SENSE | WRITE | CHARGEN | HIRAM | LOWRAM
void CPU6510::cpu_port(){
//port reads pin levels for pins that are in input mode
//port reads the output register for pins that are in output mode
//CASSETTE MOTOR pin reads zero

    cpu_io_output = (cpu_io_output & ~cpu_io_ddr) | (cpu_io_data & cpu_io_ddr);

    cpu_io_readoutput = ((cpu_io_data | ~cpu_io_ddr) & (cpu_io_output | 0x17));


    if (!(cpu_io_ddr & 0x20))
      cpu_io_readoutput &= 0xdf;

    if (!CASSETTE_SENSE && !(cpu_io_ddr & 0x10))
      cpu_io_readoutput &= 0xef;

    CASSETTE_MOTOR = ((cpu_io_ddr & cpu_io_data) & 0x20)!=0;

    CASSETTE_WRITE = ((~cpu_io_ddr | cpu_io_data) & 0x8)!=0;

	if (cpu_io_ddr & 0x01)
		LORAM=cpu_io_data & 0x01;
	else
		LORAM=1;

	if (cpu_io_ddr & 0x02)
		HIRAM=(cpu_io_data & 0x02)>>1;	
	else
		HIRAM=1;

	if (cpu_io_ddr & 0x04)
		CHAREN=(cpu_io_data & 0x04)>>2;
	else
		CHAREN=1;

	ram->ConfigureMMU( 0x03 | LORAM<<3 | HIRAM<<2 | CHAREN<<4, &m_ppMemory_map_read, &m_ppMemory_map_write);
	tape->SetMotorWrite(!CASSETTE_MOTOR, CASSETTE_WRITE);
}

void CPU6510::SetCassetteSense(bit8 sense)
{
	CASSETTE_SENSE = sense;
	cpu_port();
}

void CPU6510::Set_VIC_IRQ(ICLK sysclock)
{
	SetIRQ(sysclock);
	IRQ_VIC = 1;
}
void CPU6510::Clear_VIC_IRQ()
{
	if (IRQ_CIA==0)
		ClearSlowIRQ();
	IRQ_VIC = 0;
}
void CPU6510::Set_CIA_IRQ(ICLK sysclock)
{
	SetIRQ(sysclock);
	IRQ_CIA = 1;
}
void CPU6510::Clear_CIA_IRQ()
{
	if (IRQ_VIC==0)
		ClearSlowIRQ();
	IRQ_CIA = 0;
}
void CPU6510::Set_CIA_NMI(ICLK sysclock)
{
	SetNMI(sysclock);
}
void CPU6510::Clear_CIA_NMI()
{
	ClearNMI();
}

void CPU6510::SetDdr(bit8 v)
{
	write_cpu_io_ddr(v, CurrentClock);
}

void CPU6510::SetData(bit8 v)
{
	write_cpu_io_data(v);
}

