#include <windows.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
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
}

void CIA::Reset(ICLK sysclock)
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
	tod.byte.hr=0;
	tod_alarm=0;
	alarm.byte.ths=0;
	alarm.byte.sec=0;
	alarm.byte.min=0;
	alarm.byte.hr=0;
	sdr=0;
	icr=0;
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
	imr=0;

	Interrupt=0;
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
	const ICLKS CLOCKSYNCBAND_NEAR = 0x4000;
	const ICLKS CLOCKSYNCBAND_FAR = 0x40000000;
	ICLK ClockBehindNear = CurrentClock - CLOCKSYNCBAND_NEAR;

	if ((ICLKS)(CurrentClock - ClockReadICR) >= CLOCKSYNCBAND_FAR)
		ClockReadICR = ClockBehindNear;
}

//pragma optimize( "ag", off )

void CIA::incrementTOD()
{
bit32 *p;
	p=(bit32 *)&tod.byte.ths;
#if defined(_WIN64)
	bit8 ah,al;

	/*update tenths*/
	al = tod.byte.ths = (tod.byte.ths + 1) & 0xf;
	if (al!=0xa)
		return;
	tod.byte.ths = 0;

	/*update seconds*/
	al = (tod.byte.sec + 1) & 0xf;
	ah = (tod.byte.sec) & 0xf0;
	if (al != 0xa)
	{
		tod.byte.sec = ah | al;
		return;
	}

	ah = (ah + 0x10) & 0xf0;
	if (ah != 0x60)
	{
		tod.byte.sec = ah;
		return;
	}

	tod.byte.sec=0;

	/*update minutes*/
	al = (tod.byte.min + 1) & 0xf;
	ah = (tod.byte.min) & 0xf0;
	if (al != 0xa)
	{
		tod.byte.min =  ah | al;
		return;
	}

	ah = (ah + 0x10) & 0xf0;
	if (ah != 0x60)
	{
		tod.byte.min = ah;
		return;
	}

	tod.byte.min = 0;
	/*update hours*/
	al = tod.byte.hr;
	ah = al;
	ah = ah & 0x7f;
	if (ah == 0x12)
	{
		al = al & 0x80;
	}
	if ((al & 0x80) !=0)
	{
		/*hours is currently AM*/
		ah = al;
		ah = ah & 0x10;
		al = (al + 1) & 0xf;
		if (al != 0xa)
		{
			al = al | ah;
			if (al == 0x12)
			{
				tod.byte.hr = 0x80;
				return;
			}
			else
			{
				tod.byte.hr = al;
				return;
			}
		}
		else
		{
			tod.byte.hr = 0x10;
			return;
		}	

	}
	else
	{
		/*hours is currently PM*/
		ah = al;
		ah = ah & 0x90;
		al = (al + 1) & 0xf;
		if (al != 0xa)
		{
			if (al == 0x92)
			{
				tod.byte.hr = 0;
			}
			else
			{
				tod.byte.hr = al;
				return;
			}
		}
		else
		{
			tod.byte.hr = 0x90;
			return;
		}
	}
	
#else

	__asm
	{
		/*update tenths*/
		mov ebx, p;
		mov al,[ebx];
		add al,1;
		and al,0Fh;
		mov BYTE PTR [ebx],al; 
		cmp al, 0Ah;
		jne tod_update_exit;
		mov BYTE PTR [ebx],0

		/*update seconds*/
		mov al,1[ebx];
		mov ah,al;
		and ah,0f0h;

		add al,1;
		and al,0Fh;

		cmp al, 0Ah;
		je tod_carry_sec_lo;
		
		or al,ah;
		mov BYTE PTR 1[ebx],al;
		jmp tod_update_exit;

tod_carry_sec_lo:;

		add ah,010h;
		and ah,0F0h;

		cmp ah,060h;
		je tod_carry_sec_hi;

		mov BYTE PTR 1[ebx],ah;
		jmp tod_update_exit;

tod_carry_sec_hi:;
		mov BYTE PTR 1[ebx],0;

		/*update minutes*/
		mov al,2[ebx];
		mov ah,al;
		and ah,0f0h;

		add al,1;
		and al,0Fh;

		cmp al, 0Ah;
		je tod_carry_min_lo;
		
		or al,ah;
		mov BYTE PTR 2[ebx],al;
		jmp tod_update_exit;

tod_carry_min_lo:;

		add ah,010h;
		and ah,0F0h;

		cmp ah,060h;
		je tod_carry_min_hi;

		mov BYTE PTR 2[ebx],ah;
		jmp tod_update_exit;

tod_carry_min_hi:;
		mov BYTE PTR 2[ebx],0;


		/*update hours*/
		mov al,3[ebx];
		
		/*if hours == 12 then normalise to 0 before the increment*/
		mov ah,al;
		and ah,07fh;
		cmp ah,012h;
		jne tod_increment_hours;
		and al,080h;
tod_increment_hours:;

		bt ax,7;
		jc tod_update_pm;

		/*hours is currently AM*/
		mov ah,al;
		and ah,010h;
		add al,1;
		and al,0fh;
		cmp al, 0Ah;
		je tod_carry_hrs_am_lo;
		or al,ah;

		cmp al,012h;
		je tod_carry_hrs_am_hi;

		mov BYTE PTR 3[ebx],al;
		jmp tod_update_exit;

tod_carry_hrs_am_lo:;
		mov BYTE PTR 3[ebx],010h;
		jmp tod_update_exit;

tod_carry_hrs_am_hi:;

		mov al, 80h
		mov BYTE PTR 3[ebx],al;
		jmp tod_update_exit;

tod_update_pm:;
		/*hours is currently PM*/
		mov ah,al;
		and ah,090h;
		add al,1;
		and al,0fh;
		cmp al, 0Ah;
		je tod_carry_hrs_pm_lo;
		or al,ah;

		cmp al,092h;
		je tod_carry_hrs_pm_hi;

		mov BYTE PTR 3[ebx],al;
		jmp tod_update_exit;

tod_carry_hrs_pm_lo:;
		mov BYTE PTR 3[ebx],090h;
		jmp tod_update_exit;

tod_carry_hrs_pm_hi:;

		mov al, 0h
		mov BYTE PTR 3[ebx],al;

tod_update_exit:;
	}
#endif
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
			fastClocks = clocks;
		if (fastClocks > fastTODClocks)
			fastClocks = fastTODClocks;
		ta_counter.word = ta_counter.word - dec_a * (bit16)fastClocks;
		tb_counter.word = tb_counter.word - dec_b * (bit16)fastClocks;

		todclocks = ((fastClocks * tod_clock_rate) + tod_tick) / tod_clock_reload;
		tod_tick = ((fastClocks * tod_clock_rate) + tod_tick) % tod_clock_reload;
		if(tod_write_freeze==0)
		{
			for (ICLKS i = 0 ; i < todclocks; i++)
				incrementTOD();
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
			incrementTOD();
			CheckTODAlarmCompare(sysclock);
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
					if ((serial_int_count & 15) ==1)
					{
						new_icr|=8;
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
			//02 mode
			delay = delay & ~(CountA1);
		else
		{
			//CNT mode
			if ((cra & 1)==0)
				delay = delay & ~(CountA1);  
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
			//02 mode
			delay = delay & ~(CountB1);
		else if ((crb & 0x60)==0x20)
		{
			//CNT mode
			if ((crb & 1)==0)
				delay = delay & ~(CountB1);  
		}
		else if ((crb & 0x60)==0x40)
		{
			//Timer A underflow.
			if (timera_output)
			{
				if (crb & 1)
					delay |= CountB1;
				else
					delay &= ~CountB1;
			}
			else
				delay &= ~CountB1;
		}
		else if ((crb & 0x60)==0x60)
		{
			if (timera_output)
			{
				if (crb & 1)
					delay |= CountB1;
				else
					delay &= ~CountB1;
			}
			else
				delay &= ~CountB1;
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

		if (bTimerBbug && (new_icr & 2) !=0 && ClockReadICR == CurrentClock)
			icr |= (new_icr & ~2);
		else
			icr |= new_icr;
		if (new_icr & imr & 0x1F) 
		{ 
			if (bEarlyIRQ)
			{
				//new CIA
				if (ClockReadICR == CurrentClock)
					//TEST TLR's CIA read DC0D (icr) when Timer counts to zero;
					delay|=Interrupt0;
				else
					delay|=Interrupt1;
			}
			else
			{
				//Old CIA
				delay|=Interrupt0;
			}
		}
		
		// Check for interrupt condition
		if (delay & Interrupt1)
		{
			Interrupt = 1;
			SetSystemInterrupt();
		}

		delay=((delay << 1) & DelayMask) | feed;
		if (f_cnt_in) 
			delay |= (CountA0 | CountB0);
		flag_change=0;

		
		if (delay==old_delay && feed==old_feed)
		{
			if ((((cra & 1)!=0) && ta_counter.word<MINIMUM_GO_IDLE_TIME) || (((crb & 1)!=0) && tb_counter.word<MINIMUM_GO_IDLE_TIME))
				no_change_count=0;
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
			no_change_count=0;
	}
	SetWakeUpClock();
	SetTODWakeUpClock();
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
				currentCount = 0;
			else if ((ICLKS)(testCount - currentCount) < 0)
				currentCount = testCount;
		}
		if ((crb & 0x61)==0x1)
		{
			testCount = tb_counter.word - MINIMUM_GO_IDLE_TIME;
			if ((ICLKS)(testCount) <= 0)
				currentCount = 0;
			else if ((ICLKS)(testCount - currentCount) < 0)
				currentCount = testCount;
		}
		if (tod_write_freeze==0)
		{
			testCount = ClockNextTODWakeUpClock - curClock;
			if (((ICLKS)(testCount) > 0) && ((ICLKS)(testCount - currentCount) < 0))
				currentCount = testCount;
		}
		if ((ICLKS)currentCount < 0)
			currentCount=0;

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
bit8 t;
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
		t=icr;
		icr=0;

		delay &= ~(Interrupt0 | Interrupt1);

		if (Interrupt)
			t|=0x80;
		Interrupt=0;
		ClearSystemInterrupt();
		idle=0;
		no_change_count=0;
		return t;
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
			return tod_read_latch.byte.ths;
		else
			return tod.byte.ths;
	case 0x0B:
		return tod_read_latch.byte.hr;
	case 0x0D:		// interrupt control register
		t=icr;
		if (Interrupt)
			t|=0x80;
		return t;
	default:
		return ReadRegister(address, sysclock);
	}
}

void CIA::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
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
			ta_counter.word = ta_latch.word;
		break;
	case 0x05:		// timer a hi
		ta_latch.byte.hiByte=data;
		if ((cra & 0x01) == 0)
		{
			delay|=LoadA0;
			idle=0;
			no_change_count=0;
		}
		if (delay & LoadA2)
			ta_counter.word = ta_latch.word;
		idle=0;
		no_change_count=0;
		break;
	case 0x06:		// timer b lo
		tb_latch.byte.loByte=data;
		if (delay & LoadB2)
			tb_counter.word = tb_latch.word;
		break;
	case 0x07:		// timer b hi
		tb_latch.byte.hiByte=data;
		if ((crb & 0x01) == 0)
		{
			delay|=LoadB0;
			idle=0;
			no_change_count=0;
		}
		if (delay & LoadB2)
			tb_counter.word = tb_latch.word;
		idle=0;
		no_change_count=0;
		break;
	case 0x08:	//TOD
		if (crb & 0x80)
		{
			alarm.byte.ths=data & 0xf;
		}
		else
		{
			if (tod_write_freeze)
			{
				tod_write_freeze=0;
				tod_tick = 0;
			}
			tod.byte.ths=data & 0xf;
		}
		CheckTODAlarmCompare(sysclock);
		SetTODWakeUpClock();
		break;
	case 0x09:
		if (crb & 0x80)
		{
			alarm.byte.sec=data & 0x7f;
		}
		else
		{
			tod.byte.sec=data & 0x7f;
		}
		CheckTODAlarmCompare(sysclock);
		SetTODWakeUpClock();
		break;
	case 0x0A:
		if (crb & 0x80)
		{
			alarm.byte.min=data & 0x7f;
		}
		else
		{
			tod.byte.min=data & 0x7f;
		}
		CheckTODAlarmCompare(sysclock);
		SetTODWakeUpClock();
		break;
	case 0x0B:
		if (crb & 0x80)
		{
			alarm.byte.hr=data & 0x9f;
		}
		else
		{
			tod_write_freeze=1;
			if (tod_tick % 6 == 0)
				incrementTOD();
			if ((data & 0x1f) == 0x12)
			{
				if (data & 0x80)
					tod.byte.hr = 0x12;
				else
					tod.byte.hr = 0x92;
			}
			else
				tod.byte.hr = data & 0x9f;
		}
		CheckTODAlarmCompare(sysclock);
		SetTODWakeUpClock();
		break;
	case 0x0C://serial data register
		sdr=data;
		if (serial_int_count<=16)
			serial_int_count+=16;
		idle=0;
		no_change_count=0;
		break;
	case 0x0D:		// interrupt control register
		if (data & 0x80)
			imr|=(data & 0x7f);
		else
			imr&=(~data);
		if (icr & imr & 0x1F)
			if (Interrupt == 0)
			{
				if (this->bEarlyIRQ)
					delay|=Interrupt1;
				else
					delay|=Interrupt0;
			}
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
			feed &= ~OneShotA0;

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
				feed |= CountA2;
			else
				feed &= ~CountA2;
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

		if (data & 0x80)
		{
			tod_clock_reload = TODRELOAD50;
			tod_clock_rate = TODDIVIDER50;
			tod_clock_compare_band = TODCLOCKCOMPAREBAND50;
		}
		else
		{
			tod_clock_reload = TODRELOAD60;
			tod_clock_rate = TODDIVIDER60;
			tod_clock_compare_band = TODCLOCKCOMPAREBAND60;
		}

		if ((data ^ cra) & 0x80)
		{
			if (data & 0x80)
				tod_tick = tod_tick * 5 / 60;
			else
				tod_tick = tod_tick * 60 / 5;
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
			feed &= ~OneShotB0;


		//CNT or 02 mode
		if ((data & 0x60)==0)
		{
			//02
			if (data & 0x1) 
				feed |= CountB2;
			else
				feed &= ~CountB2;
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
		if (tod_tick <= tod_clock_compare_band)
		{
			tod_alarm=4;
			idle=0;
			no_change_count=0;
			ClockNextTODWakeUpClock = sysclock + 0x1000000;
		}
	}
}

void CIA::SetTODWakeUpClock()
{
long t;
	if (tod_write_freeze == 0)
	{
		t = GetTenthsFromTimeToAlarm((const cia_tod &)tod, (const cia_tod &)alarm);
		if (t<0)
			ClockNextTODWakeUpClock = CurrentClock + 0x1000000;
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
		ClockNextTODWakeUpClock = CurrentClock + 0xA000000;
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
				return p = a - t;
			else
				return -1;
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
				p = a - t;
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
			t=0;
		else
			t = (t + 1) & 0xF;
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
				return p = a - t;
			else
				return -1;
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
				p = a - t;
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