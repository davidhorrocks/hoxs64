#include <windows.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include "defines.h"
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

//pragma optimize( "ag", on )
#define KEYBOARDSCANINTERVAL (PALCLOCKSPERSECOND / 40)

#define KEYMATRIX_DOWN(row,col) keyboard_matrix[row] = \
	(keyboard_matrix[row] & (bit8)~(1<<col));\
	keyboard_rmatrix[col] = \
	(keyboard_rmatrix[col] & (bit8)~(1<<row))

CIA1::CIA1()
{
	cfg = 0L;
	appStatus = 0L;
	dx = 0L;
	cpu = 0L;
	vic = 0L;
	tape64 = 0L;
	pIC64 = 0L;
	pIAutoLoad = 0L;
	nextKeyboardScanClock = 0;
	ZeroMemory(&c64KeyMap[0], sizeof(c64KeyMap));
	m_bAltLatch = false;
	ResetKeyboard();
}

HRESULT CIA1::Init(CConfig *cfg, CAppStatus *appStatus, IC64 *pIC64, CPU6510 *cpu, VIC6569 *vic, Tape64 *tape64, CDX9 *dx, IAutoLoad *pAutoLoad)
{
	ClearError();
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->dx = dx;
	this->cpu = cpu;
	this->vic = vic;
	this->tape64 = tape64;
	this->pIAutoLoad = pAutoLoad;
	this->pIC64 = pIC64;
	return S_OK;
}

void CIA1::ExecuteDevices(ICLK sysclock)
{
	if ((ICLKS)(CurrentClock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + KEYBOARDSCANINTERVAL;
		ReadKeyboard();
	}
	tape64->Tick(sysclock);
}

void CIA1::SetWakeUpClock()
{
	CIA::SetWakeUpClock();
	if ((ICLKS)(nextWakeUpClock - nextKeyboardScanClock) > 0)
		nextWakeUpClock = nextKeyboardScanClock;

	if ((ICLKS)(nextWakeUpClock - tape64->nextTapeTickClock) >= 0)
		nextWakeUpClock = tape64->nextTapeTickClock;
	
}

void CIA1::ResetKeyboard()
{
int i;

	joyport1 = 0xFF;
	joyport2 = 0xFF;
	for (i=0 ; i < 8 ; i++)
	{
		keyboard_matrix[i] = 0xFF;
		keyboard_rmatrix[i] = 0xFF;
	}
}

/*
Key matrix rules  revised again
General Notes
1) Keys that are held will connect a Port A pin to a Port B pin in 
accordance with the key matrix layout.
2) The ports always read the pin levels.
3) Port B with keys held can sometimes force Port A to 1 even though  Port A 
is outputting a 0 (3 or 2 Port B strong 1s needed).. 
3) Port B with keys held can force Port A to 0 even though  Port A 
is outputting a 1 with just one Port B zero.
3) Port A with keys held can sometimes force Port B to 0 even though  Port B 
is outputting a strong 1 (3 or 2 Port A zeros needs).
4) Port A with keys held can never force Port B to 1 when Port B is 
outputting a 0.
5) The joystick can always drag the ports down to 0.
6) Port B is not symmetrical with Port A. It is as though Port B is stronger than Port A.
*/

bit8 CIA1::ReadPortA()
{
bit8 zB,data8,wB;
bit8 p;
bit8 zA,wA;
	//TEST Nitro 16 - Space bar / light pen bug fix
	if ((ICLKS)(CurrentClock - nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + KEYBOARDSCANINTERVAL;
		ReadKeyboard();
	}

	wA = PortAOutput_Strong1s() & joyport2;	
	zA = PortAOutput_Strong0s() & joyport2;


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
			2) Port B can force a Port A pin to 0 if that Port A matrix row contains 
			held keys of which the number of held keys relaying Port B 0s is equal to or 
			greater than the number of held keys that are relaying Port B "strong" 1s. 
			(not weak input mode 1s)
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
			3) Port B can force a Port A pin to 1 if that Port A matrix row contains one 
			or more held keys of which the number of keys relaying Port B strong 1s is 3 
			greater than the number of held keys that are relaying Port B 0s.
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
bit8 zA,data8,wA;
bit8 p;
bit8 zB,wB;

	if ((ICLKS)(CurrentClock - nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + KEYBOARDSCANINTERVAL;
		ReadKeyboard();
	}

	wB = PortBOutput_Strong1s() & joyport1;
	zB = PortBOutput_Strong0s() & joyport1;

	wA = PortAOutput_Strong1s() & joyport2;	
	zA = PortAOutput_Strong0s() & joyport2;
	data8 = PortBOutput_Strong0s();	

	//TEST
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
			1) Port B feed back loop
			Port B can experience a zero-ing feed back across keys in a matrix row from 
			its own outputted 0s. This can happen in a matrix row where the number of 
			held keys relaying Port B 0s is greater than the number of held keys that 
			are relaying Port B "strong" 1s This feedback can not zero out a Port B 
			strong 1.
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


void CIA1::WritePortA()
{
	if ((ICLKS)(CurrentClock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + KEYBOARDSCANINTERVAL;
		ReadKeyboard();
	}
	else
	{
		LightPen();
	}
}

void CIA1::WritePortB()
{
	if ((ICLKS)(CurrentClock -  nextKeyboardScanClock) > 0)
	{
		nextKeyboardScanClock = CurrentClock + KEYBOARDSCANINTERVAL;
		ReadKeyboard();
	}
	else
	{
		LightPen();
	}
}

void CIA1::Reset(ICLK sysclock)
{
	nextKeyboardScanClock = sysclock;
	joyport1=0xff;
	joyport2=0xff;
	ResetKeyboard();
	CIA::Reset(sysclock);
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

	//TEST Nitro 16 does not like the light pen line to go low (active) by pressing space when port b is forcing the light pen line high.
	lightpen = (ReadPortB()) & 0x10;
	vic->SetLPLineClk(CurrentClock, lightpen!=0);
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

void CIA1::ReadKeyboard()
{
char     buffer[256]; 
HRESULT  hr; 
static BOOL restore_was_up=TRUE;
static BOOL F11_was_up=TRUE;
static BOOL F10_was_up=TRUE;
DIJOYSTATE  js1;
DIJOYSTATE  js2;
static LONG xpos1=0;
static LONG ypos1=0;
static LONG xpos2=0;
static LONG ypos2=0;

LPDIRECTINPUTDEVICE7 joystick7;
BOOL joy1ok;
BOOL joy2ok;
bit8 localjoyport1;
bit8 localjoyport2;


	localjoyport2=0xff;
	localjoyport1=0xff;
	ResetKeyboard();
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

	hr = dx->pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
	if FAILED(hr) 
	{ 
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			hr = dx->pKeyboard->Acquire();
			if ( FAILED(hr) )  
				return;
			hr = dx->pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
			if FAILED(hr) 
				return;
		}
		else
			return; 
	} 
	if (SUCCEEDED(hr))
	{
		hr=S_OK;
	}

	if (joy1ok)
	{
		joystick7 = (LPDIRECTINPUTDEVICE7) dx->GetJoy(JOY1);
		joystick7->Poll();
		hr = joystick7->GetDeviceState(sizeof(DIJOYSTATE), &js1);
		if(hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			hr = joystick7->Acquire();
			hr = joystick7->GetDeviceState(sizeof(DIJOYSTATE), &js1);
		}
		if (SUCCEEDED(hr))
		{
			xpos1 = *((LONG *)(((BYTE *)&js1) + cfg->m_joy1config.dwOfs_X));
			ypos1 = *((LONG *)(((BYTE *)&js1) + cfg->m_joy1config.dwOfs_Y));
		}
		else
			joy1ok = FALSE;
	}
	if (joy2ok)
	{
		joystick7 = (LPDIRECTINPUTDEVICE7) dx->GetJoy(JOY2);
		joystick7->Poll();
		hr = joystick7->GetDeviceState(sizeof(DIJOYSTATE), &js2);
		if(hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
		{
			hr = joystick7->Acquire();
			hr = joystick7->GetDeviceState(sizeof(DIJOYSTATE), &js2);
		}
		if (SUCCEEDED(hr))
		{
			xpos2 = *((LONG *)(((BYTE *)&js2) + cfg->m_joy2config.dwOfs_X));
			ypos2 = *((LONG *)(((BYTE *)&js2) + cfg->m_joy2config.dwOfs_Y));
		}
		else
			joy2ok = FALSE;
	}

	if (joy1ok)
	{
		if (cfg->m_joy1config.bXReverse)
		{
			if (xpos1 < cfg->m_joy1config.xLeft) 
				localjoyport1 &= (bit8) ~8;
			else if (xpos1 > cfg->m_joy1config.xRight)
				localjoyport1 &= (bit8) ~4;
		}
		else
		{
			if (xpos1 < cfg->m_joy1config.xLeft) //LEFT   
				localjoyport1 &= (bit8) ~4;
			else if (xpos1 > cfg->m_joy1config.xRight) //RIGHT   
				localjoyport1 &= (bit8) ~8;
		}
		if (cfg->m_joy1config.bYReverse)
		{
			if (ypos1 < cfg->m_joy1config.yUp) 
				localjoyport1 &= (bit8) ~2;
			else if (ypos1 > cfg->m_joy1config.yDown)
				localjoyport1 &= (bit8) ~1;
		}
		else
		{
			if (ypos1 < cfg->m_joy1config.yUp) //UP
				localjoyport1 &= (bit8) ~1;
			else if (ypos1 > cfg->m_joy1config.yDown) //DOWN
				localjoyport1 &= (bit8) ~2;
		}
		if (((BYTE *)&js1)[cfg->m_joy1config.dwOfs_firebutton] & 0x80) //FIRE
		{
			localjoyport1 &= (bit8) ~16; //FIRE
		}
	}

	if (joy2ok)
	{
		if (cfg->m_joy2config.bXReverse)
		{
			if (xpos2 < cfg->m_joy2config.xLeft)
				localjoyport2 &= (bit8) ~8;
			else if (xpos2 > cfg->m_joy2config.xRight)
				localjoyport2 &= (bit8) ~4;
		}
		else
		{
			if (xpos2 < cfg->m_joy2config.xLeft) //LEFT   
				localjoyport2 &= (bit8) ~4;
			else if (xpos2 > cfg->m_joy2config.xRight) //RIGHT   
				localjoyport2 &= (bit8) ~8;
		}
		if (cfg->m_joy2config.bYReverse)
		{
			if (ypos2 < cfg->m_joy2config.yUp)
				localjoyport2 &= (bit8) ~2;
			else if (ypos2 > cfg->m_joy2config.yDown)
				localjoyport2 &= (bit8) ~1;
		}
		else
		{
			if (ypos2 < cfg->m_joy2config.yUp) //UP
				localjoyport2 &= (bit8) ~1;
			else if (ypos2 > cfg->m_joy2config.yDown) //DOWN
				localjoyport2 &= (bit8) ~2;
		}
		if (((BYTE *)&js2)[cfg->m_joy2config.dwOfs_firebutton] & 0x80) //FIRE
		{
			localjoyport2 &= (bit8) ~16; //FIRE
		}
	}

	if (RAWKEYDOWN(buffer, DIK_LALT) || GetAsyncKeyState(VK_MENU) < 0 || m_bAltLatch)	 
	{
		//Make sure all emulated c64 keys are released before recognising an emulated c64 key.
		m_bAltLatch = true;
		int k;
		for (k = 0; k < C64K_MAX && k < _countof(buffer); k++)
		{
			if ((buffer[c64KeyMap[k]] & 0x80)!=0)
				break;
		}
		if (k >= _countof(buffer) || k >= C64K_MAX)
			m_bAltLatch = false;
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
											
		if (RAWKEYDOWN(buffer, DIK_F11))
		{
			if (F10_was_up)
				pIC64->PostSoftReset(true);
			F10_was_up=FALSE;
		}
		else
			F10_was_up=TRUE;
			
			
		if (RAWKEYDOWN(buffer, DIK_F12))   
		{
			if (F11_was_up)
				pIC64->PostHardReset(true);
			F11_was_up=FALSE;
		}
		else
			F11_was_up=TRUE;
			
					
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
					cpu->SetNMI(CurrentClock);
				cpu->NMI = currentNMI;
			}
			restore_was_up=FALSE;
		}
		else
			restore_was_up=TRUE;

		if (KEYDOWN(buffer, C64K_CURSORRIGHT))
		{
			KEYMATRIX_DOWN(0, 2);
		}

		if (KEYDOWN(buffer, C64K_CURSORDOWN))
		{
			KEYMATRIX_DOWN(0, 7);
		}
	}

	if (cfg->m_bAllowOpposingJoystick)
	{
		if (KEYDOWN(buffer, C64K_JOY1UP))
			localjoyport1 &= (bit8) ~1;
		if (KEYDOWN(buffer, C64K_JOY1DOWN))   
			localjoyport1 &= (bit8) ~2;
		if (KEYDOWN(buffer, C64K_JOY1LEFT))   
			localjoyport1 &= (bit8) ~4;
		if (KEYDOWN(buffer, C64K_JOY1RIGHT))   
			localjoyport1 &= (bit8) ~8;

		if (KEYDOWN(buffer, C64K_JOY2UP))
			localjoyport2 &= (bit8) ~1;
		if (KEYDOWN(buffer, C64K_JOY2DOWN))   
			localjoyport2 &= (bit8) ~2;
		if (KEYDOWN(buffer, C64K_JOY2LEFT))   
			localjoyport2 &= (bit8) ~4;
		if (KEYDOWN(buffer, C64K_JOY2RIGHT))
			localjoyport2 &= (bit8) ~8;
	}
	else
	{
		if (KEYDOWN(buffer, C64K_JOY1UP))
			localjoyport1 &= (bit8) ~1;
		else if (KEYDOWN(buffer, C64K_JOY1DOWN))   
			localjoyport1 &= (bit8) ~2;
		if (KEYDOWN(buffer, C64K_JOY1LEFT))   
			localjoyport1 &= (bit8) ~4;
		else if (KEYDOWN(buffer, C64K_JOY1RIGHT))   
			localjoyport1 &= (bit8) ~8;

		if (KEYDOWN(buffer, C64K_JOY2UP))
			localjoyport2 &= (bit8) ~1;
		else if (KEYDOWN(buffer, C64K_JOY2DOWN))   
			localjoyport2 &= (bit8) ~2;
		if (KEYDOWN(buffer, C64K_JOY2LEFT))   
			localjoyport2 &= (bit8) ~4;
		else if (KEYDOWN(buffer, C64K_JOY2RIGHT))   
			localjoyport2 &= (bit8) ~8;
	}

	if (KEYDOWN(buffer, C64K_JOY2FIRE))   
	{
		localjoyport2 &= (bit8) ~16;
	}
	if (KEYDOWN(buffer, C64K_JOY1FIRE))   
	{
		localjoyport1 &= (bit8) ~16;
	}
	if (cfg->m_bSwapJoysticks)
	{
		joyport1 = localjoyport2;
		joyport2 = localjoyport1;
	}
	else
	{
		joyport1 = localjoyport1;
		joyport2 = localjoyport2;
	}

	LightPen();
}
