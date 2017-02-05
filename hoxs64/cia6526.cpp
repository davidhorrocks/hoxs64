#include <windows.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "savestate.h"
#include "register.h"
#include "cia6526.h"

#define NO_CHANGE_TO_IDLE 4
#define MINIMUM_STAY_IDLE 3
#define MINIMUM_GO_IDLE_TIME 5

int CIA::prec_bitcount[256];

CIA::CIA()
{
	CurrentClock=0;
	bEarlyIRQ = true;
	bTimerBbug = false;
	init_bitcount();
	ID = 0;
}

void CIA::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock=sysclock;
	DevicesClock=sysclock;
	ClockNextWakeUpClock=sysclock;
	ClockNextTODWakeUpClock=sysclock;
	serial_int_count=0;
	idle=0;
	delay=0 | CountA3 | CountB3;
	feed=0;
	old_delay=0;
	old_feed=0;
	no_change_count=0;

	pra_out=0;
	prb_out=0;
	ddra=0;
	ddrb=0;
	tod_read_freeze=0;
	tod_write_freeze=1;
	tod_clock_rate=TODDIVIDER60;
	tod_clock_reload=TODRELOAD60;
	tod_tick=0;
	tod_clock_compare_band=TODCLOCKCOMPAREBAND60;
	tod.byte.ths=0;
	tod.byte.sec=0;
	tod.byte.min=0;
	tod.byte.hr=1;
	tod_read_latch.byte.ths=0;
	tod_read_latch.byte.sec=0;
	tod_read_latch.byte.min=0;
	tod_read_latch.byte.hr=1;
	tod_alarm=0;	
	alarm.byte.ths=0;
	alarm.byte.sec=0;
	alarm.byte.min=0;
	alarm.byte.hr=0;
	sdr=0;
	cra=0;
	crb=0;
	ta_latch.word=0xffff;
	tb_latch.word=0xffff;
	ta_counter.word=0;
	tb_counter.word=0;
	f_cnt_in=f_cnt_out=0;
	f_flag_in=f_flag_out=0;
	flag_change=0;
	timera_output=0;
	timerb_output=0;

	bPB67Toggle=0;
	bPB67TimerMode=0;
	bPB67TimerOut=0;

	icr=0;
	icr_ack=0;
	fast_clear_pending_int = false;
	imr=0;

	Interrupt=0;
}

void CIA::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	ClearSystemInterrupt();

	WriteRegister(2, CurrentClock, 0);
	WriteRegister(3, CurrentClock, 0);
}

void CIA::SetMode(HCFG::CIAMODE mode, bool bTimerBbug)
{
	switch (mode)
	{
		case HCFG::CM_CIA6526:
			bEarlyIRQ = false;
			break;
		case HCFG::CM_CIA6526A:
			bEarlyIRQ = true;
			break;
		default:
			bEarlyIRQ = true;
	}
	this->bTimerBbug = bTimerBbug;
}

void CIA::PreventClockOverflow()
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;

	if ((ICLKS)(CurrentClock - ClockReadICR) >= CLOCKSYNCBAND_FAR)
		ClockReadICR = ClockBehindNear;
}

ICLK CIA::GetCurrentClock()
{
	return CurrentClock;
}

void CIA::SetCurrentClock(ICLK sysclock)
{
ICLK v = sysclock - CurrentClock;
	CurrentClock+=v;
	ClockReadICR+=v;
}

//pragma optimize( "ag", off )

void CIA::incrementTOD()
{
bit32 *p;
	p=(bit32 *)&tod.byte.ths;
	bit8 ah,al,pm;

	/*update tenths*/
	al = tod.byte.ths = (tod.byte.ths + 1) & 0xf;
	if (al!=0xa)
	{
		return;
	}
	tod.byte.ths = 0;

	/*update seconds*/
	al = (tod.byte.sec) & 0xf;
	ah = (tod.byte.sec) & 0x70;
	al = al + 1;
	if (al == 0xa)
	{
		al = 0;
		ah = (ah + 0x10);
	}
	if (ah != 0x60)
	{
		tod.byte.sec = (ah & 0x70) | (al & 0xf);
		return;
	}
	tod.byte.sec=0;

	/*update minutes*/
	al = (tod.byte.min) & 0xf;
	ah = (tod.byte.min) & 0x70;
	al = al + 1;
	if (al == 0xa)
	{
		al = 0;
		ah = (ah + 0x10);
	}
	if (ah != 0x60)
	{
		tod.byte.min = (ah & 0x70) | (al & 0xf);
		return;
	}
	tod.byte.min = 0;

	/*update hours*/
	pm = tod.byte.hr & 0x80;
	ah = tod.byte.hr & 0x1f;
	if (ah == 0x12)
	{
		ah = 0;
	}
	al = ah & 0x10;
	ah = (ah + 1) & 0x1f;
	if (ah == 0x12)
	{
		pm = pm ^ 0x80;
	}
	if (ah == 0x0a)
	{
		tod.byte.hr = pm | 0x10;
	}
	else
	{
		tod.byte.hr = pm | (al & 0x10)| (ah & 0xf);
	}
}

void CIA::ExecuteCycle(ICLK sysclock)
{
bit8 new_icr;
ICLKS clocks;
ICLKS fastClocks;
ICLKS fastTODClocks;
ICLKS todclocks;

	if ((ICLKS)(DevicesClock - sysclock) < 0)
	{
		DevicesClock = sysclock;
		ExecuteDevices(sysclock);
	}

	clocks = (ICLKS)(sysclock - CurrentClock);

	fastTODClocks = (ICLKS)(ClockNextTODWakeUpClock - CurrentClock);
	fastClocks = (ICLKS)(ClockNextWakeUpClock - CurrentClock);
	if (fastClocks > 0 && fastTODClocks > 0 && clocks > 0)
	{
		if (fastClocks > clocks)
		{
			fastClocks = clocks;
		}
		if (fastClocks > fastTODClocks)
		{
			fastClocks = fastTODClocks;
		}
		ta_counter.word = ta_counter.word - dec_a * (bit16)fastClocks;
		tb_counter.word = tb_counter.word - dec_b * (bit16)fastClocks;

		todclocks = ((fastClocks * tod_clock_rate) + tod_tick) / tod_clock_reload;
		tod_tick = ((fastClocks * tod_clock_rate) + tod_tick) % tod_clock_reload;
		if(tod_write_freeze==0)
		{
			for (ICLKS i = 0 ; i < todclocks; i++)
			{
				incrementTOD();
			}
		}
			
		CurrentClock += (ICLK)fastClocks;
		clocks -= fastClocks;
	}

	while (clocks-- > 0)
	{
		CurrentClock++;

		todclocks = ((tod_clock_rate) + tod_tick) / tod_clock_reload;
		tod_tick = ((tod_clock_rate) + tod_tick) % tod_clock_reload;
		if(tod_write_freeze==0 && todclocks !=0)
		{
			cia_tod prevtime;
			prevtime.dword = tod.dword;
			incrementTOD();
			if (prevtime.dword != tod.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}

		if (idle)
		{
			ta_counter.word -= dec_a;
			tb_counter.word -= dec_b;
			if ((ta_counter.word < MINIMUM_STAY_IDLE && dec_a!=0) || (tb_counter.word < MINIMUM_STAY_IDLE && dec_b!=0))
			{
				idle=0;
				no_change_count=0;
			}
			continue;
		}
		ClockNextWakeUpClock = sysclock;

		//TIMER A *************************************
		//bit passed through pipe to signal timer 1 to decrement
		old_delay = delay;
		old_feed = feed;
		new_icr=0;

		if (delay & CountA3)
		{
			ta_counter.word--;
		}
		timera_output= (ta_counter.word==0) && ((delay & CountA2)!=0);
		if (timera_output)
		{
			new_icr|=1;
 			delay|=LoadA1;
			// check for one shot.	
			if ((delay | feed) & OneShotA0)
			{
				cra = (cra & 0xFE);
				feed &= ~CountA2;
			}	
			if (cra & 0x40)//Generate serial interrupt after 16 timer a underflows.
			{
				if (serial_int_count)
				{
					serial_int_count--;
					if ((cra & 8) == 0)
					{
						if ((serial_int_count & 0xf) == 1)
						{
							new_icr|=8;
						}
					}
					else
					{
						if ((serial_int_count & 0xf) == 0)
						{
							new_icr|=8;
						}
					}
				}
			}

			// toggle underflow counter bit
			bPB67Toggle ^= 0x40;
			if ((cra & 0x02) != 0)
			{
				// timer A output to PB6
				if ((cra & 0x04) == 0)
				{
					// set PB6 high for one clock
					bPB67TimerOut |= 0x40;
					delay |= PB6Low0;
					delay &= ~PB6Low1;
				} 
				else
				{
					// toggle PB6 between high and low
					bPB67TimerOut ^= 0x40;//should match bPB67Toggle
				}
			}
		}


		if ((delay & LoadA1)!=0)
		{
			ta_counter.word = ta_latch.word;
			// simulate the loss of a timer tick on the loading of the latch.
			delay = delay & ~(CountA2);  
		}

		if ((cra & 0x20)==0)
		{
			//02 mode
			delay = delay & ~(CountA1);
		}
		else
		{
			//CNT mode
			if ((cra & 1)==0)
			{
				delay = delay & ~(CountA1);  
			}
		}


		//TIMER B *************************************
		//bit passed through pipe to signal timer 1 to decrement
		if (delay & CountB3)
		{
			tb_counter.word--;
		}
		timerb_output= (tb_counter.word==0) && ((delay & CountB2)!=0);
		if (timerb_output)
		{
			new_icr |= 2;

 			delay |= LoadB1;
			// check for one shot.	
			if ((delay | feed) & OneShotB0)
			{
				crb = (crb & 0xFE);
				feed &= ~CountB2;
			}

			// toggle underflow counter bit
			bPB67Toggle ^= 0x80;
			if ((crb & 0x02) != 0)
			{
				// timer B output to PB7
				if ((crb & 0x04) == 0)
				{
					// set PB7 high for one clock
					bPB67TimerOut |= 0x80;
					delay |= PB7Low0;
					delay &= ~PB7Low1;
				} 
				else
				{
					// toggle PB7 between high and low
					bPB67TimerOut ^= 0x80;//should match bPB67Toggle
				}
			}
		}

		if ((delay & LoadB1)!=0)
		{
			tb_counter.word = tb_latch.word;
			// simulate the loss of a timer tick on the loading of the latch.
			delay = delay & ~(CountB2);  
		}
		

		if ((crb & 0x60)==0)
		{
			//02 mode
			delay = delay & ~(CountB1);
		}
		else if ((crb & 0x60)==0x20)
		{
			//CNT mode
			if ((crb & 1)==0)
			{
				delay = delay & ~(CountB1);  
			}
		}
		else if ((crb & 0x60)==0x40)
		{
			//Timer A underflow.
			if (timera_output)
			{
				if (crb & 1)
				{
					delay |= CountB1;
				}
				else
				{
					delay &= ~CountB1;
				}
			}
			else
			{
				delay &= ~CountB1;
			}
		}
		else if ((crb & 0x60)==0x60)
		{
			if (timera_output)
			{
				if (crb & 1)
				{
					delay |= CountB1;
				}
				else
				{
					delay &= ~CountB1;
				}
			}
			else
			{
				delay &= ~CountB1;
			}
		}

		if ((delay & (PB6Low1 | PB7Low1)) != 0)
		{
			if ((delay & PB6Low1) != 0)
			{
				bPB67TimerOut &= ~0x40;
			}
			if ((delay & PB7Low1) != 0)
			{
				bPB67TimerOut &= ~0x80;
			}
		}

		new_icr|= ((!f_flag_in & flag_change)<<4);
		if(tod_alarm)
		{
			new_icr|= tod_alarm;
			tod_alarm=0;
		}

		icr_ack = icr_ack & ~new_icr;
		if ((delay & ClearIcr1) !=0)
		{
			icr = icr & ~icr_ack;
			icr_ack = 0;
		}
		if (bTimerBbug && (new_icr & 2) !=0 && ClockReadICR == CurrentClock)
		{
			icr |= (new_icr & ~2);
		}
		else
		{
			icr |= new_icr;
		}
		if (new_icr & imr & 0x1F) 
		{ 
			if (bEarlyIRQ)
			{
				//new CIA
				//if (ClockReadICR == CurrentClock)
				if ((delay & ReadIcr0) != 0)
				{
					//TEST TLR's CIA read DC0D (icr) when Timer counts to zero;
					//Used by testprogs\interrupts\irqnmi\cia-int-irq-new.prg
					//Used by testprogs\interrupts\irqnmi\cia-int-nmi-new.prg
					delay|=Interrupt0;
					delay|=SetIcr0;
				}
				else
				{
					delay|=(Interrupt1 | Interrupt1);
					delay|=(SetIcr1 | SetIcr1);
				}
			}
			else
			{
				//Old CIA
				delay|=Interrupt0;
				delay|=SetIcr0;
			}
		}
		
		// Check for interrupt condition
		if ((delay & SetIcr1) != 0)
		{
			icr |= 0x80;
		}
		if (delay & Interrupt1)
		{
			Interrupt = 1;
			SetSystemInterrupt();
		}

		delay=((delay << 1) & DelayMask) | feed;
		if (f_cnt_in)
		{
			delay |= (CountA0 | CountB0);
		}
		flag_change=0;
		
		if (delay==old_delay && feed==old_feed)
		{
			if ((((cra & 1)!=0) && ta_counter.word<MINIMUM_GO_IDLE_TIME) || (((crb & 1)!=0) && tb_counter.word<MINIMUM_GO_IDLE_TIME))
			{
				no_change_count=0;
			}
			else
			{
				no_change_count++;
				if (no_change_count>NO_CHANGE_TO_IDLE)
				{
					idle=1;
					dec_a=(bit16)(delay & CountA3) >> bitCountA3;
					dec_b=(bit16)(delay & CountB3) >> bitCountB3;
				}
			}
		}
		else
		{
			no_change_count=0;
		}
	}
	SetTODWakeUpClock();
	SetWakeUpClock();
}

void CIA::SetWakeUpClock()
{
	ICLK testCount, currentCount, curClock;
	currentCount = PALCLOCKSPERSECOND;
	curClock = CurrentClock;
	if (idle==0)
	{
		ClockNextWakeUpClock = curClock;
	}
	else
	{
		if ((cra & 1)!=0)
		{
			testCount = ta_counter.word - MINIMUM_GO_IDLE_TIME;
			if ((ICLKS)(testCount) <= 0)
			{
				currentCount = 0;
			}
			else if ((ICLKS)(testCount - currentCount) < 0)
			{
				currentCount = testCount;
			}
		}
		if ((crb & 0x61)==0x1)
		{
			testCount = tb_counter.word - MINIMUM_GO_IDLE_TIME;
			if ((ICLKS)(testCount) <= 0)
			{
				currentCount = 0;
			}
			else if ((ICLKS)(testCount - currentCount) < 0)
			{
				currentCount = testCount;
			}
		}
		if (tod_write_freeze==0)
		{
			testCount = ClockNextTODWakeUpClock - curClock;
			if ((ICLKS)(testCount) <= 0)
			{
				currentCount = 0;
			}
			else if (((ICLKS)(testCount - currentCount) < 0))
			{
				currentCount = testCount;
			}
			currentCount = 0;
		}
		if ((ICLKS)currentCount < 0)
		{
			currentCount=0;
		}

		ClockNextWakeUpClock = curClock + currentCount;
	}
}

void CIA::Pulse(ICLK sysclock)
{
	ExecuteCycle(sysclock);
	f_flag_in=0;
	flag_change=1;
	idle=0;
	no_change_count=0;
}

//pragma optimize( "ag", on )

bit8 CIA::ReadRegister(bit16 address, ICLK sysclock)
{
	ExecuteCycle(sysclock);

	switch(address & 0x0F)
	{
	case 0x00:		// port a
		return ReadPortA();
	case 0x01:		// port b
		return ReadPortB();
	case 0x02:		// data direction a
		return ddra;
	case 0x03:		// data direction b
		return ddrb;
	case 0x04:		// timer a lo
		return ta_counter.byte.loByte;
	case 0x05:		// timer a hi
		return ta_counter.byte.hiByte;
	case 0x06:		// timer b lo
		return tb_counter.byte.loByte;
	case 0x07:		// timer b hi
		return tb_counter.byte.hiByte;
	case 0x08://TOD
		if (tod_read_freeze)
		{
			tod_read_freeze = 0;
			return tod_read_latch.byte.ths;
		}
		else
		{
			return tod.byte.ths;
		}
	case 0x09:
		if (tod_read_freeze)
		{
			return tod_read_latch.byte.sec;
		}
		else
		{
			return tod.byte.sec;
		}
	case 0x0A:
		if (tod_read_freeze)
		{
			return tod_read_latch.byte.min;
		}
		else
		{
			return tod.byte.min;
		}
	case 0x0B:
		if (tod_read_freeze)
		{
			return tod_read_latch.byte.hr;
		}
		else
		{
			tod_read_freeze =1;
			tod_read_latch.dword = tod.dword;
			return tod.byte.hr;
		}
	case 0x0C://serial data register
		return sdr;
	case 0x0D:		// interrupt control register
		ClockReadICR = sysclock + 1;
		if (bEarlyIRQ)
		{
			if ((delay & Interrupt1) != 0)
			{
				if ((icr & 0x1f) != 0)
				{
					icr |= 0x80;
				}
			}

			if ((icr & 0x9F) != 0)
			{
				icr_ack |= ((icr & 0x9F) | 0x80);
			}
			delay |= ClearIcr0;
			delay &= ~(Interrupt0 | Interrupt1);
			delay &= ~(SetIcr0 | SetIcr1);
		}
		else
		{
			if ((icr & 0x9F) != 0)
			{
				icr_ack |= ((icr & 0x9F) | 0x80);
			}
			if (fast_clear_pending_int)
			{
				if ((delay & SetIcr1) == 0)
				{
					fast_clear_pending_int = false;
				}
				delay |= ClearIcr0;
				delay &= ~(Interrupt0 | Interrupt1);
			}
			else
			{
				if ((delay & SetIcr1) != 0)
				{
					delay |= ClearIcr0;
					fast_clear_pending_int = true;
				}			
				else
				{
					delay |= ClearIcr1;
				}
				delay &= ~(Interrupt0 | Interrupt1);
				delay &= ~(SetIcr0 | SetIcr1);
			}
		}		
		delay |= ReadIcr0;
		Interrupt = 0;
		ClearSystemInterrupt();
		idle = 0;
		no_change_count = 0;
		SetWakeUpClock();
		return icr;
	case 0x0E:		// control register a
		return cra & ~0x10;
	case 0x0F:		// control register b
		return crb & ~0x10;
	}
	return 0;
}

bit8 CIA::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
bit8 t;
	ExecuteCycle(sysclock);

	switch(address & 0x0F)
	{
	case 0x08:
		if (tod_read_freeze)
		{
			return tod_read_latch.byte.ths;
		}
		else
		{
			return tod.byte.ths;
		}
	case 0x0B:
		return tod_read_latch.byte.hr;
	case 0x0D:		// interrupt control register
		t=icr;
		if (bEarlyIRQ)
		{
			if ((delay & Interrupt1) != 0)
			{
				if ((t & 0x1f) != 0)
				{
					t |= 0x80;
				}
			}
		}
		return t;
	default:
		return ReadRegister(address, sysclock);
	}
}

void CIA::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
cia_tod prevtime;
bit8 old_imr;
	ExecuteCycle(sysclock);
	switch(address & 0x0F)
	{
	case 0x00:		// port a
		pra_out = data;
		WritePortA();
		break;
	case 0x01:		// port b
		prb_out=data;
		WritePortB();
		break;
	case 0x02:		// data direction a
		ddra = data;
		WritePortA();
		break;
	case 0x03:		// data direction b
		ddrb = data;
		WritePortB();
		break;
	case 0x04:		// timer a lo
		ta_latch.byte.loByte=data;
		if (delay & LoadA2)
		{
			ta_counter.word = ta_latch.word;
		}
		break;
	case 0x05:		// timer a hi
		ta_latch.byte.hiByte=data;
		if ((cra & 0x01) == 0)
		{
			delay|=LoadA0;
		}
		if (delay & LoadA2)
		{
			ta_counter.word = ta_latch.word;
		}
		idle=0;
		no_change_count=0;
		break;
	case 0x06:		// timer b lo
		tb_latch.byte.loByte=data;
		if (delay & LoadB2)
		{
			tb_counter.word = tb_latch.word;
		}
		break;
	case 0x07:		// timer b hi
		tb_latch.byte.hiByte=data;
		if ((crb & 0x01) == 0)
		{
			delay|=LoadB0;
		}
		if (delay & LoadB2)
		{
			tb_counter.word = tb_latch.word;
		}
		idle=0;
		no_change_count=0;
		break;
	case 0x08:	//TOD
		if (crb & 0x80)
		{
			prevtime.dword = alarm.dword;
			alarm.byte.ths=data & 0xf;
			if (prevtime.dword != alarm.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		else
		{
			prevtime.dword = tod.dword;
			if (tod_write_freeze)
			{
				tod_write_freeze=0;
				tod_tick = 0;
			}
			tod.byte.ths=data & 0xf;
			if (prevtime.dword != tod.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		SetTODWakeUpClock();
		break;
	case 0x09:
		if (crb & 0x80)
		{
			prevtime.dword = alarm.dword;
			alarm.byte.sec=data & 0x7f;
			if (prevtime.dword != alarm.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		else
		{
			prevtime.dword = tod.dword;
			tod.byte.sec=data & 0x7f;
			if (prevtime.dword != tod.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		SetTODWakeUpClock();
		break;
	case 0x0A:
		if (crb & 0x80)
		{
			prevtime.dword = alarm.dword;
			alarm.byte.min=data & 0x7f;
			if (prevtime.dword != alarm.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		else
		{
			prevtime.dword = tod.dword;
			tod.byte.min=data & 0x7f;
			if (prevtime.dword != tod.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		SetTODWakeUpClock();
		break;
	case 0x0B:
		if (crb & 0x80)
		{
			prevtime.dword = alarm.dword;
			alarm.byte.hr=data & 0x9f;
			if (prevtime.dword != alarm.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		else
		{
			prevtime.dword = tod.dword;
			tod_write_freeze=1;
			if ((data & 0x1f) == 0x12)
			{
				if (data & 0x80)
				{
					tod.byte.hr = 0x12;
				}
				else
				{
					tod.byte.hr = 0x92;
				}
			}
			else
			{
				tod.byte.hr = data & 0x9f;
			}
			if (prevtime.dword != tod.dword)
			{
				CheckTODAlarmCompare(sysclock);
			}
		}
		SetTODWakeUpClock();
		break;
	case 0x0C://serial data register
		sdr=data;
		if (serial_int_count < 16)
		{
			serial_int_count+=16;
		}
		idle=0;
		no_change_count=0;
		break;
	case 0x0D:		// interrupt control register
		old_imr = imr; 
		if (data & 0x80)
		{
			imr|=(data & 0x1F);
		}
		else
		{
			imr&=(~data);
		}
		if ((icr & imr & 0x1F) != 0)
		{
			//Both pending interrupts and currently active interrupts are never cancelled or cleared.
			if (Interrupt == 0)
			{
				if (this->bEarlyIRQ)
				{
					if ((delay & ReadIcr1) == 0)
					{
						delay |= Interrupt1;
						delay |= SetIcr1;
					}
				}
				else
				{
					delay |= Interrupt0;
					delay |= SetIcr0;
				}
			}
		}
		else
		{
			if (!this->bEarlyIRQ)
			{
				//This relates to "old" CIA 6526.
				//Currently active interrupts are never cleared.
				//But the dd0dtest.prg test shows that a pending interrupt may be cancelled.

				//Not sure if this is correct.
				//Allow a pending interrupt to be cancelled if a write to $DD0D had also occurred in the previous clock cycle.
				if ((icr & old_imr) !=0 && (delay & WriteIcr1) !=0)
				{
					/*
					The test testprogs\CIA\dd0dtest\dd0dtest.prg test number $11 requires that a pending Timer A interrupt is cancelled with a write to ICR ($DD0D in this case).
					The entry conditions seen where that: X==0 and ICR == $00. ICR is what is read from $DD0D.
					Then the CPU executes: INC $DD0D,X
					Timer A rundowns at cycle 7 of INC $DD0D,X
					$DD0D is thought to be subjected to Read($00; Cycle4) Read($00; Cycle5) Write($00; Cycle6) Write($01; Cycle7).
					A system NMI is not generated.

					On the contrary, the test testprogs\interrupts\cia-int\cia-int-nmi.prg requires that a pending Timer B interrupt is not cleared by writing to ICR.
					The conditions seen where that the processor executes:
					LDX #$7F
					STX $DD0D
					Timer B rundowns at cycle 4 of STX $DD0D which is the write of $7F to $DD0D.
					A system NMI is generated.
					*/
					delay &= ~(Interrupt1);
					delay &= ~(SetIcr1);
				}
			}
		}
		delay |= WriteIcr0;
		idle=0;
		no_change_count=0;
		break;
	case 0x0E:		// control register a
		// force load check
		if (data & 0x10)
		{
			delay|= LoadA0;	
		}

		if (data & 0x08)
		{
			feed |= OneShotA0;	
		}
		else
		{
			feed &= ~OneShotA0;
		}

		//CNT or 02 mode
		if (data & 0x20)
		{
			//CNT
			feed &= ~CountA2;
		}
		else
		{
			//02
			if (data & 0x1) 
			{
				feed |= CountA2;
			}
			else
			{
				feed &= ~CountA2;
			}
		}
		
		//Set PB6 high on Timer A start
		if ((data & 0x01) != 0 && (cra & 0x01) == 0)
		{
			bPB67Toggle |= 0x40;
		}

		if ((data & 0x02) == 0)//timer A to PB6 disabled
		{
			bPB67TimerMode &= ~0x40;
		}
		else//timer A to PB6 enabled
		{
			bPB67TimerMode |= 0x40;
			if ((data & 0x04) == 0)//PB6 pulse mode
			{
				if ((delay & PB6Low1) == 0)
				{
					bPB67TimerOut &= ~0x40;
				}
				else
				{
					bPB67TimerOut |= 0x40;
				}
			} 
			else//PB6 toggle mode
			{
				bPB67TimerOut = (bPB67TimerOut & ~0x40) | (bPB67Toggle & 0x40);
			}
		}

		if ((data ^ cra) & 0x80)
		{
			if (data & 0x80)
			{
				tod_clock_reload = TODRELOAD50;
				tod_clock_rate = TODDIVIDER50;
				tod_clock_compare_band = TODCLOCKCOMPAREBAND50;
				tod_tick = tod_tick / (TODDIVIDER60/TODDIVIDER50);
			}
			else
			{
				tod_clock_reload = TODRELOAD60;
				tod_clock_rate = TODDIVIDER60;
				tod_clock_compare_band = TODCLOCKCOMPAREBAND60;
				tod_tick = tod_tick * (TODDIVIDER60/TODDIVIDER50);
			}
		}

		cra=data & 0xEF;

		idle=0;
		no_change_count=0;
		break;
	case 0x0F:		// control register b
		// force load check
		if (data & 0x10)
		{
			delay|= LoadB0;	
		}

		if (data & 0x08)
		{
			feed |= OneShotB0;	
		}
		else
		{
			feed &= ~OneShotB0;
		}

		//CNT or 02 mode
		if ((data & 0x60)==0)
		{
			//02
			if (data & 0x1) 
			{
				feed |= CountB2;
			}
			else
			{
				feed &= ~CountB2;
			}
		}
		else if ((data & 0x60)==0x20)
		{
			//CNT
			feed &= ~CountB2;
		}
		else if ((data & 0x60)==0x40)
		{
			//TA
			feed &= ~CountB2;
		}
		else if ((data & 0x60)==0x40)
		{
			//TA && CNT
			feed &= ~CountB2;
		}

		//Set PB7 high on Timer B start
		if ((data & 0x01) != 0 && (crb & 0x01) == 0)
		{
			bPB67Toggle |= 0x80;
		}

		if ((data & 0x02) == 0)//timer B to PB7 disabled
		{
			bPB67TimerMode &= ~0x80;
		}
		else//timer B to PB7 enabled
		{
			bPB67TimerMode |= 0x80;
			if ((data & 0x04) == 0)//PB7 pulse mode
			{
				if ((delay & PB7Low1) == 0)
				{
					bPB67TimerOut &= ~0x80;
				}
				else
				{
					bPB67TimerOut |= 0x80;
				}
			} 
			else//PB7 toggle mode
			{
				bPB67TimerOut = (bPB67TimerOut & ~0x80) | (bPB67Toggle & 0x80);
			}
		}

		crb=data & 0xEF;

		idle=0;
		no_change_count=0;
		break;
	}

	SetWakeUpClock();
}

bit8 CIA::PortAOutput_Strong0s()
{
	return (pra_out | ~ddra);
}

bit8 CIA::PortBOutput_Strong0s()
{
	return ((prb_out | ~ddrb) & ~bPB67TimerMode) | (bPB67TimerOut & bPB67TimerMode);
}

bit8 CIA::PortAOutput_Strong1s()
{
	return (pra_out & ddra);
}

bit8 CIA::PortBOutput_Strong1s()
{
	return ((prb_out & ddrb) & ~bPB67TimerMode) | (bPB67TimerOut & bPB67TimerMode);

}

void CIA::CheckTODAlarmCompare(ICLK sysclock)
{
	if (tod.dword == alarm.dword)
	{
		tod_alarm=4;
		idle=0;
		no_change_count=0;
		ClockNextTODWakeUpClock = sysclock + 0x1000000;
	}
}

void CIA::SetTODWakeUpClock()
{
long t;
	if (tod_write_freeze == 0)
	{
		t = GetTenthsFromTimeToAlarm((const cia_tod &)tod, (const cia_tod &)alarm);
		if (t<0)
		{
			ClockNextTODWakeUpClock = CurrentClock + 0x1000000;
		}
		else if (t <= 2)
		{
			ClockNextTODWakeUpClock = CurrentClock;
		}
		else
		{
			ClockNextTODWakeUpClock = CurrentClock + (__int32)((__int64)(t - 1) * (__int64)tod_clock_reload / (__int64)tod_clock_rate);
		}
	}
	else
	{
		ClockNextTODWakeUpClock = CurrentClock + 0xA000000;
	}
}

long CIA::GetTenthsFromTimeToAlarm(const cia_tod &time, const cia_tod &alarm)
{
long a, t, p;
long ths = 0;
long borrow;

	a = (char)(alarm.byte.ths & 0xF);
	t = (char)(time.byte.ths & 0xF);
	
	borrow = 0;
	if (a > 9)
	{
		if (t > a || t < 0xA)
		{
			return -1;
		}
		else
		{
			if ((alarm.dword & 0xffffff00) == (time.dword & 0xffffff00))
			{
				return p = a - t;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		if (t > 9)
		{
			p = a + (0x10 - t);
		}
		else
		{
			if (a >= t)
			{
				p = a - t;
			}
			else
			{
				p = 10 + a - t;
				borrow = 1;
			}
		}
	}
	ths = p;

	a = (char)(alarm.byte.sec & 0xF);
	t = (char)(time.byte.sec & 0xF);
	if (borrow)
	{
		borrow = 0;
		if (t==9)
		{
			t=0;
		}
		else
		{
			t = (t + 1) & 0xF;
		}
	}

	if (a > 9)
	{
		if (t > a || t < 0xA)
		{
			return -1;
		}
		else
		{
			if ((alarm.dword & 0xffff0000) == (time.dword & 0xffff0000))
			{
				return p = a - t;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		if (t > 9)
		{
			p = a + (0x10 - t);
		}
		else
		{
			if (a >= t)
			{
				p = a - t;
			}
			else
			{
				p = 10 + a - t;
				borrow = 1;
			}
		}
	}
	ths = ths + p * 10;

	return ths;
}

void CIA::init_bitcount()
{
	for (int i = 0; i <= 255; i++)
	{
		int j,v,c;
		for (j = 0,v = i,c = 0; j < 8; j++, v>>=1)
		{
			if (v & 1)
			{
				c++;
			}
		}
		prec_bitcount[i] = c;
	}
}

void CIA::GetState(SsCiaV1 &state)
{
	ZeroMemory(&state, sizeof(state));
	state.CurrentClock = CurrentClock;
	state.DevicesClock = DevicesClock;
	state.ClockNextWakeUpClock = ClockNextWakeUpClock;
	state.ClockNextTODWakeUpClock = ClockNextTODWakeUpClock;
	state.delay = delay;
	state.feed = feed;
	state.old_delay = old_delay;
	state.old_feed = old_feed;
	state.idle = idle;
	state.dec_a = dec_a;
	state.dec_b = dec_b;
	state.no_change_count = no_change_count;
	state.flag_change = flag_change;
	state.sp_change = sp_change;
	state.f_flag_in = f_flag_in;
	state.f_flag_out = f_flag_out;
	state.f_sp_in = f_sp_in;
	state.f_sp_out = f_sp_out;
	state.f_cnt_in = f_cnt_in;
	state.f_cnt_out = f_cnt_out;
	state.pra_out = pra_out;
	state.prb_out = prb_out;
	state.ddra = ddra;
	state.ddrb = ddrb;
	state.ta_counter = ta_counter;
	state.tb_counter = tb_counter;
	state.ta_latch = ta_latch;
	state.tb_latch = tb_latch;
	state.tod_clock_reload = tod_clock_reload;
	state.tod_clock_rate = tod_clock_rate;
	state.tod_tick = tod_tick;
	state.tod_clock_compare_band = tod_clock_compare_band;
	state.tod_alarm = tod_alarm;
	state.tod_read_freeze = tod_read_freeze;	
	state.tod_read_latch.dword = tod_read_latch.dword;
	state.tod_write_freeze = tod_write_freeze;
	state.tod_write_latch.dword = tod_write_latch.dword;
	state.tod.dword = tod.dword;
	state.alarm.dword = alarm.dword;
	state.sdr = sdr;
	state.cra = cra;
	state.crb = crb;
	state.timera_output = timera_output;
	state.timerb_output = timerb_output;
	state.icr = icr;
	state.icr_ack = icr_ack;	
	state.imr = imr;
	state.Interrupt = Interrupt;
	state.serial_int_count = serial_int_count;
	state.bEarlyIRQ = bEarlyIRQ;
	state.bTimerBbug = bTimerBbug;
	state.ClockReadICR = ClockReadICR;
	state.bPB67TimerMode = bPB67TimerMode;
	state.bPB67TimerOut = bPB67TimerOut;
	state.bPB67Toggle = bPB67Toggle;
}

void CIA::SetState(const SsCiaV1 &state)
{
	CurrentClock = state.CurrentClock;
	DevicesClock = state.DevicesClock;
	ClockNextWakeUpClock = state.ClockNextWakeUpClock;
	ClockNextTODWakeUpClock = state.ClockNextTODWakeUpClock;
	delay = state.delay;
	feed = state.feed;
	old_delay = state.old_delay;
	old_feed = state.old_feed;
	idle = state.idle;
	dec_a = state.dec_a;
	dec_b = state.dec_b;
	no_change_count = state.no_change_count;
	flag_change = state.flag_change;
	sp_change = state.sp_change;
	f_flag_in = state.f_flag_in;
	f_flag_out = state.f_flag_out;
	f_sp_in = state.f_sp_in;
	f_sp_out = state.f_sp_out;
	f_cnt_in = state.f_cnt_in;
	f_cnt_out = state.f_cnt_out;
	pra_out = state.pra_out;
	prb_out = state.prb_out;
	ddra = state.ddra;
	ddrb = state.ddrb;
	ta_counter = state.ta_counter;
	tb_counter = state.tb_counter;
	ta_latch = state.ta_latch;
	tb_latch = state.tb_latch;
	tod_clock_reload = state.tod_clock_reload;
	tod_clock_rate = state.tod_clock_rate;
	tod_tick = state.tod_tick;
	tod_clock_compare_band = state.tod_clock_compare_band;
	tod_alarm = state.tod_alarm;
	tod_read_freeze = state.tod_read_freeze;	
	tod_read_latch.dword = state.tod_read_latch.dword;
	tod_write_freeze = state.tod_write_freeze;
	tod_write_latch.dword = state.tod_write_latch.dword;
	tod.dword = state.tod.dword;
	alarm.dword = state.alarm.dword;
	sdr = state.sdr;
	cra = state.cra;
	crb = state.crb;
	timera_output = state.timera_output;
	timerb_output = state.timerb_output;
	icr = state.icr;
	icr_ack = state.icr_ack;
	imr = state.imr;
	Interrupt = state.Interrupt;
	serial_int_count = state.serial_int_count;
	bEarlyIRQ = state.bEarlyIRQ != 0;
	bTimerBbug = state.bTimerBbug != 0;
	ClockReadICR = state.ClockReadICR;
	bPB67TimerMode = state.bPB67TimerMode;
	bPB67TimerOut = state.bPB67TimerOut;
	bPB67Toggle = state.bPB67Toggle;
}

void CIA::UpgradeStateV0ToV1(const SsCiaV0 &in, SsCiaV1 &out)
{
	ZeroMemory(&out, sizeof(SsCiaV1));
	*((SsCiaV0 *)&out) = in;
	if (out.ClockReadICR == out.CurrentClock + 1)
	{
		out.delay |= ClearIcr0;
		out.icr_ack = in.icr;
	}
	else if (out.ClockReadICR == out.CurrentClock + 2)
	{
		out.delay |= ClearIcr1;
		out.icr_ack = in.icr;
	}
}
