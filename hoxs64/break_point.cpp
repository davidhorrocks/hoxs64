#include <windows.h>
#include "dx_version.h"
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"
#include "c6502.h"
#include "break_point.h"

void CPU6502::ClearAll()
{
	br_execute_size=0;
	br_read_size=0;
	br_write_size=0;
	m_bBreakOnInterruptTaken = false;
}

void CPU6502::SetBreakOnInterruptTaken()
{
	m_bBreakOnInterruptTaken = true;
}

void CPU6502::ClearBreakOnInterruptTaken()
{
	m_bBreakOnInterruptTaken = false;
}

bool CPU6502::GetBreakOnInterruptTaken()
{
	return m_bBreakOnInterruptTaken ;
}


bool CPU6502::SetExecute(bit16 address, unsigned long count)
{
int i;
	i=FindExecuteIndex(address);
	if (i != -1)
		RemoveExecute(i);
	if (br_execute_size >= BREAK_LIST_SIZE)
		return false;

	br_execute[br_execute_size].address = address;
	br_execute[br_execute_size].count = count;
	br_execute[br_execute_size].value = -1;
	br_execute_size++;
	return true;
}

bool CPU6502::SetRead(bit16 address, short value, unsigned long count)
{
int i;
	i=FindReadIndex(address);
	if (i != -1)
		RemoveRead(i);
	if (br_read_size >= BREAK_LIST_SIZE)
		return false;

	br_read[br_read_size].address = address;
	br_read[br_read_size].count = count;
	br_read[br_read_size].value = value;
	br_read_size++;
	return true;
}


bool CPU6502::SetWrite(bit16 address, short value, unsigned long count)
{
int i;
	i=FindWriteIndex(address);
	if (i != -1)
		RemoveWrite(i);
	if (br_write_size >= BREAK_LIST_SIZE)
		return false;

	br_write[br_write_size].address = address;
	br_write[br_write_size].count = count;
	br_write[br_write_size].value = value;
	br_write_size++;
	return true;
}


void CPU6502::RemoveExecute(int index)
{
int i;
	if (index >= br_execute_size)
		return;
	for (i=index ; i < (br_execute_size - 1) ; i++)
	{
		memcpy(&br_execute[i], &br_execute[i+1], sizeof(struct cb_break));
	}
	br_execute_size--;
}

void CPU6502::RemoveRead(int index)
{
int i;
	if (index >= br_read_size)
		return;
	for (i=index ; i < (br_read_size - 1) ; i++)
	{
		memcpy(&br_read[i], &br_read[i+1], sizeof(struct cb_break));
	}
	br_read_size--;
}

void CPU6502::RemoveWrite(int index)
{
int i;
	if (index >= br_write_size)
		return;
	for (i=index ; i < (br_write_size - 1) ; i++)
	{
		memcpy(&br_write[i], &br_write[i+1], sizeof(struct cb_break));
	}
	br_write_size--;
}


int CPU6502::CheckExecute(bit16 address)
{
int i;
	i=FindExecuteIndex(address);
	if (i != -1)
	{
		if (br_execute[i].count<=1)
			return i;
		br_execute[i].count--;
	}
	return -1;
} 

int CPU6502::CheckRead(bit16 address)
{
int i;
	i=FindReadIndex(address);
	if (i != -1)
	{
		if (br_read[i].count<=1)
			return i;
		br_read[i].count--;
	}
	return -1;
} 

int CPU6502::CheckWrite(bit16 address)
{
int i;
	i=FindWriteIndex(address);
	if (i != -1)
	{
		if (br_write[i].count<=1)
			return i;
		br_write[i].count--;
	}
	return -1;
} 

int CPU6502::FindExecuteIndex(bit16 address)
{
int i;
	for (i=0 ; i < (br_execute_size) ; i++)
	{
		if (address == br_execute[i].address)
			return i;
	}	
	return -1;
}

int CPU6502::FindReadIndex(bit16 address)
{
int i;
	for (i=0 ; i < (br_read_size) ; i++)
	{
		if (address == br_read[i].address)
			return i;
	}	
	return -1;
}

int CPU6502::FindWriteIndex(bit16 address)
{
int i;
	for (i=0 ; i < (br_write_size) ; i++)
	{
		if (address == br_write[i].address)
			return i;
	}	
	return -1;
}


bool CPU6502::IsBreakPoint(bit16 address)
{
	if (FindExecuteIndex(address) != -1)
		return true;
	if (FindReadIndex(address) != -1)
		return true;
	if (FindWriteIndex(address) != -1)
		return true;
	return false;
}


void CPU6502::ClearBreakPoint(bit16 address)
{
int i;

	for (i=0 ; i < (br_execute_size) ; i++)
	{
		if (address == br_execute[i].address)
		{
			RemoveExecute(i);
		}
	}	
	for (i=0 ; i < (br_read_size) ; i++)
	{
		if (address == br_read[i].address)
		{
			RemoveRead(i);
		}
	}	
	for (i=0 ; i < (br_write_size) ; i++)
	{
		if (address == br_write[i].address)
		{
			RemoveWrite(i);
		}
	}	
}

void CPU6502::StartDebug()
{
	m_bDebug = 1;
}

void CPU6502::StopDebug()
{
	m_bDebug = 0;
}
