#include "register.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "tap.h"
#include "c64keys.h"
#include "cia1.h"
#include "ErrorLogger.h"

#define KEYBOARDMINSCANINTERVAL (PAL_CLOCKS_PER_FRAME / 2)
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

HRESULT CIA1::Init(CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, IMonitorVic* vic, ISid *sid, Tape64 *tape64, CDX9 *dx, IAutoLoad* pAutoLoad)
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
	this->pIAutoLoad = pAutoLoad;
	this->SetMode(appStatus->m_CIAMode, appStatus->m_bTimerBbug);
	this->UpdateKeyMap();
	return S_OK;
}

void CIA1::UpdateKeyMap()
{
	if (appStatus != nullptr)
	{
		unsigned int i;
		for (i = 0; i < _countof(c64KeyMap); i++)
		{
			if (i < _countof(appStatus->m_KeyMap))
			{
				this->c64KeyMap[i] = appStatus->m_KeyMap[i];
			}
			else
			{
				this->c64KeyMap[i] = 0;
			}
		}
	}
}

ICLK CIA1::NextScanDelta()
{
	ICLK desiredVideoPosition = (ICLK)dist_pal_frame(randengine);
	ICLK currentVideoPosition = vic->GetCurrentRasterLine() * PAL_CLOCKS_PER_LINE + vic->GetCurrentRasterCycle() - 1;
	ICLK vicdiff = vic->CurrentClock - this->CurrentClock;
	currentVideoPosition = (currentVideoPosition + PAL_CLOCKS_PER_FRAME - vicdiff)  % PAL_CLOCKS_PER_FRAME;

	int delta;
	if ((ICLKS)(desiredVideoPosition - currentVideoPosition) > 0)
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
	if ((ICLKS)(sysclock - this->ClockNextWakeUpClock) > 0)
	{
		this->ExecuteCycle(this->ClockNextWakeUpClock);
	}
	else
	{ 
		this->ExecuteCycle(sysclock);
	}

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
	CIA::InitCommonReset(sysclock, poweronreset);
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
	row &= 0x7;
	col &= 0x7;
	KEYMATRIX_DOWN(row , col);
}

bool CIA1::ReadJoyAxis(int joyindex, struct joyconfig& joycfg, unsigned int& axis, bool& fire1, bool& fire2, unsigned char c64keyboard[])
{
LPDIRECTINPUTDEVICE7 joystick7;
HRESULT  hr;
unsigned int i;
unsigned int j;

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

			DWORD povvalue = *((DWORD *)(((BYTE *)&this->js) + joycfg.povAvailable[i]));
			if (LOWORD(povvalue) != 0xFFFF)
			{
				if (povvalue < ButtonItemData::POVRightUp || povvalue >= ButtonItemData::POVLeftUp)
				{
					axis |= JOYDIR_UP;
				}
				else if (povvalue >= ButtonItemData::POVRightDown && povvalue < ButtonItemData::POVLeftDown)
				{
					axis |= JOYDIR_DOWN;
				}

				if (povvalue < ButtonItemData::POVUpLeft && povvalue >= ButtonItemData::POVDownLeft)
				{
					axis |= JOYDIR_LEFT;
				}
				else if (povvalue >= ButtonItemData::POVUpRight && povvalue < ButtonItemData::POVDownRight)
				{
					axis |= JOYDIR_RIGHT;
				}

				break;
			}
		}
		
		if (joycfg.isValidXAxis && joycfg.horizontalAxisAxisCount > 0 && joycfg.joyObjectKindX == HCFG::JoyKindAxis)
		{
			LONG xpos = *((LONG *)(((BYTE *)&this->js) + joycfg.dwOfs_X));
			if (xpos < joycfg.xLeft)
			{
				if (joycfg.isXReverse)
				{
					axis |= JOYDIR_RIGHT;
				}
				else
				{
					axis |= JOYDIR_LEFT;
				}
			}
			else if (xpos > joycfg.xRight)
			{
				if (joycfg.isXReverse)
				{
					axis |= JOYDIR_LEFT;
				}
				else
				{
					axis |= JOYDIR_RIGHT;
				}
			}
		}

		if (joycfg.isValidYAxis && joycfg.verticalAxisAxisCount > 0 && joycfg.joyObjectKindY == HCFG::JoyKindAxis)
		{
			LONG ypos = *((LONG *)(((BYTE *)&this->js) + joycfg.dwOfs_Y));
			if (ypos < joycfg.yUp)
			{
				if (joycfg.isYReverse)
				{
					axis |= JOYDIR_DOWN;
				}
				else
				{
					axis |= JOYDIR_UP;
				}
			}
			else if (ypos > joycfg.yDown)
			{
				if (joycfg.isYReverse)
				{
					axis |= JOYDIR_UP;
				}
				else
				{
					axis |= JOYDIR_DOWN;
				}
			}
		}

		for (i=0; i < joycfg.downButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.downButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_DOWN;
				break;
			}
		}

		for (i=0; i < joycfg.upButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.upButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_UP;
				break;
			}
		}

		for (i=0; i < joycfg.rightButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.rightButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_RIGHT;
				break;
			}
		}

		for (i=0; i < joycfg.leftButtonCount; i++)
		{
			if (((BYTE *)&this->js)[joycfg.leftButtonOffsets[i]] & 0x80)
			{
				axis |= JOYDIR_LEFT;
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

		if (joycfg.enableKeyAssign)
		{
			for (j=0; j < joycfg.MAXKEYMAPS; j++)
			{
				for (i=0; i < joycfg.keyNButtonCount[j]; i++)
				{
					if (((BYTE *)&this->js)[joycfg.keyNButtonOffsets[j][i]] & 0x80)
					{
						c64keyboard[joycfg.keyNoAssign[j]] = 0x80;
						break;
					}
				}

				for (i=0; i < joycfg.keyNAxisCount[j]; i++)
				{
					LONG axisvalue = *((LONG *)(((BYTE *)&this->js) + joycfg.keyNAxisOffsets[j][i]));
					bool axistriggered = false;
					switch (joycfg.keyNAxisDirection[j][i])
					{
						case GameControllerItem::DirectionMax:
							if (axisvalue > joycfg.axes[j][i].maxAxisTrigger)
							{
								axistriggered = true;
							}

							break;
						case GameControllerItem::DirectionMin:
							if (axisvalue < joycfg.axes[j][i].minAxisTrigger)
							{
								axistriggered = true;
							}

							break;
					}

					if (axistriggered)
					{
						c64keyboard[joycfg.keyNoAssign[j]] = 0x80;
						break;
					}
				}

				for (i=0; i < joycfg.keyNPovCount[j]; i++)
				{
					LONG povvalue = *((DWORD *)(((BYTE *)&this->js) + joycfg.keyNPovOffsets[j][i]));
					if (LOWORD(povvalue) != 0xFFFF)
					{
						bool povtriggered = false;
						switch (joycfg.keyNPovDirection[j][i])
						{
						case GameControllerItem::DirectionUp:	
							if (povvalue < ButtonItemData::POVRightUp || povvalue >= ButtonItemData::POVLeftUp)
							{
								povtriggered = true;
							}

							break;
						case GameControllerItem::DirectionDown:
							if (povvalue >= ButtonItemData::POVRightDown && povvalue < ButtonItemData::POVLeftDown)
							{
								povtriggered = true;
							}

							break;
						case GameControllerItem::DirectionLeft:
							if (povvalue < ButtonItemData::POVUpLeft && povvalue >= ButtonItemData::POVDownLeft)
							{
								povtriggered = true;
							}

							break;
						case GameControllerItem::DirectionRight:
							if (povvalue >= ButtonItemData::POVUpRight && povvalue < ButtonItemData::POVDownRight)
							{
								povtriggered = true;
							}

							break;
						}

						if (povtriggered)
						{
							c64keyboard[joycfg.keyNoAssign[j]] = 0x80;
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

void CIA1::EnableInput(bool enabled)
{
	this->enableInput = enabled;
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

void CIA1::WriteDebuggerReadKeyboard()
{
#ifdef DEBUG
	TCHAR sDebug[50];
	_stprintf_s(sDebug, _countof(sDebug), TEXT("%08X %08X %08X %d %03X %02X"), vic->GetFrameCounter(), vic->CurrentClock, this->CurrentClock, (int)(ICLKS)(vic->CurrentClock - this->CurrentClock), vic->GetCurrentRasterLine(), vic->GetCurrentRasterCycle());
	OutputDebugString(sDebug);
	OutputDebugString(TEXT("\n"));
#endif
}

void CIA1::ReadKeyboard()
{
	unsigned char buffer[256];
	unsigned char c64keyboard[C64Keys::C64K_COUNTOFKEYS];
	HRESULT  hr;
	static int softcursorleftcount = 0;
	static int softcursorupcount = 0;
	static int softf2count = 0;
	static int softf4count = 0;
	static int softf6count = 0;
	static int softf8count = 0;
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
	unsigned int i;

	localjoyport2=0xff;
	localjoyport1=0xff;
	joy1ok = dx->joyok[JOY1];
	joy2ok = dx->joyok[JOY2];
	hr = DIERR_INPUTLOST;
	if (appStatus->m_bAutoload)
	{
		pIAutoLoad->AutoLoadHandler(CurrentClock);
		joyport1 = 0xff;
		joyport2 = 0xff;
		LightPen();
		return;
	}

	if (!this->enableInput)
	{
		return;
	}

	ResetKeyboard();
	if (ErrorLogger::HideWindow)
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

	if (keyboardok)
	{
		for (i = 0; i < _countof(c64keyboard); i++)
		{							
			c64keyboard[i] = buffer[c64KeyMap[i]];
		}
	}
	else
	{
		ZeroMemory(&c64keyboard[0], sizeof(c64keyboard));
	}

	if (joy1ok)
	{
		joy1ok = ReadJoyAxis(JOY1, appStatus->m_joy1config, joy1axis, joy1fire1, joy1fire2, c64keyboard);
	}

	if (joy2ok)
	{
		joy2ok = ReadJoyAxis(JOY2, appStatus->m_joy2config, joy2axis, joy2fire1, joy2fire2, c64keyboard);
	}

	if (joy1ok)
	{
		if (joy1axis & JOYDIR_LEFT)
		{
			c64keyboard[C64Keys::C64K_JOY1LEFT] = 0x80;
		}
		
		if (joy1axis & JOYDIR_RIGHT)
		{
			c64keyboard[C64Keys::C64K_JOY1RIGHT] = 0x80;
		}

		if (joy1axis & JOYDIR_UP)
		{
			c64keyboard[C64Keys::C64K_JOY1UP] = 0x80;
		}
		
		if (joy1axis & JOYDIR_DOWN)
		{
			c64keyboard[C64Keys::C64K_JOY1DOWN] = 0x80;
		}

		if (joy1fire1)
		{
			c64keyboard[C64Keys::C64K_JOY1FIRE] = 0x80;
		}

		if (joy1fire2)
		{
			c64keyboard[C64Keys::C64K_JOY1FIRE2] = 0x80;
		}
	}

	if (joy2ok)
	{
		if (joy2axis & JOYDIR_LEFT)
		{
			c64keyboard[C64Keys::C64K_JOY2LEFT] = 0x80;
		}
		
		if (joy2axis & JOYDIR_RIGHT)
		{
			c64keyboard[C64Keys::C64K_JOY2RIGHT] = 0x80;
		}

		if (joy2axis & JOYDIR_UP)
		{
			c64keyboard[C64Keys::C64K_JOY2UP] = 0x80;
		}
		
		if (joy2axis & JOYDIR_DOWN)
		{
			c64keyboard[C64Keys::C64K_JOY2DOWN] = 0x80;
		}

		if (joy2fire1)
		{
			c64keyboard[C64Keys::C64K_JOY2FIRE] = 0x80;
		}

		if (joy2fire2)
		{
			c64keyboard[C64Keys::C64K_JOY2FIRE2] = 0x80;
		}
	}

	if (keyboardok)
	{		
		if (RAWKEYDOWN(buffer, DIK_LALT) || GetAsyncKeyState(VK_MENU) < 0 || m_bAltLatch)
		{
			//Make sure all emulated c64 keys are released before recognising an emulated c64 key.
			m_bAltLatch = true;
			unsigned int k;
			for (k = 0; k < C64Keys::C64K_COUNTOFKEYS && k < _countof(buffer); k++)
			{
				if ((buffer[c64KeyMap[k]] & 0x80) != 0)
				{
					break;
				}
			}

			if (k >= _countof(buffer) || k >= C64Keys::C64K_COUNTOFKEYS)
			{
				m_bAltLatch = false;
			}
		}
		else
		{
			for (i = 0; i < _countof(c64keyboard); i++)
			{							
				if (c64keyboard[i] != 0)
				{
					C64Keys::KeyRC& keylogic = C64Keys::KeyRowCol[i];
					unsigned row = keylogic.row;
					unsigned col = keylogic.col;
					keyboard_matrix[row] = 	keyboard_matrix[row] & keylogic.rowmask;
					keyboard_rmatrix[col] =	keyboard_rmatrix[col] & keylogic.colmask;
				}
			}

			if (c64keyboard[C64Keys::C64K_F2] != 0)
			{
				KEYMATRIX_DOWN(6, 4);
				if (softf2count > 0)
				{
					KEYMATRIX_DOWN(0, 4);
				}
				else 
				{
					softf2count++;
				}
			}
			else
			{
				if (softf2count > 0)
				{
					softf2count--;
					KEYMATRIX_DOWN(6, 4);
				}
			}
			
			if (c64keyboard[C64Keys::C64K_F4] != 0)
			{
				KEYMATRIX_DOWN(6, 4);
				if (softf4count > 0)
				{
					KEYMATRIX_DOWN(0, 5);
				}
				else 
				{
					softf4count++;
				}
			}
			else
			{
				if (softf4count > 0)
				{
					softf4count--;
					KEYMATRIX_DOWN(6, 4);
				}
			}
			
			if (c64keyboard[C64Keys::C64K_F6] != 0)
			{
				KEYMATRIX_DOWN(6, 4);
				if (softf6count > 0)
				{
					KEYMATRIX_DOWN(0, 6);
				}
				else 
				{
					softf6count++;
				}
			}
			else
			{
				if (softf6count > 0)
				{
					softf6count--;
					KEYMATRIX_DOWN(6, 4);
				}
			}
			
			if (c64keyboard[C64Keys::C64K_F8] != 0)
			{
				KEYMATRIX_DOWN(6, 4);
				if (softf8count > 0)
				{
					KEYMATRIX_DOWN(0, 3);
				}
				else 
				{
					softf8count++;
				}
			}
			else
			{
				if (softf8count > 0)
				{
					softf8count--;
					KEYMATRIX_DOWN(6, 4);
				}
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

			if (c64keyboard[C64Keys::C64K_RESTORE] != 0)
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

			if (c64keyboard[C64Keys::C64K_CURSORUP] != 0)
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

			if (c64keyboard[C64Keys::C64K_CURSORLEFT] != 0)
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
			if (c64keyboard[C64Keys::C64K_JOY1UP] != 0)
			{
				localjoyport1 &= (bit8) ~1;
			}

			if (c64keyboard[C64Keys::C64K_JOY1DOWN] != 0)
			{
				localjoyport1 &= (bit8) ~2;
			}

			if (c64keyboard[C64Keys::C64K_JOY1LEFT] != 0)
			{
				localjoyport1 &= (bit8) ~4;
			}

			if (c64keyboard[C64Keys::C64K_JOY1RIGHT] != 0)
			{
				localjoyport1 &= (bit8) ~8;
			}

			if (c64keyboard[C64Keys::C64K_JOY2UP] != 0)
			{
				localjoyport2 &= (bit8) ~1;
			}

			if (c64keyboard[C64Keys::C64K_JOY2DOWN] != 0)
			{
				localjoyport2 &= (bit8) ~2;
			}

			if (c64keyboard[C64Keys::C64K_JOY2LEFT] != 0)
			{
				localjoyport2 &= (bit8) ~4;
			}

			if (c64keyboard[C64Keys::C64K_JOY2RIGHT] != 0)
			{
				localjoyport2 &= (bit8) ~8;
			}
		}
		else
		{
			if (c64keyboard[C64Keys::C64K_JOY1UP] != 0)
			{
				localjoyport1 &= (bit8) ~1;
			}
			else if (c64keyboard[C64Keys::C64K_JOY1DOWN] != 0)
			{
				localjoyport1 &= (bit8) ~2;
			}

			if (c64keyboard[C64Keys::C64K_JOY1LEFT] != 0)
			{
				localjoyport1 &= (bit8) ~4;
			}
			else if (c64keyboard[C64Keys::C64K_JOY1RIGHT] != 0)
			{
				localjoyport1 &= (bit8) ~8;
			}

			if (c64keyboard[C64Keys::C64K_JOY2UP] != 0)
			{
				localjoyport2 &= (bit8) ~1;
			}
			else if (c64keyboard[C64Keys::C64K_JOY2DOWN] != 0)
			{
				localjoyport2 &= (bit8) ~2;
			}

			if (c64keyboard[C64Keys::C64K_JOY2LEFT] != 0)
			{
				localjoyport2 &= (bit8) ~4;
			}
			else if (c64keyboard[C64Keys::C64K_JOY2RIGHT] != 0)
			{
				localjoyport2 &= (bit8) ~8;
			}
		}

		if (c64keyboard[C64Keys::C64K_JOY1FIRE] != 0)
		{
			localjoyport1 &= (bit8) ~16;
		}

		if (c64keyboard[C64Keys::C64K_JOY1FIRE2] != 0)
		{
			localpotAx = 0x0;
		}

		if (c64keyboard[C64Keys::C64K_JOY2FIRE] != 0)
		{
			localjoyport2 &= (bit8) ~16;
		}

		if (c64keyboard[C64Keys::C64K_JOY2FIRE2] != 0)
		{
			localpotBx = 0x0;
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
	CIA::GetCommonState(state.cia);
	state.nextKeyboardScanClock = nextKeyboardScanClock;
}

void CIA1::SetState(const SsCia1V2 &state)
{
	CIA::SetCommonState(state.cia);
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

void CIA1::UpgradeStateV2ToV3(const SsCia1V2& in, SsCia1V2& out)
{
	out = {};
	CIA::UpgradeStateV2ToV3(in.cia, out.cia);
}