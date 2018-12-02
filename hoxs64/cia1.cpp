#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "hexconv.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "vic6569.h"
#include "tap.h"
#include "c64keys.h"
#include "cia1.h"

#define KEYBOARDMINSCANINTERVAL (3000)
#define DEVICEACQUIRECLOCKS (500 * PALCLOCKSPERSECOND / 1000)

#define KEYMATRIX_DOWN(row,col) keyboard_matrix[row] = \
	(keyboard_matrix[row] & (bit8)~(1<<col));\
	keyboard_rmatrix[col] = \
	(keyboard_rmatrix[col] & (bit8)~(1<<row))

CIA1::CIA1()
	: dist_pal_frame(0, PAL_CLOCKS_PER_FRAME - 1)
{
	appStatus = 0L;
	dx = 0L;
	cpu = 0L;
	vic = 0L;
	tape64 = 0L;
	pIC64 = 0L;
	nextKeyboardScanClock = 0;
	ZeroMemory(&c64KeyMap[0], sizeof(c64KeyMap));
	m_bAltLatch = false;
	ResetKeyboard();
	restore_was_up=true;
	F12_was_up=true;
	F11_was_up=true;
	keyboardNotAcquiredClock = 0;
	ZeroMemory(&this->js, sizeof(this->js));
}

HRESULT CIA1::Init(CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, VIC6569 *vic, ISid *sid, Tape64 *tape64, CDX9 *dx)
{
	ClearError();
	this->ID = 1;
	this->appStatus = appStatus;
	this->dx = dx;
	this->cpu = cpu;
	this->vic = vic;
	this->sid = sid;
	this->tape64 = tape64;
	this->pIC64 = pIC64;
	return S_OK;
}

unsigned int CIA1::NextScanDelta()
{
	int desiredVideoPosition = dist_pal_frame(randengine);
	int currentVideoPosition = vic->vic_raster_line * PAL_CLOCKS_PER_LINE + vic->vic_raster_cycle - 1;	
	int delta;
	if (desiredVideoPosition > currentVideoPosition)
	{
		delta = desiredVideoPosition - currentVideoPosition;
	}
	else 
	{
		delta = desiredVideoPosition + PAL_CLOCKS_PER_FRAME - currentVideoPosition;
	}

	if (delta < KEYBOARDMINSCANINTERVAL)
	{
		delta += PAL_CLOCKS_PER_FRAME;
	}

	return delta;
}

void CIA1::ExecuteDevices(ICLK sysclock)
{
	if ((ICLKS)(sysclock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = sysclock + NextScanDelta();
		ReadKeyboard();
	}

	tape64->Tick(sysclock);
}

void CIA1::SetWakeUpClock()
{
	CIA::SetWakeUpClock();
	if ((ICLKS)(ClockNextWakeUpClock - nextKeyboardScanClock) > 0)
	{
		ClockNextWakeUpClock = nextKeyboardScanClock;
	}

	if ((ICLKS)(ClockNextWakeUpClock - tape64->nextTapeTickClock) >= 0)
	{
		ClockNextWakeUpClock = tape64->nextTapeTickClock;
	}
}

void CIA1::ResetKeyboard()
{
int i;

	joyport1 = 0xFF;
	joyport2 = 0xFF;
	potAx = 0xff;
	potAy = 0xff;
	potBx = 0xff;
	potBy = 0xff;
	for (i=0 ; i < 8 ; i++)
	{
		keyboard_matrix[i] = 0xFF;
		keyboard_rmatrix[i] = 0xFF;
	}

	ZeroMemory(&this->js, sizeof(this->js));
}

bit8 CIA1::ReadPortA()
{
	if ((ICLKS)(CurrentClock - nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + NextScanDelta();
		ReadKeyboard();
	}

	return ReadPortA_NoUpdateKeyboard();
}

/*
* Key matrix rules  revised again
* General Notes
* Keys that are held will connect a Port A pin to a Port B pin in 
* accordance with the key matrix layout.
* The ports always read the pin levels.
* Port B with keys held can sometimes force Port A to 1 even though  Port A 
* is outputting a 0 (3 or 2 Port B strong 1s needed).. 
* Port B with keys held can force Port A to 0 even though  Port A 
* is outputting a 1 with just one Port B zero.
* Port A with keys held can sometimes force Port B to 0 even though  Port B 
* is outputting a strong 1 (3 or 2 Port A zeros are needed).
* Port A with keys held can never force Port B to 1 when Port B is 
* outputting a 0.
* The joystick can always drag the ports down to 0.
* Port B is not symmetrical with Port A. It is as though Port B is stronger than Port A.
*/

bit8 CIA1::ReadPortA_NoUpdateKeyboard()
{
bit8 zB,data8,wB;
bit8 p;
bit8 zA,wA;	
	//Port A
	wA = PortAOutput_Strong1s() & joyport2;	
	zA = PortAOutput_Strong0s() & joyport2;

	//Port B
	wB = PortBOutput_Strong1s() & joyport1;
	zB = PortBOutput_Strong0s() & joyport1;
	data8 = PortAOutput_Strong0s();

	for (int j=7; j >= 0; j--)
	{
		for (int i=7; i >= 0; i--)
		{
			/*
			1) Port A feed back loop
			Port A can experience a zero-ing feed back across keys in a matrix column from 
			its own outputted 0s. The can happen between two held keys in matrix column 
			for which Port B is not outputting a strong 1. This feedback can zero out a 
			Port A strong 1.
			*/
			bit8 matrixColumn = keyboard_rmatrix[i];
			if (matrixColumn == 0xff)
			{
				continue;
			}

			bit8 bp = (1 << i);
			int strongPortA0 = prec_bitcount[(~zA & ~matrixColumn) & 0xff];
			int strongPortB1 = (wB & bp) != 0;
			if (strongPortA0 > 0 && strongPortA0 >= strongPortB1 * 3)
			{
				zB &= ~(bp);
				wB &= ~(bp);
			}
		}

		for (int i=7; i >= 0; i--)
		{
			bit8 matrixRow = keyboard_matrix[i];
			if (matrixRow == 0xff)
			{
				continue;
			}

			bit8 bp = (1 << i);
			/*
			* Port B can force a Port A pin to 0 if that Port A matrix row contains 
			* held keys of which the number of held keys relaying Port B 0s is equal to or 
			* greater than the number of held keys that are relaying Port B "strong" 1s. 
			* (not weak input mode 1s)
			*/
			int strongPortB0 = prec_bitcount[(~zB & ~matrixRow) & 0xff];
			int strongPortB1 = prec_bitcount[(wB & ~matrixRow) & 0xff];

			//if ((strongPortB0 > 0) && (strongPortB0 - strongPortB1 >= 0))// David lower resistance keys
			if ((strongPortB0 > 0) && (strongPortB0 - strongPortB1 > 0)) // Mariusz higher resistance keys stronger Port B 1
			{
				zA &= ~(bp);
				wA &= ~(bp);
			}

			/*
			* Port B can force a Port A pin to 1 if that Port A matrix row contains one 
			* or more held keys of which the number of keys relaying Port B strong 1s is 3 
			* greater than the number of held keys that are relaying Port B 0s.
			*/
			if (strongPortB1 - strongPortB0 >= 3)
			{
				zA |= (1 << i);
				wA |= (1 << i);
			}
		}
	}

	data8 = zA;
	p =  data8 & joyport2;
	return p;
}

bit8 CIA1::ReadPortB()
{
	if ((ICLKS)(CurrentClock - nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + NextScanDelta();
		ReadKeyboard();
	}

	return ReadPortB_NoUpdateKeyboard();
}

bit8 CIA1::ReadPortB_NoUpdateKeyboard()
{
bit8 zA,data8,wA;
bit8 p;
bit8 zB,wB;

	//Port B
	wB = PortBOutput_Strong1s() & joyport1;
	zB = PortBOutput_Strong0s() & joyport1;

	//Port A
	wA = PortAOutput_Strong1s() & joyport2;	
	zA = PortAOutput_Strong0s() & joyport2;
	data8 = PortBOutput_Strong0s();	

	//TEST
	for (int j=7; j >= 0; j--)
	{
		for (int i=7; i >= 0; i--)
		{
			/*
			* Port A feed back loop.
			* Port A can experience a zero-ing feed back across keys in a matrix column from 
			* its own outputted 0s. The can happen between two held keys in matrix column 
			* for which Port B is not outputting a strong 1. This feedback can zero out a 
			* Port A strong 1.
			*/
			bit8 matrixColumn = keyboard_rmatrix[i];
			if (matrixColumn == 0xff)
				continue;
			bit8 bp = (1 << i);
			int strongPortA0 = prec_bitcount[(~zA & ~matrixColumn) & 0xff];
			int strongPortB1 = (wB & bp) != 0;
			if (strongPortA0 > 0 && strongPortA0 >= strongPortB1 * 3)
			{
				zB &= ~(bp);
				wB &= ~(bp);
			}
		}

		for (int i=7; i >= 0; i--)
		{
			bit8 matrixRow = keyboard_matrix[i];
			if (matrixRow == 0xff)
				continue;
			bit8 bp = (1 << i);
			/*
			* Port B feed back loop.
			* Port B can experience a zero-ing feed back across keys in a matrix row from 
			* its own outputted 0s. This can happen in a matrix row where the number of 
			* held keys relaying Port B 0s is greater than the number of held keys that 
			* are relaying Port B "strong" 1s This feedback can not zero out a Port B 
			* strong 1.
			*/
			int strongPortB0 = prec_bitcount[(~zB & ~matrixRow) & 0xff];
			int strongPortB1 = prec_bitcount[(wB & ~matrixRow) & 0xff];

			//if ((strongPortB0 > 0) && (strongPortB0 - strongPortB1 >= 0)) // David lower resistance keys
			if ((strongPortB0 > 0) && (strongPortB0 - strongPortB1 > 0)) // Mariusz higher resistance keys stronger Port B 1
			{
				zA &= ~(bp);
				wA &= ~(bp);
			}
		}
	}

	data8 = zB;	
	p = data8 & joyport1;
	return p;
}


void CIA1::WritePortA(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new)
{
	if ((ICLKS)(CurrentClock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + NextScanDelta();
		ReadKeyboard();
	}
	else
	{
		LightPen();
	}
}

void CIA1::WritePortB(bool is_ddr, bit8 ddr_old, bit8 portdata_old, bit8 ddr_new, bit8 portdata_new)
{
	if ((ICLKS)(CurrentClock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + NextScanDelta();
		ReadKeyboard();
	}
	else
	{
		LightPen();
	}
}

void CIA1::InitReset(ICLK sysclock, bool poweronreset)
{
	CIA::InitReset(sysclock, poweronreset);
	nextKeyboardScanClock = sysclock;
	joyport1=0xff;
	joyport2=0xff;
	potAx=0xff;
	potAy=0xff;
	potBx=0xff;
	potBy=0xff;
	out4066PotX=0xff;
	out4066PotY=0xff;
}

void CIA1::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ResetKeyboard();
	CIA::Reset(sysclock, poweronreset);
	LightPen();
}


void CIA1::SetSystemInterrupt()
{
	cpu->Set_CIA_IRQ(CurrentClock);
}

void CIA1::ClearSystemInterrupt()
{
	cpu->Clear_CIA_IRQ();
}

void CIA1::LightPen()
{
bit8 lightpen;
bit8 enablePot;
	//TEST Nitro 16 does not like the light pen line to go low (active) by pressing space when port b is forcing the light pen line high.
	lightpen = (ReadPortB_NoUpdateKeyboard()) & 0x10;
	vic->SetLPLineClk(CurrentClock, lightpen!=0);

	bit8 oldPotX = this->out4066PotX;
	bit8 oldPotY = this->out4066PotY;
	bit8 newPotY = 0xff;
	bit8 newPotX = 0xff;
	enablePot = (ReadPortA_NoUpdateKeyboard()) & 0xc0;
	if (enablePot & 0x80)
	{	
		newPotX &= this->potBx;
		newPotY &= this->potBy;
	}

	if (enablePot & 0x40)
	{	
		newPotX &= this->potAx;
		newPotY &= this->potAy;
	}

	if (oldPotX != newPotX)
	{
		this->out4066PotX = newPotX;
		sid->Set_PotX(this->CurrentClock, newPotX);		
	}

	if (oldPotY != newPotY)
	{
		this->out4066PotY = newPotY;
		sid->Set_PotY(this->CurrentClock, newPotY);
	}
}

#define KEYDOWN(name,key) (name[c64KeyMap[key]] & 0x80) 
#define RAWKEYDOWN(name,key) (name[key] & 0x80) 
#define JOYMIN (-400)
#define JOYMAX (400)

void CIA1::SetKeyMatrixDown(bit8 row, bit8 col)
{
	row &= 0xf;
	col &= 0xf;
	KEYMATRIX_DOWN(row , col);
}

bool CIA1::ReadJoyAxis(int joyindex, struct joyconfig& joycfg, unsigned int& axis, bool& fire1, bool& fire2)
{
LPDIRECTINPUTDEVICE7 joystick7;
HRESULT  hr;
unsigned int i;
const DWORD povRightUp = 9000 - 2250;
const DWORD povRightDown = 9000 + 2250;
const DWORD povLeftUp = 27000 + 2250;
const DWORD povLeftDown = 27000 - 2250;
const DWORD povUpLeft = 36000 - 2250;
const DWORD povUpRight = 2250;
const DWORD povDownLeft = 18000 + 2250;
const DWORD povDownRight = 18000 - 2250;

	axis = 0;
	fire1 = false;
	fire2 = false;
	joystick7 = (LPDIRECTINPUTDEVICE7) dx->GetJoy(joyindex);
	joystick7->Poll();
	hr = joystick7->GetDeviceState(joycfg.sizeOfInputDeviceFormat, &this->js);
	if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
	{
		if ((this->CurrentClock - joycfg.joyNotAcquiredClock) > DEVICEACQUIRECLOCKS)
		{
			hr = joystick7->Acquire();
			if (SUCCEEDED(hr))
			{
				hr = joystick7->GetDeviceState(joycfg.sizeOfInputDeviceFormat, &this->js);
			}
			else
			{
				joycfg.joyNotAcquiredClock = this->CurrentClock;
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		for (i = 0; i < _countof(joycfg.povAvailable); i++)
		{
			if (joycfg.povAvailable[i] == 0)
			{
				break;
			}

			DWORD pov = *((LONG *)(((BYTE *)&this->js) + joycfg.povAvailable[i]));
			if (LOWORD(pov) != 0xFFFF)
			{
				if (pov < povRightUp || pov > povLeftUp)
				{
					axis |= JOYDIR_UP;
				}
				else if (pov > povRightDown && pov < povLeftDown)
				{
					axis |= JOYDIR_DOWN;
				}

				if (pov < povUpLeft && pov > povDownLeft)
				{
					axis |= JOYDIR_LEFT;
				}
				else if (pov > povUpRight && pov < povDownRight)
				{
					axis |= JOYDIR_RIGHT;
				}

				break;
			}
		}
		
		if (joycfg.isValidXAxis && joycfg.horizontalAxisCount > 0 && joycfg.joyObjectKindX == HCFG::JoyKindAxis)
		{
			LONG xpos = *((LONG *)(((BYTE *)&this->js) + joycfg.dwOfs_X));
			if (xpos < joycfg.xLeft)
			{
				if (joycfg.isXReverse)
				{
					axis |= JOYDIR_RIGHT;
					axis = axis & ~(JOYDIR_LEFT);
				}
				else
				{
					axis |= JOYDIR_LEFT;
					axis = axis & ~(JOYDIR_RIGHT);
				}
			}
			else if (xpos > joycfg.xRight)
			{
				if (joycfg.isXReverse)
				{
					axis |= JOYDIR_LEFT;
					axis = axis & ~(JOYDIR_RIGHT);
				}
				else
				{
					axis |= JOYDIR_RIGHT;
					axis = axis & ~(JOYDIR_LEFT);
				}
			}
		}

		if (joycfg.isValidYAxis && joycfg.verticalAxisCount > 0 && joycfg.joyObjectKindY == HCFG::JoyKindAxis)
		{
			LONG ypos = *((LONG *)(((BYTE *)&this->js) + joycfg.dwOfs_Y));
			if (ypos < joycfg.yUp)
			{
				if (joycfg.isYReverse)
				{
					axis |= JOYDIR_DOWN;
					axis = axis & ~(JOYDIR_UP);
				}
				else
				{
					axis |= JOYDIR_UP;
					axis = axis & ~(JOYDIR_DOWN);
				}
			}
			else if (ypos > joycfg.yDown)
			{
				if (joycfg.isYReverse)
				{
					axis |= JOYDIR_UP;
					axis = axis & ~(JOYDIR_DOWN);
				}
				else
				{
					axis |= JOYDIR_DOWN;
					axis = axis & ~(JOYDIR_UP);
				}
			}
		}

		for (i=0; i < joycfg.downButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.downButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_DOWN;
				axis = axis & ~(JOYDIR_UP);
				break;
			}
		}

		for (i=0; i < joycfg.upButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.upButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_UP;
				axis = axis & ~(JOYDIR_DOWN);
				break;
			}
		}

		for (i=0; i < joycfg.rightButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.rightButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_RIGHT;
				axis = axis & ~(JOYDIR_LEFT);
				break;
			}
		}

		for (i=0; i < joycfg.leftButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.leftButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_LEFT;
				axis = axis & ~(JOYDIR_RIGHT);
				break;
			}
		}

		for (i=0; i < joycfg.fire1ButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.fire1ButtonOffsets[i]] & 0x80)
			{
				fire1 = true;
				break;
			}
		}

		for (i=0; i < joycfg.fire2ButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.fire2ButtonOffsets[i]] & 0x80)
			{
				fire2 = true;
				break;
			}
		}

		return true;
	}

	return false;
}

bit8 CIA1::Get_PotAX()
{
	return this->potAx;
}

bit8 CIA1::Get_PotAY()
{
	return this->potAy;
}

bit8 CIA1::Get_PotBX()
{
	return this->potBx;
}

bit8 CIA1::Get_PotBY()
{
	return this->potBy;
}

void CIA1::ReadKeyboard()
{
char     buffer[256]; 
HRESULT  hr;
static int softcursorleftcount = 0;
static int softcursorupcount = 0;
bool joy1ok;
bool joy2ok;
bool keyboardok;
bit8 localjoyport1;
bit8 localjoyport2;
//3-2-1-0 bit postion in joynaxis
//U-D-L-R //U is up, D is down L is left, R is right
unsigned int joy1axis = 0;
unsigned int joy2axis = 0;
bool joy1fire1 = 0;
bool joy2fire1 = 0;
bool joy1fire2 = 0;
bool joy2fire2 = 0;
bit8 localpotAx = 0xff;
bit8 localpotAy = 0xff;
bit8 localpotBx = 0xff;
bit8 localpotBy = 0xff;
	
	localjoyport2=0xff;
	localjoyport1=0xff;
	joy1ok = dx->joyok[JOY1];
	joy2ok = dx->joyok[JOY2];
	hr = DIERR_INPUTLOST;
	if (appStatus->m_bAutoload)
	{
		joyport1 = 0xff;
		joyport2 = 0xff;
		LightPen();
		return;
	}

	ResetKeyboard();
	if (G::IsHideWindow)
	{
		return;
	}
	
	hr = dx->pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
	if SUCCEEDED(hr) 
	{
		keyboardok = true;
	}
	else
	{ 
		keyboardok = false;
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			if ((this->CurrentClock - keyboardNotAcquiredClock) > DEVICEACQUIRECLOCKS)
			{
				hr = dx->pKeyboard->Acquire();
				if (SUCCEEDED(hr))
				{
					hr = dx->pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
					if SUCCEEDED(hr)
					{
						keyboardok = true;
					}
				}
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		hr=S_OK;
	}

	if (joy1ok)
	{
		joy1ok = ReadJoyAxis(JOY1, appStatus->m_joy1config, joy1axis, joy1fire1, joy1fire2);
	}

	if (joy2ok)
	{
		joy2ok = ReadJoyAxis(JOY2, appStatus->m_joy2config, joy2axis, joy2fire1, joy2fire2);
	}

	if (joy1ok)
	{
		if (joy1axis & JOYDIR_LEFT)
		{
			localjoyport1 &= (bit8) ~4;
		}
		else if (joy1axis & JOYDIR_RIGHT)
		{
			localjoyport1 &= (bit8) ~8;
		}

		if (joy1axis & JOYDIR_UP)
		{
			localjoyport1 &= (bit8) ~1;
		}
		else if (joy1axis & JOYDIR_DOWN)
		{
			localjoyport1 &= (bit8) ~2;
		}

		if (joy1fire1)
		{
			localjoyport1 &= (bit8) ~16;
		}

		if (joy1fire2)
		{
			localpotAx = 0x0;
		}
	}

	if (joy2ok)
	{
		if (joy2axis & JOYDIR_LEFT)
		{
			localjoyport2 &= (bit8) ~4;
		}
		else if (joy2axis & JOYDIR_RIGHT)
		{
			localjoyport2 &= (bit8) ~8;
		}

		if (joy2axis & JOYDIR_UP)
		{
			localjoyport2 &= (bit8) ~1;
		}
		else if (joy2axis & JOYDIR_DOWN)
		{
			localjoyport2 &= (bit8) ~2;
		}

		if (joy2fire1)
		{
			localjoyport2 &= (bit8) ~16;
		}

		if (joy2fire2)
		{
			localpotBx = 0x0;
		}
	}

	if (keyboardok)
	{
		if (RAWKEYDOWN(buffer, DIK_LALT) || GetAsyncKeyState(VK_MENU) < 0 || m_bAltLatch)	 
		{
			//Make sure all emulated c64 keys are released before recognising an emulated c64 key.
			m_bAltLatch = true;
			int k;
			for (k = 0; k < C64K_MAX && k < _countof(buffer); k++)
			{
				if ((buffer[c64KeyMap[k]] & 0x80) != 0)
				{
					break;
				}
			}

			if (k >= _countof(buffer) || k >= C64K_MAX)
			{
				m_bAltLatch = false;
			}
		}
		else
		{
			if (KEYDOWN(buffer, C64K_COLON))
			{
				KEYMATRIX_DOWN(5, 5);
			}

			if (KEYDOWN(buffer, C64K_STOP))
			{
				KEYMATRIX_DOWN(7, 7);
			}

			if (KEYDOWN(buffer, C64K_1))  
			{
				KEYMATRIX_DOWN(7, 0);
			}

			if (KEYDOWN(buffer, C64K_2))  
			{
				KEYMATRIX_DOWN(7, 3);
			}
			
			if (KEYDOWN(buffer, C64K_3))  
			{
				KEYMATRIX_DOWN(1, 0);
			}
			
			if (KEYDOWN(buffer, C64K_4))  
			{
				KEYMATRIX_DOWN(1, 3);
			}
			
			if (KEYDOWN(buffer, C64K_5))  
			{
				KEYMATRIX_DOWN(2, 0);
			}
			
			if (KEYDOWN(buffer, C64K_6))  
			{
				KEYMATRIX_DOWN(2, 3);
			}
			
			if (KEYDOWN(buffer, C64K_7))  
			{
				KEYMATRIX_DOWN(3, 0);
			}
			
			if (KEYDOWN(buffer, C64K_8))  
			{
				KEYMATRIX_DOWN(3, 3);
			}
			
			if (KEYDOWN(buffer, C64K_9))  
			{
				KEYMATRIX_DOWN(4, 0);
			}
			
			if (KEYDOWN(buffer, C64K_0))  
			{
				KEYMATRIX_DOWN(4, 3);
			}
			
			if (KEYDOWN(buffer, C64K_MINUS))  
			{
				KEYMATRIX_DOWN(5, 3);
			}
			
			if (KEYDOWN(buffer, C64K_PLUS))  
			{
				KEYMATRIX_DOWN(5, 0);
			}

			if (KEYDOWN(buffer, C64K_EQUAL))  
			{
				KEYMATRIX_DOWN(6, 5);
			}
			
			if (KEYDOWN(buffer, C64K_POUND))  
			{
				KEYMATRIX_DOWN(6, 0);
			}

			if (KEYDOWN(buffer, C64K_DEL))
			{
				KEYMATRIX_DOWN(0, 0);
			}
			
			if (KEYDOWN(buffer, C64K_STOP))   
			{
				KEYMATRIX_DOWN(7, 7);
			}
			
			if (KEYDOWN(buffer, C64K_Q))   
			{
				KEYMATRIX_DOWN(7, 6);
			}
			
			if (KEYDOWN(buffer, C64K_W))   
			{
				KEYMATRIX_DOWN(1, 1);
			}
			
			if (KEYDOWN(buffer, C64K_E))   
			{
				KEYMATRIX_DOWN(1, 6);
			}
			
			if (KEYDOWN(buffer, C64K_R))   
			{
				KEYMATRIX_DOWN(2, 1);
			}
			
			if (KEYDOWN(buffer, C64K_T))   
			{
				KEYMATRIX_DOWN(2, 6);
			}
			
			if (KEYDOWN(buffer, C64K_Y))   
			{
				KEYMATRIX_DOWN(3, 1);
			}
			
			if (KEYDOWN(buffer, C64K_U))   
			{
				KEYMATRIX_DOWN(3, 6);
			}
			
			if (KEYDOWN(buffer, C64K_I))   
			{
				KEYMATRIX_DOWN(4, 1);
			}
			
			if (KEYDOWN(buffer, C64K_O))   
			{
				KEYMATRIX_DOWN(4, 6);
			}
			
			if (KEYDOWN(buffer, C64K_P))   
			{
				KEYMATRIX_DOWN(5, 1);
			}
			
			if (KEYDOWN(buffer, C64K_ASTERISK))
			{
				KEYMATRIX_DOWN(6, 1);
			}
			
			if (KEYDOWN(buffer, C64K_ARROWUP))
			{
				KEYMATRIX_DOWN(6, 6);
			}
			
			if (KEYDOWN(buffer, C64K_RETURN))
			{
				KEYMATRIX_DOWN(0, 1);
			}
			
			if (KEYDOWN(buffer, C64K_CONTROL))
			{
				KEYMATRIX_DOWN(7, 2);
			}
			
			if (KEYDOWN(buffer, C64K_A))   
			{
				KEYMATRIX_DOWN(1, 2);
			}
			
			if (KEYDOWN(buffer, C64K_S))   
			{
				KEYMATRIX_DOWN(1, 5);
			}
			
			if (KEYDOWN(buffer, C64K_D))   
			{
				KEYMATRIX_DOWN(2, 2);
			}
			
			if (KEYDOWN(buffer, C64K_F))   
			{
				KEYMATRIX_DOWN(2, 5);
			}
			
			if (KEYDOWN(buffer, C64K_G))   
			{
				KEYMATRIX_DOWN(3, 2);
			}
			
			if (KEYDOWN(buffer, C64K_H))   
			{
				KEYMATRIX_DOWN(3, 5);
			}
			
			if (KEYDOWN(buffer, C64K_J))   
			{
				KEYMATRIX_DOWN(4, 2);
			}
			
			if (KEYDOWN(buffer, C64K_K))   
			{
				KEYMATRIX_DOWN(4, 5);
			}
			
			if (KEYDOWN(buffer, C64K_L))   
			{
				KEYMATRIX_DOWN(5, 2);
			}
			
			if (KEYDOWN(buffer, C64K_SEMICOLON))
			{
				KEYMATRIX_DOWN(6, 2);
			}
			
			if (KEYDOWN(buffer, C64K_AT))   
			{
				KEYMATRIX_DOWN(5, 6);
			}
			
			if (KEYDOWN(buffer, C64K_ARROWLEFT))
			{
				KEYMATRIX_DOWN(7, 1);
			}
			
			if (KEYDOWN(buffer, C64K_LEFTSHIFT))
			{
				KEYMATRIX_DOWN(1, 7);
			}
			
			if (KEYDOWN(buffer, C64K_COMMODORE))
			{
				KEYMATRIX_DOWN(7, 5);
			}

			if (KEYDOWN(buffer, C64K_Z))   
			{
				KEYMATRIX_DOWN(1, 4);
			}
			
			if (KEYDOWN(buffer, C64K_X))   
			{
				KEYMATRIX_DOWN(2, 7);
			}
			
			if (KEYDOWN(buffer, C64K_C))   
			{
				KEYMATRIX_DOWN(2, 4);
			}
			
			if (KEYDOWN(buffer, C64K_V))   
			{
				KEYMATRIX_DOWN(3, 7);
			}
			
			if (KEYDOWN(buffer, C64K_B))   
			{
				KEYMATRIX_DOWN(3, 4);
			}
			
			if (KEYDOWN(buffer, C64K_N))   
			{
				KEYMATRIX_DOWN(4, 7);
			}
			
			if (KEYDOWN(buffer, C64K_M))   
			{
				KEYMATRIX_DOWN(4, 4);
			}
			
			if (KEYDOWN(buffer, C64K_COMMA))   
			{
				KEYMATRIX_DOWN(5, 7);
			}
			
			if (KEYDOWN(buffer, C64K_DOT))   
			{
				KEYMATRIX_DOWN(5, 4);
			}
			
			if (KEYDOWN(buffer, C64K_SLASH))
			{
				KEYMATRIX_DOWN(6, 7);
			}
			
			if (KEYDOWN(buffer, C64K_RIGHTSHIFT)) 
			{
				KEYMATRIX_DOWN(6, 4);
			}
					
			if (KEYDOWN(buffer, C64K_SPACE))
			{
				KEYMATRIX_DOWN(7, 4);
			}
			
			if (KEYDOWN(buffer, C64K_F1))   
			{
				KEYMATRIX_DOWN(0, 4);
			}
			
			if (KEYDOWN(buffer, C64K_F3))   
			{
				KEYMATRIX_DOWN(0, 5);
			}
			
			if (KEYDOWN(buffer, C64K_F5))   
			{
				KEYMATRIX_DOWN(0, 6);
			}
			
			if (KEYDOWN(buffer, C64K_F7))   
			{
				KEYMATRIX_DOWN(0, 3);
			}
											
			if (RAWKEYDOWN(buffer, DIK_F12))   
			{
				if (F12_was_up)
				{
					pIC64->PostHardReset(true);
				}

				F12_was_up=false;
			}
			else
			{
				F12_was_up=true;
			}

			if (RAWKEYDOWN(buffer, DIK_F11))
			{
				if (F11_was_up)
				{
					pIC64->PostSoftReset(true);
				}

				F11_was_up=false;
			}
			else
			{
				F11_was_up=true;
			}

			if (KEYDOWN(buffer, C64K_HOME))  
			{
				KEYMATRIX_DOWN(6, 3);
			}

			if (KEYDOWN(buffer, C64K_RESTORE))
			{
				if (restore_was_up)
				{
					bit8 currentNMI = cpu->NMI;
					if (cpu->NMI == 0)
					{
						cpu->SetNMI(CurrentClock);
					}

					cpu->NMI = currentNMI;
				}

				restore_was_up=false;
			}
			else
			{
				restore_was_up=true;
			}

			if (KEYDOWN(buffer, C64K_CURSORRIGHT))
			{
				KEYMATRIX_DOWN(0, 2);
			}

			if (KEYDOWN(buffer, C64K_CURSORDOWN))
			{
				KEYMATRIX_DOWN(0, 7);
			}

			if (KEYDOWN(buffer, C64K_CURSORUP))
			{
				KEYMATRIX_DOWN(6, 4);
				if (softcursorupcount > 0)
				{
					KEYMATRIX_DOWN(0, 7);
				}
				else 
				{
					softcursorupcount++;
				}
			}
			else
			{
				if (softcursorupcount > 0)
				{
					softcursorupcount--;
					KEYMATRIX_DOWN(6, 4);
				}
			}

			if (KEYDOWN(buffer, C64K_CURSORLEFT))
			{
				KEYMATRIX_DOWN(6, 4);
				if (softcursorleftcount > 0)
				{
					KEYMATRIX_DOWN(0, 2);
				}
				else
				{
					softcursorleftcount++;
				}
			}
			else
			{
				if (softcursorleftcount > 0)
				{
					softcursorleftcount--;
					KEYMATRIX_DOWN(6, 4);
				}
			}
		}

		if (appStatus->m_bAllowOpposingJoystick)
		{
			if (KEYDOWN(buffer, C64K_JOY1UP))
			{
				localjoyport1 &= (bit8) ~1;
			}

			if (KEYDOWN(buffer, C64K_JOY1DOWN))   
			{
				localjoyport1 &= (bit8) ~2;
			}

			if (KEYDOWN(buffer, C64K_JOY1LEFT))   
			{
				localjoyport1 &= (bit8) ~4;
			}

			if (KEYDOWN(buffer, C64K_JOY1RIGHT))
			{
				localjoyport1 &= (bit8) ~8;
			}

			if (KEYDOWN(buffer, C64K_JOY2UP))
			{
				localjoyport2 &= (bit8) ~1;
			}

			if (KEYDOWN(buffer, C64K_JOY2DOWN))
			{
				localjoyport2 &= (bit8) ~2;
			}

			if (KEYDOWN(buffer, C64K_JOY2LEFT))
			{
				localjoyport2 &= (bit8) ~4;
			}

			if (KEYDOWN(buffer, C64K_JOY2RIGHT))
			{
				localjoyport2 &= (bit8) ~8;
			}
		}
		else
		{
			if (KEYDOWN(buffer, C64K_JOY1UP))
			{
				localjoyport1 &= (bit8) ~1;
			}
			else if (KEYDOWN(buffer, C64K_JOY1DOWN))
			{
				localjoyport1 &= (bit8) ~2;
			}

			if (KEYDOWN(buffer, C64K_JOY1LEFT))
			{
				localjoyport1 &= (bit8) ~4;
			}
			else if (KEYDOWN(buffer, C64K_JOY1RIGHT))
			{
				localjoyport1 &= (bit8) ~8;
			}

			if (KEYDOWN(buffer, C64K_JOY2UP))
			{
				localjoyport2 &= (bit8) ~1;
			}
			else if (KEYDOWN(buffer, C64K_JOY2DOWN))
			{
				localjoyport2 &= (bit8) ~2;
			}

			if (KEYDOWN(buffer, C64K_JOY2LEFT))
			{
				localjoyport2 &= (bit8) ~4;
			}
			else if (KEYDOWN(buffer, C64K_JOY2RIGHT))
			{
				localjoyport2 &= (bit8) ~8;
			}
		}

		if (KEYDOWN(buffer, C64K_JOY2FIRE))   
		{
			localjoyport2 &= (bit8) ~16;
		}

		if (KEYDOWN(buffer, C64K_JOY1FIRE))   
		{
			localjoyport1 &= (bit8) ~16;
		}
	}

	if (appStatus->m_bSwapJoysticks)
	{
		joyport1 = localjoyport2;
		joyport2 = localjoyport1;
		potBx  = localpotAx;
		potAx  = localpotBx;
	}
	else
	{
		joyport1 = localjoyport1;
		joyport2 = localjoyport2;
		potBx  = localpotBx;
		potAx  = localpotAx;
	}

	LightPen();
}

void CIA1::GetState(SsCia1V2 &state)
{
	ZeroMemory(&state, sizeof(state));
	CIA::GetState(state.cia);
	state.nextKeyboardScanClock = nextKeyboardScanClock;
}

void CIA1::SetState(const SsCia1V2 &state)
{
	CIA::SetState(state.cia);
	nextKeyboardScanClock = state.nextKeyboardScanClock;
}

void CIA1::UpgradeStateV0ToV1(const SsCia1V0 &in, SsCia1V1 &out)
{
	ZeroMemory(&out, sizeof(SsCia2V1));
	CIA::UpgradeStateV0ToV1(in.cia, out.cia);
	out.nextKeyboardScanClock = in.nextKeyboardScanClock;
}

void CIA1::UpgradeStateV1ToV2(const SsCia1V1 &in, SsCia1V2 &out)
{
	ZeroMemory(&out, sizeof(SsCia2V2));
	CIA::UpgradeStateV1ToV2(in.cia, out.cia);
	out.nextKeyboardScanClock = in.nextKeyboardScanClock;
}
