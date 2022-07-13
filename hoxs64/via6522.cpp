#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"
#include "savestate.h"
#include "c6502.h"
#include "d1541.h"
#include "via6522.h"
#include "p64.h"
#include "d64.h"

void VIA::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock = sysclock;
	DevicesClock = sysclock;
	bLatchA = false;
	bLatchB = false;
	ora=0;
	ira=0;
	orb=0;
	irb=0;
	ddra=0;
	ddrb=0;
	if (poweronreset)
	{
		timer1_counter.word=0x01AA;
		timer1_latch.word=0x01AA;
		timer2_counter.word=0x01AA;
		timer2_latch.word=0x01AA;
		shiftRegisterData=0;
	}
	acr=0;
	pcr=0;
	ca1_in=false;
	ca1_in_prev=false;
	ca2_in=false;
	ca2_in_prev=false;
	cb1_in=false;
	cb1_in_prev=false;
	cb2_in=false;
	cb2_in_prev=false;
	ca2_out=true;
	cb1_out=true;
	cb2_out=true;	
	ifr=0;
	ier=0;
	delay=0;
	feed=VIACountA2 | VIACountB2;
	shiftRegisterMode=0;
	shiftClockLevel=true;
	bPB7TimerMode = 0;
	bPB7Toggle = 0;
	bPB7TimerOut = 0;
	dec_2=1;
	no_change_count=0;
	idle=0;
	shiftRegisterData=0;
	shiftCounter=0;
}

void VIA::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ClearSystemInterrupt();
	WriteRegister(11, CurrentClock, acr);
	WriteRegister(12, CurrentClock, pcr);
	LocalCA2Output(true);
	LocalCB1Output(true);
	LocalCB2Output(true);
	SetPinsPortA(PortAOutput());
	SetPinsPortB(PortBOutput());
}

void VIA::ExecuteCycle(ICLK sysclock)
{
bit8 new_ifr=0;
ICLKS clocks;
bit8 shiftmsb;

	if ((ICLKS)(DevicesClock - sysclock) < 0)
	{
		DevicesClock = sysclock;
		ExecuteDevices(sysclock);
	}

	clocks = (ICLKS)(sysclock - CurrentClock);
	while (clocks-- > 0)
	{
		CurrentClock++;
		if (idle)
		{
			timer1_counter.word -= 1;
			timer2_counter.word -= dec_2;
			if (timer1_counter.word < VIA_MINIMUM_STAY_IDLE)
			{
				// Timer 1 16 bit counter is close to timing out.
				idle=0;
				no_change_count=0;
			}

			if (shiftRegisterMode == 0)
			{
				if (timer2_counter.word < VIA_MINIMUM_STAY_IDLE && dec_2 != 0)
				{
					// Timer 2 16 bit counter is close to timing out.
					idle=0;
					no_change_count=0;
				}
			}
			else if (timer2_counter.byte.loByte < VIA_MINIMUM_STAY_IDLE && dec_2 != 0)
			{
				// Timer 2 8 bit low order counter is close to timing out.
				idle=0;
				no_change_count=0;
			}
			else if (shiftRegisterMode == 2 || shiftRegisterMode == 6)
			{
				// Shift Register 02 modes
				idle=0;
				no_change_count=0;
			}

			continue;
		}

		old_delay = delay;
		old_feed = feed;
		if (delay & VIALoadA2)
		{
            timer1_counter.word = timer1_latch.word;
		}
        else if (delay & VIACountA3)
        {
			timer1_counter.word -= 1;
        }

		if (timer1_counter.word==0)
		{
            if ((acr & 0x40) != 0 && (feed & VIAPostOneShotA0) == 0)
			{
				//Continuous; Free running; Square wave if PB7 out is enabled; nnnn ... 0001 0000 FFFF nnnn ... 0001 0000 FFFF nnnn ...
				delay |= VIALoadA0;
				delay |= VIATimer1Timeout0;
				bPB7Toggle = bPB7Toggle ^ 0x80;
			}
			else
			{
				//One shot; Timed interrupt; Underflow nnnn ... 0001 0000 FFFF FFFE FFFD ... 0000 FFFF FFFE FFFD
				if ((delay & VIAPostOneShotA0) == 0)
                {
					delay |= VIATimer1Timeout0;
                    bPB7Toggle = bPB7Toggle ^ 0x80;
                }

				feed |= VIAPostOneShotA0;
                delay |= VIALoadA0;
			}

			if ((bPB7TimerMode & 0x80) != 0)
			{
				//timer 1 to PB7 enabled
				bPB7TimerOut = bPB7Toggle;
			}
		}

		if ((delay & VIATimer1Timeout1) != 0)
		{
			new_ifr |= VIA_INT_T1;
		}

		if (delay & VIALoadB2)
		{
			timer2_counter.byte.loByte = timer2_latch.byte.loByte;
		}
		else if (delay & VIACountB3)
        {
			timer2_counter.word -= 1;
        }

		bool timer2LowTimeout = false;
		bool timer2AllTimeout = false;
		if (timer2_counter.word==0 && (delay & VIACountB2))
		{
			if ((delay & VIAPostOneShotB0) == 0)
			{
				delay |= VIATimer2Timeout0;
			}

			feed |= VIAPostOneShotB0;
			timer2AllTimeout = true;
		}
		
		if (timer2_counter.byte.loByte==0 && (delay & VIACountB2))
		{
			timer2LowTimeout = true;			
		}

		if ((delay & VIATimer2Timeout1) != 0)
		{
			new_ifr |= VIA_INT_T2;
		}

		// The shift register control
		switch (shiftRegisterMode)
		{
		case 0:// DISABLED
			if ((pcr & 0xC0) == 0x80)
			{
				if (delay & VIACB2Low0)
				{
					if (cb2_out)
					{
						LocalCB2Output(false);
					}
				}
				else
				{
					if (!cb2_out)
					{
						LocalCB2Output(true);
					}
				}
			}

			break;
		case 1:// SHIFT IN UNDER CONTROL OF T2 
			if (timer2LowTimeout)
			{
				delay |= VIAShift0;
				delay |= VIALoadB0;
			}

			if ((delay & VIAShift2) != 0)
			{				
				if (shiftCounter < 8)
				{
					shiftClockLevel = !shiftClockLevel;
					if (shiftClockLevel)
					{
						// CB1 rising. Shift in from CB2
						shiftRegisterData <<= 1; 
						shiftRegisterData |= cb2_in ? 1 : 0;
						shiftCounter++;
						if (shiftCounter == 8)
						{
							new_ifr |= VIA_INT_SER;
						}
					}
				}
			}

			LocalCB1Output(shiftClockLevel);
			break;
		case 2:// SHIFT IN UNDER CONTROL OF 02 
			if ((delay & VIAShift2) != 0)
			{				
				if (shiftCounter < 8)
				{
					shiftClockLevel = !shiftClockLevel;
					if (shiftClockLevel)
					{
						// CB1 rising. Shift in from CB2
						shiftRegisterData <<= 1; 
						shiftRegisterData |= cb2_in ? 1 : 0;
						shiftCounter++;
						if (shiftCounter == 8)
						{
							new_ifr |= VIA_INT_SER;
						}
					}
				}
			}

			LocalCB1Output(shiftClockLevel);
			break;
		case 3:// SHIFT IN UNDER CONTROL OF EXT.CLK (CB1)
			shiftRegisterData = shiftRegisterData;
			break;
		case 4:// SHIFT OUT FREE-RUNNING AT T2 RATE
			if (timer2LowTimeout)
			{
				delay |= VIAShift0;
				delay |= VIALoadB0;
			}

			if ((delay & VIAShift2) != 0)
			{				
				shiftClockLevel = !shiftClockLevel;
				if (shiftClockLevel)
				{
					shiftmsb = (shiftRegisterData >> 7) & 1;
					shiftRegisterData = (shiftRegisterData << 1) | shiftmsb;
					LocalCB2Output(shiftmsb != 0);
				}
			}

			LocalCB1Output(shiftClockLevel);
			break;
		case 5:// SHIFT OUT UNDER CONTROL OF T2
			if (timer2LowTimeout)
			{
				delay |= VIAShift0;
				delay |= VIALoadB0;
			}

			if ((delay & VIAShift2) != 0)
			{				
				if (shiftCounter < 8)
				{
					shiftClockLevel = !shiftClockLevel;
					if (shiftClockLevel)
					{
						shiftmsb = (shiftRegisterData >> 7) & 1;
						shiftRegisterData = (shiftRegisterData << 1) | shiftmsb;
						LocalCB2Output(shiftmsb != 0);
						shiftCounter++;
						if (shiftCounter == 8)
						{
							new_ifr |= VIA_INT_SER;
						}
					}
				}
			}

			LocalCB1Output(shiftClockLevel);
			break;
		case 6:// SHIFT OUT UNDER CONTROL OF 02
			if ((delay & VIAShift2) != 0)
			{				
				if (shiftCounter < 8)
				{
					shiftClockLevel = !shiftClockLevel;
					if (shiftClockLevel)
					{
						shiftmsb = (shiftRegisterData >> 7) & 1;
						shiftRegisterData = (shiftRegisterData << 1) | shiftmsb;
						LocalCB2Output(shiftmsb != 0);
						shiftCounter++;
						if (shiftCounter == 8)
						{
							new_ifr |= VIA_INT_SER;
						}
					}
				}
			}

			LocalCB1Output(shiftClockLevel);
			break;
		case 7:// SHIFT OUT UNDER CONTROL OF EXT.CLK
			LocalCB1Output(shiftClockLevel);
			break;
		}

		if ((pcr & 0x0C) == 0x08)
		{
			if (delay & VIACA2Low0)
			{
				if (ca2_out)
				{
					LocalCA2Output(false);
				}
			}
			else
			{
				if (!ca2_out)
				{
					LocalCA2Output(true);
				}
			}
		}

		if (delay & VIATrans1)
		{
			if (delay & VIACA1Trans1)
			{
				TransitionCA1();
			}

			if (delay & VIACA2Trans1)
			{
				TransitionCA2();
			}

			if (delay & VIACB1Trans1)
			{
				TransitionCB1();
			}

			if (delay & VIACB2Trans1)
			{
				TransitionCB2();
			}
		}

		ifr |= new_ifr;
		if (new_ifr & ier)
		{
			delay |= VIAInterrupt0;
		}

		if (delay & VIAInterrupt1)
        {
			SetSystemInterrupt();
        }
			
		delay = ((delay << 1) & VIADelayMask) | feed;
		if (delay==old_delay && feed==old_feed)
		{
			if ((timer1_counter.word < VIA_MINIMUM_GO_IDLE_TIME) || (((acr & 0x20) == 0) && timer2_counter.word < VIA_MINIMUM_GO_IDLE_TIME) || (((acr & 0x1c) != 0) && timer2_counter.byte.loByte < VIA_MINIMUM_GO_IDLE_TIME) || ((delay & (VIAShift0 | VIAShift1 | VIAShift2 | VIAShift3 | VIAShift4)) != 0))
            {
				no_change_count = 0;
            }
			else
			{
				no_change_count++;
				if (no_change_count > VIA_NO_CHANGE_TO_IDLE)
				{
					idle=1;
					dec_2=(bit16) ((acr & 0x20)==0);
				}
			}
		}
		else
        {
			no_change_count=0;
        }
	}
}

void VIA::WakeUp()
{
	no_change_count=0;
	idle=0;
}

bit8 VIA::ReadRegister(bit16 address, ICLK sysclock)
{
bit8 t;

	ExecuteCycle(sysclock);
	switch (address & 15)
	{
	case 0://port b
		switch ((pcr>>5) & 7)
		{
		case 0:// cb2 interrupt on negative edge, clear interrupt on port b access
			ifr &= ~(VIA_INT_CB2);
			break;
		case 1:// cb2 independent interrupt on negative edge
			break;
		case 2:// cb2 interrupt on positive edge, clear interrupt on port b access
			ifr &= ~(VIA_INT_CB2);
			break;			
		case 3:// cb2 independent interrupt on positive edge
			break;
		case 4:// handshake, cb2 goes low until a cb1 active edge occurs
			// No read handshake on port b
			break;
		case 5:// pulse cb2 for 1 clock
			// No read handshake on port b
			break;
		case 6:// hold cb2 low
			break;
		case 7:// hold cb2 hi
			break;
		}

		ifr &= ~(VIA_INT_CB1);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}

		if (((acr & 2) !=0) && bLatchB)
		{
			//Port latching
			bLatchB = false;
			t = irb;
		}
		else
		{
			//No latching
			t = ReadPinsPortB();
		}

		t = (((orb & ddrb) | (t & ~ddrb)) & ~bPB7TimerMode) | (bPB7TimerMode & bPB7TimerOut);
        return t;
	case 1://port a
		switch ((pcr>>1) & 7)
		{		
		case 0:// ca2 interrupt on negative edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 1:// ca2 independent int on negative edge
			break;
		case 2:// ca2 interrupt on positive edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 3:// ca2 independent interrupt on positive edge
			break;
		case 4:// handshake, ca2 goes low until a ca1 active edge occurs
			ifr &= ~(VIA_INT_CA2);
			delay |= VIACA2Low0;
			feed |= VIACA2Low0;
			WakeUp();
			break;
		case 5:// pulse ca2 for 1 clock
			ifr &= ~(VIA_INT_CA2);
			delay |= VIACA2Low0;
			feed &= ~VIACA2Low0;
			WakeUp();
			break;
		case 6:// hold ca2 low
			break;
		case 7:// hold ca2 hi
			break;
		}

		ifr &= ~(VIA_INT_CA1);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}

		if (((acr & 1) !=0) && bLatchA)
		{
			//Port latching
			bLatchA = false;
			return ira;
		}
		else
		{
			//No latching
			return ReadPinsPortA();
		}

	case 2://ddrb
		return ddrb;
	case 3://ddra
		return ddra;
	case 4://t1c-l
		WakeUp();
		ifr &= (~VIA_INT_T1);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}

		return timer1_counter.byte.loByte;
	case 5://t1c-h
		return timer1_counter.byte.hiByte;
	case 6://t1l-l
		return timer1_latch.byte.loByte;
	case 7://t1l-h
		return timer1_latch.byte.hiByte;
	case 8://t2c-l
		WakeUp();
		ifr &= (~VIA_INT_T2);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}

		return timer2_counter.byte.loByte;
	case 9://t2c-h
		return timer2_counter.byte.hiByte;
	case 10://sr
		if ((ifr & VIA_INT_SER) != 0)
		{
			shiftCounter = 0;
			shiftClockLevel = true;
			if (shiftRegisterMode == 2 || shiftRegisterMode == 6)
			{
				// 02 mode
				// "SHIFT IN UNDER CONTROL OF 02" or "SHIFT OUT UNDER CONTROL OF 02"
				feed |= VIAShift0;
				delay |= (VIAShift0 | VIAShift1);
			}
			else
			{
				delay |= VIAShift1;
			}
		}

		ifr &= (~VIA_INT_SER);
		if ((ifr & ier) == 0)
		{			
			ClearSystemInterrupt();
		}

		return shiftRegisterData;
	case 11://acr
		return acr;
	case 12://pcr
		return pcr;
	case 13://ifr
		if (ifr & ier)
		{
			SetSystemInterrupt();
			return 0x80 | ifr;
		}
		else
		{
			ClearSystemInterrupt();
			return ifr;
		}
	case 14://ier
		return ier | 0x80;
	case 15:// port a no handshake
		switch ((pcr>>1) & 7)
		{
		case 0:// ca2 interrupt on negative edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 1:// ca2 independent interrupt on negative edge
			break;
		case 2:// ca2 interrupt on positive edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 3:// ca2 independent interrupt on positive edge
			break;
		case 4:// handshake, ca2 goes low until a ca1 active edge occurs
			ifr &= ~(VIA_INT_CA2);
			break;
		case 5:// pulse ca2 for 1 clock
			ifr &= ~(VIA_INT_CA2);
			break;
		case 6:// hold ca2 low
			break;
		case 7:// hold ca2 hi
			break;
		}

		ifr &= ~(VIA_INT_CA1);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}
		
		if (((acr & 1) != 0) && bLatchA)
		{
			//Port latching
			bLatchA = false;
			return ira;
		}
		else
		{
			//No latching
			return ReadPinsPortA();
		}
	default:
		return 0;
	}
}

bit8 VIA::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
bit8 t;
	ExecuteCycle(sysclock);

	switch (address & 15)
	{
	case 0://port b
		//TEST latchPortB
		if (((acr & 2) !=0) && bLatchB)
		{
			//Port latching
			t = irb;
		}
		else
		{
			//No latching
			t = ReadPinsPortB();
		}

		return (orb & ddrb) | (t & ~ddrb);
	case 1://port a
		//TEST latchPortA
		if (((acr & 1) !=0) && bLatchA)
		{
			//Port latching
			return ira;
		}
		else
		{
			//No latching
			return ReadPinsPortA();
		}
	case 4://t1c-l
		return timer1_counter.byte.loByte;
	case 5://t1c-h
		return timer1_counter.byte.hiByte;
	case 6://t1l-l
		return timer1_latch.byte.loByte;
	case 7://t1l-h
		return timer1_latch.byte.hiByte;
	case 8://t2c-l
		return timer2_counter.byte.loByte;
	case 9://t2c-h
		return timer2_counter.byte.hiByte;
	case 10://sr
		return shiftRegisterData;
	case 11://acr
		return acr;
	case 12://pcr
		return pcr;
	case 13://ifr
		if (ifr & ier)
		{
			return 0x80 | ifr;
		}
		else
		{
			return ifr;
		}
	case 15://port a no handshake
		if (((acr & 1) !=0) && bLatchA)
		{
			//Port latching
			return ira;
		}
		else
		{
			//No latching
			return ReadPinsPortA();
		}

		break;
	default:
		return ReadRegister(address, sysclock);
	}
}

void VIA::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
bit8 oldpb6;
bit8 newpb6;

	ExecuteCycle(sysclock);
	switch (address & 15)
	{
	case 0://port b
		switch ((pcr>>5) & 7)
		{
		case 0:// cb2 as input, interrupt on negative edge, clear interrupt on port b access
			ifr &= ~(VIA_INT_CB2);
			break;
		case 1:// cb2 as input independent interrupt on negative edge
			break;
		case 2:// cb2 as input, interrupt on positive edge, clear interrupt on port b access
			ifr &= ~(VIA_INT_CB2);
			break;
		case 3:// cb2 as input, independent interrupt on positive edge
			break;
		case 4:// cb2 as output, handshake, cb2 goes low
			ifr &= ~(VIA_INT_CB2);
			delay |= VIACB2Low0;
			feed |= VIACB2Low0;
			WakeUp();
			break;
		case 5:// cb2 as output, pulse cb2 for 1 clock
			ifr &= ~(VIA_INT_CB2);
			delay |= VIACB2Low0;
			feed &= ~VIACB2Low0;
			WakeUp();
			break;
		case 6:// hold cb2 low
			break;
		case 7:// hold cb2 hi
			break;
		}

		ifr &= ~(VIA_INT_CB1);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

		if (acr & 0x20)
		{
			// Timer 2 counts negative going edges on PB6.
			oldpb6 = this->ReadPinsPortB() & this->PortBOutput() & 0x40;		
		}

		orb = data;
		SetPinsPortB(PortBOutput());
		if (acr & 0x20)
		{
			// Timer 2 counts negative going edges on PB6.
			newpb6 = this->ReadPinsPortB() & this->PortBOutput() & 0x40;
			if (newpb6 == 0 && oldpb6 != 0)
			{
				// A negative going edge occurred on PB6.
				WakeUp();
				delay |= VIACountB2;
			}
		}

		break;			
	case 1://port a
		switch ((pcr>>1) & 7)
		{		
		case 0:// ca2 as input, interrupt on negative edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 1:// ca2 as input, independent interrupt on negative edge
			break;
		case 2:// ca2 as input, interrupt on positive edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 3:// ca2 as input, independent interrupt on positive edge
			break;
		case 4:// ca2 as output, handshake, ca2 goes low until a ca1 active edge occurs
			ifr &= ~(VIA_INT_CA2);
			delay |= VIACA2Low0;
			feed |= VIACA2Low0;
			WakeUp();
			break;
		case 5:// ca2 as output, pulse ca2 for 1 clock
			ifr &= ~(VIA_INT_CA2);
			delay |= VIACA2Low0;
			feed &= ~VIACA2Low0;
			WakeUp();
			break;
		case 6:// hold ca2 low
			break;
		case 7:// hold ca2 hi
			break;
		}

		ifr &= ~(VIA_INT_CA1);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

		ora = data;
		SetPinsPortA(PortAOutput());
		break;
	case 2://ddrb
		if (acr & 0x20)
		{
			// Timer 2 counts negative going edges on PB6.
			oldpb6 = this->ReadPinsPortB() & this->PortBOutput() & 0x40;		
		}

		ddrb = data;
		SetPinsPortB(PortBOutput());
		if (acr & 0x20)
		{
			// Timer 2 counts negative going edges on PB6.
			newpb6 = this->ReadPinsPortB() & this->PortBOutput() & 0x40;
			if (newpb6 == 0 && oldpb6 != 0)
			{
				// A negative going edge occurred on PB6.
				WakeUp();
				delay |= VIACountB2;
			}
		}
		break;
	case 3://ddra
		ddra = data;
		SetPinsPortA(PortAOutput());
		break;
	case 4://t1c-l
		timer1_latch.byte.loByte = data;
		break;
	case 5://t1c-h
		//Load T1 latch high then load T1 counter from the T1 latch
		WakeUp();
		timer1_latch.byte.hiByte = data;
		timer1_counter.word = timer1_latch.word;

		ifr &= ~(VIA_INT_T1);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

        delay |= VIALoadA2;
		delay &= ~VIACountA3;
        delay &= ~(VIACountA3 | VIALoadA1);
		feed &= ~VIAPostOneShotA0;
		delay &= ~VIAPostOneShotA0;

		//Toggle PB7 on write to Timer 1 counter high byte		
        bPB7Toggle = bPB7TimerMode ^ 0x80;
		if ((bPB7TimerMode & 0x80) != 0)
		{
			//timer 1 to PB7 enabled
			bPB7TimerOut = bPB7Toggle;
		}
		break;
	case 6://t1l-l
		timer1_latch.byte.loByte = data;
		break;
	case 7://t1l-h
		timer1_latch.byte.hiByte = data;
		ifr &= (~VIA_INT_T1);
		if ((ifr & ier)==0)
		{
			ClearSystemInterrupt();
		}

		break;
	case 8://t2c-l
		timer2_latch.byte.loByte = data;
		break;
	case 9://t2c-h
		//Load T2 latch high then load T2 counter from the T2 latch
		WakeUp();
		timer2_counter.byte.hiByte = data;
		timer2_counter.byte.loByte = timer2_latch.byte.loByte;
		ifr &= ~(VIA_INT_T2);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

		delay &= ~VIACountB3;
        delay &= ~(VIACountB3 | VIALoadB1);
		feed &= ~VIAPostOneShotB0;
		delay &= ~VIAPostOneShotB0;
		break;
	case 10://sr
		if ((ifr & VIA_INT_SER) != 0)
		{
			shiftCounter = 0;
			shiftClockLevel = true;
			if (shiftRegisterMode == 2 || shiftRegisterMode == 6)
			{
				// 02 mode
				// "SHIFT IN UNDER CONTROL OF 02" or "SHIFT OUT UNDER CONTROL OF 02"
				feed |= VIAShift0;
				delay |= (VIAShift0 | VIAShift1);
			}
			else
			{
				delay |= VIAShift1;
			}
		}

		ifr &= (~VIA_INT_SER);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

		shiftRegisterData = data;
		break;
	case 11://acr
		acr = data;
		shiftRegisterMode = ((data >> 2) & 7);
        if ((feed & VIAPostOneShotA0) != 0)
        {
            bPB7Toggle = bPB7TimerMode ^ 0x80;
        }

		if ((data & 0x20) == 0)
		{
			// Count 02 mode;
			dec_2 = 1;
			feed |= VIACountB2;
            delay |= VIACountB2; 
		}
		else
		{
			// Counting PB6 mode. 
			// PB6 counting is not emulated. 
			// Is PB6 counted in output mode? If not then we should be OK because PB6 for VIA 1 is connected to a 1541 drive dip switch and PB6 for VIA 2 is unconnected.
			dec_2 = 0;
			delay &= ~(VIACountB2); 
			feed &= ~VIACountB2;
		}

		if ((data & 0x80) == 0)
		{
            //timer 1 to PB7 disabled
			bPB7TimerMode = 0;
		}
		else
		{
            //timer 1 to PB7 enabled
			bPB7TimerMode = 0x80;
			bPB7TimerOut = bPB7Toggle;
		}

		if (shiftRegisterMode != 2 && shiftRegisterMode != 6)
		{
			// Not 02 mode
			feed &= ~(VIAShift0);
		}

		if ((data & 1) == 0)
		{
			bLatchA = false;
		}
		if ((data & 2) == 0)
		{
			bLatchB = false;
		}

		WakeUp();
		break;
	case 12://pcr
		pcr = data;
		switch ((data>>1) & 7)
		{
		case 0:// ca2 interrupt on negative edge, clear interrupt on port a access
		case 1:// ca2 independent interrupt on negative edge
		case 2:// ca2 interrupt on positive edge, clear interrupt on port a access
		case 3:// ca2 independent interrupt on positive edge
			delay &= ~VIACA2Low0;
			feed &= ~VIACA2Low0;
			LocalCA2Output(true);
			break;
		case 4:// handshake, ca2 goes low until a ca1 active edge occurs
			delay &= ~VIACA2Low0;
			feed &= ~VIACA2Low0;
			WakeUp();
			break;
		case 5:// pulse ca2 for 1 clock
			delay &= ~VIACA2Low0;
			feed &= ~VIACA2Low0;
			WakeUp();
			break;
		case 6:// hold ca2 low
			delay |= VIACA2Low0;
			feed |= VIACA2Low0;
			LocalCA2Output(false);
			WakeUp();
			break;
		case 7:// hold ca2 hi
			delay &= ~VIACA2Low0;
			feed &= ~VIACA2Low0;
			LocalCA2Output(true);
			WakeUp();
			break;
		}
		
		if (shiftRegisterMode == 0)
		{
			// The shift register is not controlling CB2.
			switch ((pcr>>5) & 7)
			{
			case 0:// cb2 interrupt on negative edge, clear interrupt on port a access
			case 1:// cb2 independent interrupt on negative edge
			case 2:// cb2 interrupt on positive edge, clear interrupt on port a access
			case 3:// cb2 independent interrupt on positive edge
				delay &= ~VIACB2Low0;
				feed &= ~VIACB2Low0;
				LocalCB2Output(true);
				break;
			case 4:// handshake, cb2 goes low until a cb1 active edge occurs
				delay &= ~VIACB2Low0;
				feed &= ~VIACB2Low0;
				WakeUp();
				break;
			case 5:// pulse cb2 for 1 clock
				delay &= ~VIACB2Low0;
				feed &= ~VIACB2Low0;
				WakeUp();
				break;
			case 6:// hold cb2 low
				delay |= VIACB2Low0;
				feed |= VIACB2Low0;
				LocalCB2Output(false);
				WakeUp();
				break;
			case 7:// hold cb2 hi
				delay &= ~VIACB2Low0;
				feed &= ~VIACB2Low0;
				LocalCB2Output(true);
				WakeUp();
				break;
			}
		}

		break;
	case 13://ifr
		ifr = ifr & ~data;
		if (ifr & ier)
		{
			SetSystemInterrupt();
		}
		else
		{
			ClearSystemInterrupt();
		}

		break;
	case 14://ier
		if (data & 0x80)
		{
			ier |= (data & 0x7f);
		}
		else
		{
			ier &= (~data & 0x7f);
		}

		if (ifr & ier)
		{
			SetSystemInterrupt();
		}
		else
		{
			ClearSystemInterrupt();
		}

		break;
	case 15://port a no handshake
		switch ((pcr>>1) & 7)
		{
		case 0:// ca2 interrupt on negative edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 1:// ca2 independent interrupt on negative edge
			break;
		case 2:// ca2 interrupt on positive edge, clear interrupt on port a access
			ifr &= ~(VIA_INT_CA2);
			break;
		case 3:// ca2 independent interrupt on positive edge
			break;
		case 4:// handshake, ca2 goes low until a ca1 active edge occurs
			ifr &= ~(VIA_INT_CA2);
			break;
		case 5:// handshake pulse ca2 for 1 clock
			ifr &= ~(VIA_INT_CA2);
			break;
		case 6:// hold ca2 low
			break;
		case 7:// hold ca2 hi
			break;
		}

		ifr &= ~(VIA_INT_CA1);
		if ((ifr & ier) == 0)
		{
			ClearSystemInterrupt();
		}

		ora = data;
		SetPinsPortA(PortAOutput());
		break;
	}
}

bit8 VIA::PortAOutput()
{
	return (~ddra) | (ora & ddra);
}

bit8 VIA::PortBOutput()
{
	return (orb & ddrb & ~bPB7TimerMode) | (bPB7TimerOut & ddrb & bPB7TimerMode) | (~ddrb);
}

void VIA::OnTransitionCA1Low()
{
}

void VIA::TransitionCA1()
{
	if (pcr & 1)
	{
		//CA1 positive active edge
		if (ca1_in_prev == false)
		{
			ActiveTransitionCA1();
		}
	}
	else
	{
		//CA1 negative active edge
		if (ca1_in_prev != false)
		{
			ActiveTransitionCA1();
		}
	}
	if (ca1_in_prev != false)
	{
		OnTransitionCA1Low();
	}

	ca1_in_prev = !ca1_in_prev;
}

void VIA::TransitionCA2()
{
	switch ((pcr >> 1) & 7)
	{
	case 0://CA2 negative active edge	
		if (ca2_in_prev != false)
		{
			ActiveTransitionCA2();
		}
		break;
	case 1://CA2 independent interrupt negative active edge
		if (ca2_in_prev != false)
		{
			ActiveTransitionCA2();
		}
	case 2://CA2 positive active edge	
		if (ca2_in_prev == false)
		{
			ActiveTransitionCA2();
		}
		break;
	case 3://CA2 independent interrupt positive active edge
		if (ca2_in_prev == false)
		{
			ActiveTransitionCA2();
		}
		break;
	}

	ca2_in_prev = !ca2_in_prev;
}

void VIA::TransitionCB1()
{
	if (pcr & 0x10)
	{
		//CB1 positive active edge
		if (cb1_in_prev == false)
		{
			ActiveTransitionCB1();
		}
	}
	else
	{
		//CB1 negative active edge
		if (cb1_in_prev != false)
		{
			ActiveTransitionCB1();
		}
	}

	cb1_in_prev = !cb1_in_prev;
}

void VIA::TransitionCB2()
{
	switch ((pcr >> 5) & 7)
	{
	case 0://CB2 negative active edge	
		if (cb2_in_prev != false)
		{
			ActiveTransitionCB2();
		}
		break;
	case 1://CB2 independant interrupt negative active edge
		if (cb2_in_prev != false)
		{
			ActiveTransitionCB2();
		}
		break;
	case 2://CB2 positive active edge	
		if (cb2_in_prev == false)
		{
			ActiveTransitionCB2();
		}
		break;
	case 3://CB2 independant interrupt positive active edge
		if (cb2_in_prev == false)
		{
			ActiveTransitionCB2();
		}
		break;
	}

	cb2_in_prev = !cb2_in_prev;
}

void VIA::ActiveTransitionCA1()
{
	//TEST latchPortA
	if (((acr & 1) !=0) && !bLatchA)
	{
		bLatchA = true;
		ira = ReadPinsPortA();
	}

	if ((pcr & 0xe) == 0x8)
	{
		//Handshake CA2 CA2 goes high on CA1 active transition
		delay &= ~VIACA2Low0;
		feed &= ~VIACA2Low0;
		if (ca2_out==0)
		{
			LocalCA2Output(true);
		}

		WakeUp();
	}

	ifr |= VIA_INT_CA1;
	if (VIA_INT_CA1 & ier)
	{
		delay |= VIAInterrupt1;
		WakeUp();
	}
}

void VIA::ActiveTransitionCA2()
{
	ifr |= VIA_INT_CA2;
	if (VIA_INT_CA2 & ier)
	{
		delay |= VIAInterrupt1;
		WakeUp();
	}
}


void VIA::ActiveTransitionCB1()
{
	//TEST latchPortB
	if (((acr & 2) !=0) && !bLatchB)
	{
		bLatchB = true;
		irb = ReadPinsPortB();
	}

	if ((pcr & 0xe0) == 0x80)
	{
		//Handshake CB2 CB2 goes high on CB1 active transition
		delay &= ~VIACB2Low0;
		feed &= ~VIACB2Low0;
		if (shiftRegisterMode == 0)
		{
			// The shift register is not controlling CB2.
			if (cb2_out==0)
			{
				LocalCB2Output(true);
			}
		}

		WakeUp();
	}

	if (shiftRegisterMode != 4)
	{
		//is not: SHIFT OUT FREE-RUNNING AT T2 RATE
		ifr |= VIA_INT_CB1;
	}

	if (VIA_INT_CB1 & ier)
	{
		delay |= VIAInterrupt1;
		WakeUp();
	}
}

void VIA::ActiveTransitionCB2()
{
	ifr |= VIA_INT_CB2;
	if (VIA_INT_CB2 & ier)
	{
		delay |= VIAInterrupt1;
		WakeUp();
	}
}

void VIA::SetCA1Input(bool value, int phase)
{
	ca1_in = value;
	if (phase == 0)
	{
		if (ca1_in_prev != value)
		{
			delay = (delay) | VIACA1Trans1;
		}
		else
		{
			delay = delay & ~(VIACA1Trans1);
		}
	}
	else
	{
		bool next_in = (delay & VIACA1Trans1) ? !ca1_in_prev : ca1_in_prev;
		if (next_in != value)
		{
			delay|=VIACA1Trans0;
		}
		else
		{
			delay&=~VIACA1Trans0;
		}
	}

	WakeUp();
}

void VIA::SetCA2Input(bool value, int phase)
{
	ca2_in = value;
	if (phase == 0)
	{
		if (ca2_in_prev != value)
		{
			delay = (delay & ~(VIACA2Trans0 | VIACA2Trans1)) | VIACA2Trans1;
		}
		else
		{
			delay&=~VIACA2Trans1;
		}
	}
	else
	{
		bool next_in = (delay & VIACA2Trans1) ? !ca2_in_prev : ca2_in_prev;
		if (next_in != value)
		{
			delay|=VIACA2Trans0;
		}
		else
		{
			delay&=~VIACA2Trans0;
		}
	}
}

void VIA::SetCB1Input(bool value, int phase)
{
	cb1_in = value;
	if (phase == 0)
	{
		if (cb1_in_prev != value)
		{
			delay = (delay & ~(VIACB1Trans0 | VIACB1Trans1));
			if (shiftRegisterMode != 4)
			{
				//is not: SHIFT OUT FREE-RUNNING AT T2 RATE
				delay |= VIACB1Trans1;
			}
		}
		else
		{
			delay&=~VIACB1Trans1;
		}
	}
	else
	{
		bool next_in = (delay & VIACB1Trans1) ? !cb1_in_prev : cb1_in_prev;
		if (next_in != value)
		{
			if (shiftRegisterMode != 4)
			{
				//is not: SHIFT OUT FREE-RUNNING AT T2 RATE
				delay |= VIACB1Trans0;
			}
		}
		else
		{
			delay&=~VIACB1Trans0;
		}
	}
}

void VIA::SetCB2Input(bool value, int phase)
{
	cb2_in = value;
	if (phase == 0)
	{
		if (cb2_in_prev != value)
		{
			delay = (delay & ~(VIACB2Trans0 | VIACB2Trans1)) | VIACB2Trans1;
		}
		else
		{
			delay&=~VIACB2Trans1;
		}
	}
	else
	{
		bool next_in = (delay & VIACB2Trans1) ? !cb2_in_prev : cb2_in_prev;
		if (next_in != value)
		{
			delay|=VIACB2Trans0;
		}
		else
		{
			delay&=~VIACB2Trans0;
		}
	}
}

void VIA::LocalCA2Output(bool value)
{
	if (ca2_out != value)
	{
		ca2_out = value;
		SetCA2Output(value);
		SetCA2Input(value, 1);
	}
}

void VIA::LocalCB1Output(bool value)
{
	if (cb1_out != value)
	{
		cb1_out = value;
		SetCB1Output(value);
		SetCB1Input(value, 1);
	}
}

void VIA::LocalCB2Output(bool value)
{
	if (cb2_out != value)
	{
		cb2_out = value;
		SetCB2Output(value);
		SetCB2Input(value, 1);
	}
}

ICLK VIA::GetCurrentClock()
{
	return CurrentClock;
}

void VIA::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CurrentClock = sysclock;
	DevicesClock += v;
}

void VIA::GetState(SsViaCommon &state)
{
	ZeroMemory(&state, sizeof(state));
	state.ID = ID;
	state.CurrentClock = CurrentClock;	
	state.DevicesClock = DevicesClock;	
	state.bLatchA = bLatchA;
	state.bLatchB = bLatchB;
	state.ora = ora;
	state.ira = ira;
	state.orb = orb;
	state.irb = irb;
	state.ddra = ddra;
	state.ddrb = ddrb;
	state.timer1_counter = timer1_counter;
	state.timer2_counter = timer2_counter;
	state.timer1_latch = timer1_latch;
	state.timer2_latch = timer2_latch;
	state.acr = acr;
	state.pcr = pcr;
	state.ca1_in = ca1_in ? 1 : 0;
	state.ca1_in_prev = ca1_in_prev ? 1 : 0;
	state.ca2_in = ca2_in ? 1 : 0;
	state.ca2_in_prev = ca2_in_prev ? 1 : 0;
	state.cb1_in = cb1_in ? 1 : 0;
	state.cb1_in_prev = cb1_in_prev ? 1 : 0;
	state.cb2_in = cb2_in ? 1 : 0;
	state.cb2_in_prev = cb2_in_prev ? 1 : 0;
	state.ca2_out = ca2_out ? 1 : 0;
	state.cb2_out = cb2_out ? 1 : 0;
	state.shiftRegisterData = shiftRegisterData;
	state.ifr = ifr;
	state.ier = ier;
	state.shiftRegisterMode = shiftRegisterMode;
	state.shiftClockLevel = shiftClockLevel ? 1 : 0;
	state.delay = delay;
	state.feed = feed;
	state.old_delay = old_delay;
	state.old_feed = old_feed;
	state.shiftCounter = shiftCounter;
	state.Interrupt = Interrupt;
	state.bPB7TimerMode = bPB7TimerMode;
	state.bPB7Toggle = bPB7Toggle;
	state.bPB7TimerOut = bPB7TimerOut;
	state.no_change_count = no_change_count;
	state.dec_2 = dec_2;
	state.idle = idle;
}

void VIA::SetState(const SsViaCommon &state)
{
	ID = state.ID;
	CurrentClock = state.CurrentClock;	
	DevicesClock = state.DevicesClock;	
	bLatchA = state.bLatchA != 0;
	bLatchB = state.bLatchB != 0;
	ora = state.ora;
	ira = state.ira;
	orb = state.orb;
	irb = state.irb;
	ddra = state.ddra;
	ddrb = state.ddrb;
	timer1_counter = state.timer1_counter;
	timer2_counter = state.timer2_counter;
	timer1_latch = state.timer1_latch;
	timer2_latch = state.timer2_latch;
	acr = state.acr;
	pcr = state.pcr;
	ca1_in = state.ca1_in != 0;
	ca1_in_prev = state.ca1_in_prev != 0;
	ca2_in = state.ca2_in != 0;
	ca2_in_prev = state.ca2_in_prev != 0;
	cb1_in = state.cb1_in != 0;
	cb1_in_prev = state.cb1_in_prev != 0;
	cb2_in = state.cb2_in != 0;
	cb2_in_prev = state.cb2_in_prev != 0;
	ca2_out = state.ca2_out != 0;
	cb2_out = state.cb2_out != 0;
	shiftRegisterData = state.shiftRegisterData;
	ifr = state.ifr;
	ier = state.ier;
	shiftRegisterMode = state.shiftRegisterMode;
	shiftClockLevel = state.shiftClockLevel != 0;
	delay = state.delay;
	feed = state.feed;
	old_delay = state.old_delay;
	old_feed = state.old_feed;
	shiftCounter = state.shiftCounter;
	Interrupt = state.Interrupt;
	bPB7TimerMode = state.bPB7TimerMode;
	bPB7Toggle = state.bPB7Toggle;
	bPB7TimerOut = state.bPB7TimerOut;
	no_change_count = state.no_change_count;
	dec_2 = state.dec_2;
	idle = state.idle;
}
