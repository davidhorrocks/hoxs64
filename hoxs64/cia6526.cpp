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
	randengine.seed(rd());
	CurrentClock=0;
	bEarlyIRQ = true;
	bTimerBbug = false;
	init_bitcount();
	ID = 0;
	is_warm = false;
}

void CIA::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock=sysclock;
	DevicesClock=sysclock;
	ClockNextWakeUpClock=sysclock;
	ClockNextTODWakeUpClock=sysclock;
	idle=false;
	feed=0;
	delay=0;
	old_delay=0;
	old_feed=0;
	delay_aux_mask = 0;
	delay|= CountA3 | CountB3;//Preloaded with 1s. This this correct?
	feed|=SetCnt0;
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
	serial_int_count=0;
	serial_interrupt_delay=0;
	serial_data_register=0;
	serial_shift_buffer=0;
	serial_data_write_pending=false;
	serial_data_write_loading=false;
	cra=0;
	crb=0;
	ta_latch.word=0xffff;
	tb_latch.word=0xffff;
	ta_counter.word=0;
	tb_counter.word=0;
	f_sp_in=true;
	f_sp_out=false;
	f_cnt_in=true;
	f_cnt_out=true;//CNT high by default
	f_flag_in=false;	
	flag_change=false;
	timera_output=0;
	timerb_output=0;
	bPB67Toggle=0;
	bPB67TimerMode=0;
	bPB67TimerOut=0;
	icr=0;
	icr_ack=0;
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
	{
		ClockReadICR = ClockBehindNear;
	}

	if (CurrentClock > CIA_MAX_TEMPERATURE_TIME)
	{
		is_warm = true;
	}
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
				idle=false;
				no_change_count=0;
			}
			continue;
		}
		ClockNextWakeUpClock = sysclock;

		old_delay = delay;
		old_feed = feed;
		new_icr=0;
		bool current_cnt = this->ReadCntPinLevel();
		//TIMER A
		//A bit passed through the pipeline to signal Timer A to decrement
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
				//serial out
				if (this->serial_data_write_loading && serial_int_count == 0)
				{
					this->serial_data_write_loading = false;
					this->serial_shift_buffer = this->serial_data_register;
					serial_int_count = 16;
				}
				if (serial_int_count)
				{
					delay |= SetCntFlip0;
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
		bool is_CNT_rising = false;
		if (serial_int_count != 0 && (delay & SetCntFlip2) != 0 && (delay & SetCntFlip3) == 0)
		{
			if (this->SetSerialCntOut(!this->f_cnt_out))
			{
				is_CNT_rising = this->f_cnt_out;
				serial_int_count--;
				if (serial_int_count == 1)
				{
					serial_interrupt_delay = 1;
				}
			}
			if (this->f_cnt_out)
			{
				feed |= SetCnt0;
			}
			else
			{
				feed &= ~SetCnt0;
			}
		}

		if (serial_interrupt_delay)
		{
			if ((serial_interrupt_delay & 0x04))
			{
				new_icr|=8;
			}
			serial_interrupt_delay = (serial_interrupt_delay << 1) & 0x3f;
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
				//Timer A stopped
				delay = delay & ~(CountA1);  
			}
		}

		if (this->serial_data_write_pending)
		{
			this->serial_data_write_pending = false;
			this->serial_data_write_loading = true;
		}

		//TIMER B
		//A bit passed through the pipeline to signal Timer B to decrement
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
			//Count 02 clocks.
			delay = delay & ~(CountB1);
		}
		else if ((crb & 0x60)==0x20)
		{
			//Count CNT positive transitions.
			if ((crb & 1)==0)
			{
				delay = delay & ~(CountB1);  
			}
		}
		else if ((crb & 0x60)==0x40)
		{
			//Timer A underflows.
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
			//Timer A underflows while CNT is high.
			if (timera_output != 0 && ((delay & SetCnt1) != 0))
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

		if (!f_flag_in && flag_change)
		{
			new_icr |= 0x10;
		}
		if(tod_alarm)
		{
			new_icr|= tod_alarm;
			tod_alarm=0;
		}

		icr_ack = icr_ack & ~new_icr;
		if (this->bEarlyIRQ)
		{
			if ((delay & ClearIcr1) !=0)
			{
				icr = icr & ~icr_ack;
				icr_ack = 0;
			}
		}
		else
		{
			if ((delay & ClearIcr1) != 0)
			{
				icr = icr & ~icr_ack;
				icr = icr & 0x7f;
				icr_ack = 0;
			}
		}

		if (bTimerBbug && (new_icr & 2) !=0 && ClockReadICR == CurrentClock && (delay & (ReadIcr1)) == 0)
		{
			icr |= new_icr;
			icr_ack |= 2;
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
				if (ClockReadICR == CurrentClock)
				{
					//TEST TLR's CIA read DC0D (icr) when Timer counts to zero;
					//Used by testprogs\interrupts\irqnmi\cia-int-irq-new.prg
					//Used by testprogs\interrupts\irqnmi\cia-int-nmi-new.prg
					delay|=Interrupt0;
					delay|=SetIcr0;
				}
				else
				{
					delay|=Interrupt1;
					delay|=SetIcr1;
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
		if (is_CNT_rising)
		{
			this->delay |= CountA0;
			this->delay |= CountB0;
		}

		flag_change=false;		
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
					idle=true;
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
	if (!idle)
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
	f_flag_in=false;
	flag_change=true;
	idle=false;
	no_change_count=0;
}


bool CIA::ReadSpPinLevel()
{
	if (cra & 0x40)
	{
		//Serial output
		return this->f_sp_out && this->f_sp_in;
	}
	else
	{
		//Serial input
		return this->f_sp_in;
	}
}

bool CIA::ReadCntPinLevel()
{
	if (cra & 0x40)
	{
		//Serial output
		return this->f_cnt_out && this->f_cnt_in;
	}
	else
	{
		//Serial input
		return this->f_cnt_in;
	}	
}

void CIA::SetSerialSpOut(bool value)
{
	this->f_sp_out = value;
}

bool CIA::SetSerialCntOut(bool value)
{
	bool isChanged = false;
	if ((cra & 0x40) != 0)
	{
		//Serial port output
		if (value && !this->f_cnt_out)
		{
			//CNT rising
			isChanged = true;
			if (serial_int_count)
			{
				serial_shift_buffer = (serial_shift_buffer << 1) & 0xff;
			}
		}
		else if (!value && this->f_cnt_out)
		{
			//CNT falling
			isChanged = true;
			if (serial_int_count)
			{				
				this->SetSerialSpOut((serial_shift_buffer & 0x80) != 0);					
			}
		}
	}
	this->f_cnt_out = value;
	return isChanged;
}

void CIA::CntChanged()
{
	if (this->delay & SetCnt3)
	{
		this->delay |= CountA0;
		this->delay |= CountB0;
	}
}

//pragma optimize( "ag", on )

bit8 CIA::ReadRegister(bit16 address, ICLK sysclock)
{
bit8 result;
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
		return serial_data_register;
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
			delay &= ~(Interrupt1);
			delay &= ~(SetIcr1);
			result = icr;
		}
		else
		{
			delay |= ClearIcr0;
			delay &= ~(Interrupt1);
			result = icr;
			icr = icr & 0x80;
		}

		delay |= ReadIcr0;
		Interrupt = 0;
		ClearSystemInterrupt();
		idle = false;
		no_change_count = 0;
		SetWakeUpClock();
		return result;
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
bit8 data_old;

	ExecuteCycle(sysclock);
	switch(address & 0x0F)
	{
	case 0x00:		// port a
		data_old = pra_out;
		pra_out = data;
		WritePortA(false, ddra, data_old, ddra, data);
		break;
	case 0x01:		// port b
		data_old = prb_out;
		prb_out = data;
		WritePortB(false, ddrb, data_old, ddrb, data);
		break;
	case 0x02:		// data direction a
		data_old = ddra;
		ddra = data;
		WritePortA(true, data_old, pra_out, data, pra_out);
		break;
	case 0x03:		// data direction b
		data_old = ddrb;
		ddrb = data;
		WritePortB(true, data_old, prb_out, data, prb_out);
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
		idle=false;
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
		idle=false;
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
		serial_data_register = data;
		serial_data_write_pending = true;
		serial_data_write_loading = false;
		idle=false;
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
				if ((delay & ClearIcr2) != 0)
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
		idle=false;
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

		if ((data ^ cra) & 0x40)
		{
			//serial direction changing
			this->serial_int_count = 0;
			this->serial_data_write_pending = false;
			this->serial_data_write_loading = false;
			if (this->SetSerialCntOut(true))
			{
				if ((delay & SetCntFlip3) == 0)
				{
					this->delay |= CountA0;
					this->delay |= CountB0;
				}
			}
			if (this->f_cnt_out)
			{
				feed |= SetCnt0;
				delay |= SetCnt0;
			}
			else
			{
				feed &= ~SetCnt0;
				delay &= ~SetCnt0;
			}
			this->delay &= ~(SetCntFlip0 | SetCntFlip1 | SetCntFlip2 | SetCntFlip3);
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

		idle=false;
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

		idle=false;
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
		idle=false;
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

void CIA::GetState(SsCiaV2 &state)
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
	state.serial_interrupt_delay = serial_interrupt_delay;
	state.delay_aux_mask = delay_aux_mask;
	state.serial_shift_buffer = serial_shift_buffer;
	state.serial_data_write_pending = serial_data_write_pending;
	state.serial_data_write_loading = serial_data_write_loading;
	state.serial_other = 0;
	state.int32_buffer0 = 0;
	state.int32_buffer1 = 0;
	state.int32_buffer2 = 0;
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
	state.serial_data_register = serial_data_register;
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

void CIA::SetState(const SsCiaV2 &state)
{
	CurrentClock = state.CurrentClock;
	DevicesClock = state.DevicesClock;
	ClockNextWakeUpClock = state.ClockNextWakeUpClock;
	ClockNextTODWakeUpClock = state.ClockNextTODWakeUpClock;
	delay = state.delay;
	feed = state.feed;
	old_delay = state.old_delay;
	old_feed = state.old_feed;
	idle = state.idle != 0;
	dec_a = state.dec_a;
	dec_b = state.dec_b;
	no_change_count = state.no_change_count;
	flag_change = state.flag_change != 0;
	serial_interrupt_delay = state.serial_interrupt_delay;
	delay_aux_mask = state.delay_aux_mask;
	serial_shift_buffer = state.serial_shift_buffer;
	serial_data_write_pending = state.serial_data_write_pending != 0;
	serial_data_write_loading = state.serial_data_write_loading != 0;
	//state.serial_other;
	//state.int32_buffer0;
	//state.int32_buffer1;
	//state.int32_buffer2;
	f_sp_in = state.f_sp_in != 0;
	f_sp_out = state.f_sp_out != 0;
	f_cnt_in = state.f_cnt_in != 0;
	f_cnt_out = state.f_cnt_out != 0;
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
	serial_data_register = state.serial_data_register;
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

void CIA::UpgradeStateV1ToV2(const SsCiaV1 &in, SsCiaV2 &out)
{
	out.CurrentClock = in.CurrentClock;
	out.DevicesClock = in.DevicesClock;
	out.ClockNextWakeUpClock = in.ClockNextWakeUpClock;
	out.ClockNextTODWakeUpClock = in.ClockNextTODWakeUpClock;
	out.delay = in.delay;
	out.feed = in.feed;
	out.old_delay = in.old_delay;
	out.old_feed = in.old_feed;
	out.idle = in.idle != 0;
	out.dec_a = in.dec_a;
	out.dec_b = in.dec_b;
	out.no_change_count = in.no_change_count;
	out.flag_change = in.flag_change != 0;
	out.serial_interrupt_delay = in.serial_interrupt_delay;
	out.delay_aux_mask = in.delay_aux_mask;
	out.serial_shift_buffer = in.serial_shift_buffer;
	out.serial_data_write_pending = in.serial_data_write_pending != 0;
	out.serial_data_write_loading = in.serial_data_write_loading != 0;
	out.serial_other = in.serial_other;
	out.int32_buffer0 = in.int32_buffer0;
	out.int32_buffer1 = in.int32_buffer1;
	out.int32_buffer2 = in.int32_buffer2;
	out.f_sp_in = in.f_sp_in != 0;
	out.f_sp_out = in.f_sp_out != 0;
	out.f_cnt_in = in.f_cnt_in != 0;
	out.f_cnt_out = in.f_cnt_out != 0;
	out.pra_out = in.pra_out;
	out.prb_out = in.prb_out;
	out.ddra = in.ddra;
	out.ddrb = in.ddrb;
	out.ta_counter = in.ta_counter;
	out.tb_counter = in.tb_counter;
	out.ta_latch = in.ta_latch;
	out.tb_latch = in.tb_latch;
	out.tod_clock_reload = in.tod_clock_reload;
	out.tod_clock_rate = in.tod_clock_rate;
	out.tod_tick = in.tod_tick;
	out.tod_clock_compare_band = in.tod_clock_compare_band;
	out.tod_alarm = in.tod_alarm;
	out.tod_read_freeze = in.tod_read_freeze;	
	out.tod_read_latch.dword = in.tod_read_latch.dword;
	out.tod_write_freeze = in.tod_write_freeze;
	out.tod_write_latch.dword = in.tod_write_latch.dword;
	out.tod.dword = in.tod.dword;
	out.alarm.dword = in.alarm.dword;
	out.serial_data_register = in.serial_data_register;
	out.cra = in.cra;
	out.crb = in.crb;
	out.timera_output = in.timera_output;
	out.timerb_output = in.timerb_output;
	out.icr = in.icr;
	out.icr_ack = in.icr_ack;
	out.imr = in.imr;
	out.Interrupt = in.Interrupt;
	out.serial_int_count = in.serial_int_count;
	out.bEarlyIRQ = in.bEarlyIRQ != 0;
	out.bTimerBbug = in.bTimerBbug != 0;
	out.ClockReadICR = in.ClockReadICR;
	out.bPB67TimerMode = in.bPB67TimerMode;
	out.bPB67TimerOut = in.bPB67TimerOut;
	out.bPB67Toggle = in.bPB67Toggle;
}
