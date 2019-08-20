#include <windows.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "savestate.h"
#include "register.h"
#include "hconfig.h"
#include "appstatus.h"
#include "sidvoice.h"
#include "sidwavetable.h"

#define SHIFTERTESTDELAY (0x49300)
#define SAMPLEHOLDRESETTIME  0x001D0000
#define NOISE_ZERO_FEED ((~((1U << 20) | (1 << 18) | (1 << 14) | (1 << 11) | (1 << 9) | (1 << 5) | (1 << 2) | (1 << 0))) & 0x7fffff)
#define ENVELOPE_LFSR_RESET (0x7fff)

const bit16 SIDVoice::AdsrTable[16] = {
 0x7F00,
 0x0006,
 0x003C,
 0x0330,
 0x20C0,
 0x6755,
 0x3800,
 0x500E,
 0x1212,
 0x0222,
 0x1848,
 0x59B8,
 0x3840,
 0x77E2,
 0x7625,
 0x0A93
};

void SIDVoice::Reset(ICLK sysclock, bool poweronreset)
{
	if (poweronreset)
	{
		counter=0x555555;
	}

	shifterTestCounter=0;
	zeroTheShiftRegister = false;
	gotNextVolume = false;
	nextvolume = 0;
	volume=0;
	sampleHoldDelayClock = sysclock;
	exponential_counter_period=1;
	next_exponential_counter_period = 0;
	frequency=0;
	sustain_level=0;
	attack_value=0;
	decay_value=0;
	release_value=0;
	ring_mod=0;
	sync=0;
	zeroTheCounter=false;
	wavetype=sidWAVNONE;
	envmode=sidRELEASE;
	next_envmode = sidRELEASE;
	latched_envmode = sidRELEASE;
	want_latched_envmode = false;
	envmode_changing_delay = 0;
	envelope_count_delay = 0;
	exponential_count_delay = 0;
	reset_envelope_counter = false;
	reset_exponential_counter = false;
	envelope_tick = false;	
	pulse_width_reg=0;
	pulse_width_counter=0;
	gate=0;
	test=0;
	sidShiftRegister=0x7fffff;
	sidShiftRegisterFill = 1;
	phaseOfShiftRegister = 0;
	keep_zero_volume=true;
	envelope_counter=ENVELOPE_LFSR_RESET;
	envelope_compare=SIDVoice::AdsrTable[0];
	exponential_counter=0;
	control=0;
	noiseFeedbackMask1 = 0x000;
	noiseFeedbackMask0 = 0xfff;
	noiseFeedbackSample1 = 0;
	noiseFeedbackSample2 = 0;
	lastSample = CalcWaveOutput(wavetype, this->noiseFeedbackSample1, this->noiseFeedbackMask0, this->noiseFeedbackMask1);
}

SIDVoice::~SIDVoice()
{
}

HRESULT SIDVoice::Init(CAppStatus *appStatus)
{
	this->appStatus = appStatus;
	return S_OK;
}

void __forceinline SIDVoice::ClockShiftRegister()
{
bit32 t;
	if (test)
	{
		t = ((sidShiftRegister >> 17) & 0x1) ^ 0x1;
	}
	else
	{
		t = ((sidShiftRegister >> 22) ^ (sidShiftRegister >> 17)) & 0x1;
		if (zeroTheShiftRegister)
		{
			t = 0;
		}
	}	

	sidShiftRegister = ((sidShiftRegister << 1) | t) & 0x7fffff;
}

void SIDVoice::SyncRecheck()
{
	if (zeroTheCounter && !modulator_voice->zeroTheCounter)
	{
		counter = 0;
		zeroTheCounter = false;
	}	
}

void SIDVoice::Envelope()
{
bit32 t, c;
	samplevolume = volume;

	if (test)
	{
		phaseOfShiftRegister = 0;
		shifterTestCounter--;
		if (shifterTestCounter<0)
		{
			shifterTestCounter=SHIFTERTESTDELAY;
			sidShiftRegister |= sidShiftRegisterFill;
			sidShiftRegisterFill = sidShiftRegisterFill <<= 1;
			zeroTheShiftRegister = false;
		}

		affected_voice->zeroTheCounter = false;
	}
	else
	{
		if (phaseOfShiftRegister)
		{
			switch(phaseOfShiftRegister)
			{
			case 1:
				NoiseWriteback(this->control, this->noiseFeedbackSample1, this->noiseFeedbackMask0, 0);
				ClockShiftRegister();
				phaseOfShiftRegister++;
				break;
			case 2:
				phaseOfShiftRegister = 0;
				break;
			}
		}
		else
		{
			NoiseWriteback(this->control, this->noiseFeedbackSample1, this->noiseFeedbackMask0, 0);
		}

		t = counter;
		c = (t + frequency) & 0x00ffffff;
		if (c & 0x080000 & ~t) 
		{
			phaseOfShiftRegister = 1;
		}

		if ((~t & 0x0800000 & c) != 0 && affected_voice->sync != 0)
		{
			affected_voice->zeroTheCounter = true;
		}

		counter = c;
	}

	if (envmode_changing_delay)
	{
		envmode_changing_delay--;
		UpdateNextEnvMode();
	}

	if (reset_exponential_counter)
	{
		reset_exponential_counter = false;
		exponential_counter = 0;
		envelope_tick = true;
	}

	exponential_counter_period = next_exponential_counter_period;
	
	if (envelope_count_delay)
	{
		envelope_count_delay--;
		if (envelope_count_delay == 0)
		{
			if (envmode == sidATTACK || exponential_counter == exponential_counter_period)
			{
				// Sustain is checked one cycle before the volume is decremented.
				if (envmode == sidDECAY && volume == sustain_level)
				{
					nextvolume = volume;
					gotNextVolume = true;
				}

				reset_exponential_counter = true;
			}
		}
	}

	if (exponential_count_delay)
	{
		exponential_count_delay--;
		if (exponential_count_delay == 0)
		{
			if (exponential_counter == exponential_counter_period)
			{
				if (envmode == sidATTACK)
				{
					if (!want_latched_envmode)
					{
						// Release to attack one cycle before the volume is decremented will freeze the volume for one rate period.
						nextvolume = volume;
						gotNextVolume = true;
					}
				}

				// Sustain is checked one cycle before the volume is decremented.
				if (envmode == sidDECAY && volume == sustain_level)
				{
					nextvolume = volume;
					gotNextVolume = true;
				}

				reset_exponential_counter = true;
			}
		}
		else if (exponential_count_delay == 1)
		{
			// The ADSR may remain in decay / release mode even if attack mode is selected in the next clock.
			if (exponential_counter == exponential_counter_period)
			{
				latched_envmode = envmode;
				want_latched_envmode = true;
			}
		}
	}

	if (envelope_tick)
	{
		envelope_tick = false;
		if (gotNextVolume)
		{
			gotNextVolume = false;
			volume = nextvolume;
		}
		else
		{
			volume = GetNextVolume();
		}

		switch (volume)
		{
		case 0x00:
			next_exponential_counter_period = 0x01;
			break;
		case 0x06:
			next_exponential_counter_period = 0x1e;
			break;
		case 0x0e:
			next_exponential_counter_period = 0x10;
			break;
		case 0x1a:
			next_exponential_counter_period = 0x08;
			break;
		case 0x36:
			next_exponential_counter_period = 0x04;
			break;
		case 0x5d:
			next_exponential_counter_period = 0x02;
			break;
		case 0xff:
			next_exponential_counter_period = 0x01;
			break;
		}

		exponential_counter = 0;		
	}
	
	if (reset_envelope_counter)
	{
		reset_envelope_counter = false;
		envelope_counter = ENVELOPE_LFSR_RESET;			
		exponential_counter++;
		if (envmode == sidATTACK)
		{
			envelope_count_delay = 1;
		}
		else
		{
			// There is a one cycle extra delay if the exponential counter period is more than one.
			exponential_count_delay = exponential_counter_period == 1 ? 1 : 2;

			// The ADSR may remain in decay / release mode even if attack mode is selected in the next clock.
			if (exponential_counter == exponential_counter_period && exponential_counter_period == 1)
			{
				latched_envmode = envmode;
				want_latched_envmode = true;
			}
		}
	}

	if (envelope_counter == envelope_compare)
	{
		reset_envelope_counter = true;
		want_latched_envmode = false;
		latched_envmode = envmode;
	}
	else
	{
		bit16 feedback = ((envelope_counter >> 14) ^ (envelope_counter >> 13)) & 1;
		envelope_counter = ((envelope_counter << 1) & 0x7ffe) | feedback;
	}
}

bit8 SIDVoice::GetNextVolume()
{
bit8 nextvol = volume;
	if (!keep_zero_volume)
	{
		switch (want_latched_envmode ? latched_envmode : envmode)
		{
		case sidATTACK:
			nextvol = (volume + 1) & 255;
			if (nextvol == 255)
			{
				envelope_compare = SIDVoice::AdsrTable[decay_value];
				envmode = sidDECAY;
			}

			break;
		case sidDECAY:
		case sidRELEASE:
			nextvol = (volume - 1) & 255;
			break;
		}

		if (nextvol == 0)
		{
			keep_zero_volume = true;
		}
	}

	return nextvol;
}

void SIDVoice::UpdateNextEnvMode()
{
	switch(next_envmode)
	{
	case sidATTACK:
		if (envmode_changing_delay == 2)
		{
			// In a post titled "Understanding the Sid" drfiemost said, "One funny fact is that in the first cycle of the attack and decay phases the wrong rate is used, as the R0 line reacts with one cycle delay."
			// The assumption is that the decay rate is selected for one clock before selecting the attack rate.
			// http://forum.6502.org/viewtopic.php?f=8&t=4150&start=105
			if (this->reset_envelope_counter && this->exponential_counter_period != 1)
			{
				// Hack required for attack during release where one extra delay cycle may be required.
				envmode = sidDECAY;
			}
			else
			{
				envmode = sidATTACK;
			}

			envelope_compare = SIDVoice::AdsrTable[decay_value];
		}
		else if (envmode_changing_delay == 1)
		{
			// Two clocks later, select the attack rate.
			envmode = sidATTACK;
			envelope_compare = SIDVoice::AdsrTable[attack_value];
		}
		else if (envmode_changing_delay == 0)
		{
			keep_zero_volume = false;
		}

		break;
	case sidDECAY:
		if (envmode_changing_delay == 0)
		{
			envmode = sidDECAY;
			envelope_compare = SIDVoice::AdsrTable[decay_value];
		}

		break;
	case sidRELEASE:
		if (envmode_changing_delay == 1)
		{
			if (envmode == sidDECAY)
			{
				envmode = sidRELEASE;
				envelope_compare = SIDVoice::AdsrTable[release_value];
			}
		}
		else if (envmode_changing_delay == 0)
		{
			envmode = sidRELEASE;
			envelope_compare = SIDVoice::AdsrTable[release_value];			
		}

		break;
	}
}

void SIDVoice::SetWave(bit8 new_control)
{
bit8 prev_control = control;
	control = new_control;
	wavetype = (eWaveType)((new_control >> 4) & 0xf);
	sync=(new_control & 2);
	ring_mod=(new_control & 4);
	if ((new_control ^ prev_control) & 0xf0)//control changing
	{				
		if (wavetype == sidWAVNONE)
		{
			sampleHoldDelayClock = pISidChip->CurrentClock + SAMPLEHOLDRESETTIME;
			sampleHold = lastSample;
			wavetype = sidWAVNONE;
		}
	}

	if (new_control & 8)
	{
		if (test == 0) // test bit going on
		{
			sidShiftRegisterFill = 1;
			phaseOfShiftRegister = 0;
			shifterTestCounter = SHIFTERTESTDELAY;
			if ((prev_control & 0xF0) == 0x80 && (new_control & 0xF0) == 0xA0)
			{
				// check_A_to_8_new.prg:
				// No noise writeback
			}
			else if ((prev_control & 0xF0) == 0x80 && (new_control & 0xF0) == 0x90)
			{
				// Test 9a ?
				// No noise writeback
			}
			else if ((prev_control & 0xF0) == 0x80 && (new_control & 0xF0) == 0xB0)
			{
				// Test 11a ?
				// No noise writeback
			}
			else if ((prev_control & 0xF0) == 0x80 && (new_control & 0xC0) == 0xC0 && (new_control & 0x30) != 0)
			{
				// wf12nsr-8580.prg test 8
				NoiseWriteback(new_control, 0, 0xfff, 0x03C0);
			}
			else if ((prev_control & 0xF0) == 0xC0 && (new_control & 0xC0) == 0xC0 && (new_control & 0x30) != 0)
			{
				// wf12nsr-8580.prg test 12
				NoiseWriteback(new_control, 0, 0xfff, 0x0180);
			}
			else if ((prev_control & 0xC0) == 0xC0 && (prev_control & 0x30) != 0)
			{
				// wf12nsr-8580.prg test 15
				NoiseWriteback(prev_control, 0, 0xfff, 0x0000);
			}

			test=1;
			counter=0;
		}			
	}
	else
	{
		if (test) // test bit going off.
		{
			if ((prev_control & 0xF0) == 0xA0 && (new_control & 0xF0) == 0x80)
			{
				// check_A_to_8_new.prg:
				// No noise writeback
			}
			else if ((prev_control & 0xF0) == 0x90 && (new_control & 0xF0) == 0x80)
			{
				// Test 9a ?
				// No noise writeback
			}
			else if ((prev_control & 0xF0) == 0xB0 && (new_control & 0xF0) == 0x80)
			{
				// Test 11a ?
				// No noise writeback
			}
			else if ((prev_control & 0xC0) == 0xC0 && (prev_control & 0x30) != 0 && (new_control & 0xF0) == 0x80)
			{
				// wf12nsr-8580.prg tests 8, 12
				NoiseWriteback(prev_control, 0, 0xfff, 0x03c0);
			}
			else if (((new_control & 0x80) != 0 && (new_control & 0x70) != 0) || ((prev_control & 0x80) != 0 && (prev_control & 0x70) != 0 && (new_control & 0x80) != 0))
			{				
				NoiseWriteback(prev_control, this->noiseFeedbackSample1, this->noiseFeedbackMask0, this->noiseFeedbackMask1);
			}

			ClockShiftRegister();
			phaseOfShiftRegister = 2;
			test=0;
		}
	}

	if ((new_control & 0xC0) == 0xC0)
	{
		zeroTheShiftRegister = true;
	}
	else
	{
		zeroTheShiftRegister = false;
	}

	if (new_control & 1)
	{
		if (gate == 0)
		{
			gate = 1;
			next_envmode = sidATTACK;
			envmode_changing_delay = 3;
		}
	}
	else
	{
		if (gate != 0)
		{
			gate = 0;
			next_envmode = sidRELEASE;
			
			// Release during attack appears require to 3 cycles to take effect.
			envmode_changing_delay = 3;
		}
	}
}

bit16 SIDVoice::CalcWaveOutput(bit8 waveType, bit16 &noiseFeedbackWave, bit16 &mask0, bit16 &mask1)
{
bool msb;
DWORD dwMsb;
bit16 wave;
	
	mask0 = 0xfff;
	mask1 = 0;
	switch (waveType)
	{
	case sidTRIANGLE:
		if (ring_mod)
		{
			msb = ((modulator_voice->counter ^ counter) & 0x00800000) == 0;
		}
		else
		{
			msb = (counter & 0x00800000) != 0;
		}

		if (msb)
		{
			wave = (unsigned short)  (((~counter) & 0x007fffff) >> 11);
		}
		else
		{
			wave = (unsigned short)  ((counter & 0x007fffff) >> 11);
		}

		noiseFeedbackWave = wave;
		return wave;
	case sidSAWTOOTH:
		wave = noiseFeedbackWave = (unsigned short) (counter >> 12);
		noiseFeedbackWave = wave;
		return wave;
	case sidPULSE:
		if(test)
		{
			wave =  0x0fff;
		}
		else
		{
			if ((counter>>12) >= pulse_width_reg)
			{
				wave =  0x0fff;
			}
			else
			{
				wave = 0x0000;
			}
		}

		noiseFeedbackWave = wave;
		return wave;
	case sidNOISE:
		noiseFeedbackWave = 0xfff;
		return ShiftRegisterOutput();
	case sidTRIANGLEPULSE:
		if (ring_mod)
		{
			dwMsb = ((~modulator_voice->counter ^ counter) & 0x00800000);
		}
		else
		{
			dwMsb = (counter & 0x00800000);
		}

		wave = CalcWaveOutput(sidPULSE, noiseFeedbackWave, mask0, mask1) & (unsigned short)sidWave_PT[(unsigned short)  ((dwMsb | (counter & 0x007fffff)) >> 12)] << 4;
		noiseFeedbackWave = wave;
		return wave;
	case sidSAWTOOTHPULSE:
		wave = CalcWaveOutput(sidPULSE, noiseFeedbackWave, mask0, mask1) & sidWave_PS[counter >> 12] << 4;
		noiseFeedbackWave = wave;
		return wave;
	case sidTRIANGLESAWTOOTHPULSE:
		wave = CalcWaveOutput(sidPULSE, noiseFeedbackWave, mask0, mask1) & (unsigned short)sidWave_PST[counter >> 12] << 4;
		noiseFeedbackWave = wave;
		return wave;
	case sidTRIANGLESAWTOOTH:
		wave = (unsigned short)sidWave_ST[counter >> 12] << 4;
		noiseFeedbackWave = wave;
		return wave;
	case sidWAVNONE:
		if ((ICLKS)(pISidChip->CurrentClock - sampleHoldDelayClock) >= 0)
		{
			noiseFeedbackWave = 0;
			mask0 = 0x000;
			mask1 = 0x000;
			return 0;
		}
		else
		{
			noiseFeedbackWave = 0;
			mask0 = 0x000;
			mask1 = 0x000;
			return sampleHold;
		}		
	case sidNOISETRIANGLE:
		wave = CalcWaveOutput(sidTRIANGLE, noiseFeedbackWave, mask0, mask1);
		wave &= ShiftRegisterOutput();
		return wave;
	case sidNOISESAWTOOTH:
		wave = CalcWaveOutput(sidSAWTOOTH, noiseFeedbackWave, mask0, mask1);
		wave &= ShiftRegisterOutput();
		return wave;
	case sidNOISEPULSE:
		wave = CalcWaveOutput(sidPULSE, noiseFeedbackWave, mask0, mask1);
		noiseFeedbackWave = wave;
		if (test)
		{
			wave &= 0xfe0;
		}
		else
		{
			wave &= 0xfc0;
		}

		mask0 = 0x0fff;
		mask1 = 0x0180;
		wave &= ShiftRegisterOutput();
		return wave;
	default:
		noiseFeedbackWave = 0;
		mask0 = 0x000;
		mask1 = 0x000;
		return 0;
	}
}

bit16 SIDVoice::ShiftRegisterOutput()
{
	// Noise: the lower four bits are grounded while the others are connected respectively to the bits 0, 2, 5, 9, 11, 14, 18 and 20 of the LFSR register;
	return ((unsigned short)(
	((sidShiftRegister & 0x100000) >> 9) |
	((sidShiftRegister & 0x040000) >> 8) |
	((sidShiftRegister & 0x004000) >> 5) |
	((sidShiftRegister & 0x000800) >> 3) |
	((sidShiftRegister & 0x000200) >> 2) |
	((sidShiftRegister & 0x000020) << 1) |
	((sidShiftRegister & 0x000004) << 3) |
	((sidShiftRegister & 0x000001) << 4)
	));
}

void SIDVoice::Modulate()
{
short sample;
	lastSample = CalcWaveOutput(wavetype, noiseFeedbackSample1, noiseFeedbackMask0, noiseFeedbackMask1);
	sample = ((short)(lastSample & 0xfff) - 0x800);
	fVolSample = ((double)((long)sample * (long)volume) / (255.0));
}

void SIDVoice::NoiseWriteback(bit8 control, bit16 sample, bit16 mask0, bit16 mask1)
{
	if ((control & 0x80) != 0 && (control & 0x70) != 0)
	{	
		bit32 t;	
		t = (bit32)(sample & mask0) | mask1;
		t = ((t & 0x010) >> 4) | 
			((t & 0x020) >> 3) |
			((t & 0x040) >> 1) |
			((t & 0x080) << 2) |
			((t & 0x100) << 3) |
			((t & 0x200) << 5) |
			((t & 0x400) << 8) |
			((t & 0x800) << 9);
		sidShiftRegister = (sidShiftRegister & (t | NOISE_ZERO_FEED));
	}
}

void SIDVoice::PreventClockOverflow(ICLK sysclock)
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = sysclock - CLOCKSYNCBAND_NEAR;
	if ((ICLKS)(sysclock - this->sampleHoldDelayClock) >= CLOCKSYNCBAND_FAR)
	{
		this->sampleHoldDelayClock = ClockBehindNear;
	}
}

void SIDVoice::GetState(SsSidVoiceV3 &state)
{
	state.counter = counter;
	state.frequency = frequency;
	state.volume = volume;
	state.sampleHoldDelayClock = sampleHoldDelayClock;
	state.envmode = envmode;
	state.wavetype = wavetype;
	state.sync = sync;
	state.ring_mod = ring_mod;
	state.sustain_level = sustain_level;
	state.zeroTheCounter = zeroTheCounter ?  1 : 0;
	state.exponential_counter_period = exponential_counter_period;
	state.attack_value = attack_value;
	state.decay_value = decay_value;
	state.release_value = release_value;
	state.pulse_width_counter = pulse_width_counter;
	state.pulse_width_reg = pulse_width_reg;
	state.sampleHold = sampleHold;
	state.lastSample = lastSample;
	state.fVolSample = fVolSample;
	state.gate = gate;
	state.test = test;
	state.sidShiftRegister = sidShiftRegister;
	state.keep_zero_volume = keep_zero_volume ? 1 : 0;
	state.envelope_counter = envelope_counter;
	state.envelope_compare = envelope_compare;
	state.exponential_counter = exponential_counter;
	state.control = control;
	state.shifterTestCounter = shifterTestCounter;
	state.phaseOfShiftRegister = phaseOfShiftRegister;
	state.nextvolume = nextvolume;
	state.samplevolume = samplevolume;
	state.next_envmode = next_envmode;
	state.envmode_changing_delay = envmode_changing_delay;
	state.envelope_count_delay = envelope_count_delay;
	state.exponential_count_delay = exponential_count_delay;
	state.next_exponential_counter_period = next_exponential_counter_period;
	state.gotNextVolume = gotNextVolume ? 1 : 0;
	state.reset_envelope_counter = reset_envelope_counter ? 1 : 0;
	state.reset_exponential_counter = reset_exponential_counter ? 1 : 0;
	state.envelope_tick = envelope_tick ? 1 : 0;

	// Fields added to the new version 3.
	state.want_latched_envmode = want_latched_envmode ? 1 : 0;
	state.latched_envmode = latched_envmode;
	state.sidShiftRegisterFill = sidShiftRegisterFill;
	state.noiseFeedbackSample1 = noiseFeedbackSample1;
	state.noiseFeedbackSample2 = noiseFeedbackSample2;
	state.noiseFeedbackMask0 = noiseFeedbackMask0;
	state.noiseFeedbackMask1 = noiseFeedbackMask1;
	state.zeroTheShiftRegister = zeroTheShiftRegister ? 1 : 0;
}

void SIDVoice::SetState(ICLK sysclock, const SsSidVoiceV3 &state)
{
	counter = state.counter;
	frequency = state.frequency;
	volume = (bit8)state.volume;
	sampleHoldDelayClock = state.sampleHoldDelayClock;
	envmode = (eEnvelopeMode) state.envmode;
	wavetype = (eWaveType) state.wavetype;
	sync = state.sync;
	ring_mod = state.ring_mod;
	sustain_level = state.sustain_level;
	zeroTheCounter = state.zeroTheCounter != 0;
	exponential_counter_period = state.exponential_counter_period;
	attack_value = state.attack_value;
	decay_value = state.decay_value;
	release_value = state.release_value;
	pulse_width_counter = state.pulse_width_counter;
	pulse_width_reg = state.pulse_width_reg;
	sampleHold = state.sampleHold;
	lastSample = state.lastSample;
	fVolSample = state.fVolSample;
	gate = state.gate;
	test = state.test;
	sidShiftRegister = state.sidShiftRegister;
	keep_zero_volume = state.keep_zero_volume != 0;
	envelope_counter = state.envelope_counter;
	envelope_compare = state.envelope_compare;
	exponential_counter = state.exponential_counter;
	control = state.control;
	shifterTestCounter = state.shifterTestCounter;
	phaseOfShiftRegister = state.phaseOfShiftRegister;
	nextvolume = state.nextvolume;
	samplevolume = state.samplevolume;
	next_envmode = (eEnvelopeMode)state.next_envmode;
	envmode_changing_delay = state.envmode_changing_delay;
	envelope_count_delay = state.envelope_count_delay;
	exponential_count_delay = state.exponential_count_delay;
	next_exponential_counter_period = state.next_exponential_counter_period;
	gotNextVolume = state.gotNextVolume != 0;
	reset_envelope_counter = state.reset_envelope_counter != 0;
	reset_exponential_counter = state.reset_exponential_counter != 0;
	envelope_tick = state.envelope_tick != 0;

	// Fields added to the new version 3.
	want_latched_envmode = state.want_latched_envmode != 0;	
	latched_envmode = (eEnvelopeMode)state.latched_envmode;
	sidShiftRegisterFill = state.sidShiftRegisterFill;
	noiseFeedbackSample1 = state.noiseFeedbackSample1;
	noiseFeedbackSample2 = state.noiseFeedbackSample2;
	noiseFeedbackMask0 = state.noiseFeedbackMask0;
	noiseFeedbackMask1 = state.noiseFeedbackMask1;
	zeroTheShiftRegister  = state.zeroTheShiftRegister != 0;
}

void SIDVoice::UpgradeStateV0ToV1(const SsSidVoice &in, SsSidVoiceV1 &out)
{
	ZeroMemory(&out, sizeof(out));
	out.counter = in.counter;
	out.frequency = in.frequency;
	out.volume = in.volume;
	out.sampleHoldDelayClock = in.sampleHoldDelayClock;
	out.envmode = (eEnvelopeMode) in.envmode;
	out.wavetype = (eWaveType)((in.control) >> 4) & 0xf;
	out.sync = in.sync;
	out.ring_mod = in.ring_mod;
	out.sustain_level = in.sustain_level;
	out.zeroTheCounter = in.zeroTheCounter ? 1 : 0;
	out.exponential_counter_period = in.exponential_counter_period;
	out.attack_value = in.attack_value;
	out.decay_value = in.decay_value;
	out.release_value = in.release_value;
	out.pulse_width_counter = in.pulse_width_counter;
	out.pulse_width_reg = in.pulse_width_reg;
	out.sampleHold = in.sampleHold;
	out.lastSample = in.lastSample;
	out.fVolSample = in.fVolSample;
	out.gate = in.gate;
	out.test = in.test;
	out.sidShiftRegister = in.sidShiftRegister;
	out.keep_zero_volume = in.keep_zero_volume;
	out.envelope_counter = in.envelope_counter;
	out.envelope_compare = in.envelope_compare;
	out.exponential_counter = in.exponential_counter;
	out.control = in.control;
	out.shifterTestCounter = in.shifterTestCounter;
	out.phaseOfShiftRegister = 0;
	out.noiseFeedbackSample1 = in.lastSample;
}

void SIDVoice::UpgradeStateV1ToV2(const SsSidVoiceV1 &in, SsSidVoiceV2 &out)
{
	ZeroMemory(&out, sizeof(out));
	out.counter = in.counter;
	out.frequency = in.frequency;
	out.volume = in.volume;
	out.sampleHoldDelayClock = in.sampleHoldDelayClock;
	out.envmode = (eEnvelopeMode) in.envmode;
	out.wavetype = (eWaveType)((in.control) >> 4) & 0xf;
	out.sync = in.sync;
	out.ring_mod = in.ring_mod;
	out.sustain_level = in.sustain_level;
	out.zeroTheCounter = in.zeroTheCounter;
	out.exponential_counter_period = in.exponential_counter_period;
	out.attack_value = in.attack_value;
	out.decay_value = in.decay_value;
	out.release_value = in.release_value;
	out.pulse_width_counter = in.pulse_width_counter;
	out.pulse_width_reg = in.pulse_width_reg;
	out.sampleHold = in.sampleHold;
	out.lastSample = in.lastSample;
	out.fVolSample = in.fVolSample;
	out.gate = in.gate;
	out.test = in.test;
	out.sidShiftRegister = in.sidShiftRegister;
	out.keep_zero_volume = in.keep_zero_volume;
	out.envelope_counter = in.envelope_counter;
	out.envelope_compare = in.envelope_compare;
	out.exponential_counter = in.exponential_counter;
	out.control = in.control;
	out.shifterTestCounter = in.shifterTestCounter;
	out.phaseOfShiftRegister = 0;
	out.noiseFeedbackSample1 = in.lastSample;

	// Fields added to the new version 2.
	out.nextvolume = (bit8)in.volume;
	out.samplevolume = (bit8)in.volume;
	out.next_envmode = (eEnvelopeMode)in.envmode;
	out.envmode_changing_delay = 0;
	out.envelope_count_delay = 0;
	out.exponential_count_delay = 0;
	out.next_exponential_counter_period = in.exponential_counter_period;
	out.gotNextVolume = 0;
	out.reset_envelope_counter = 0;
	out.reset_exponential_counter = 0;
	out.envelope_tick = 0;		
}

void SIDVoice::UpgradeStateV2ToV3(const SsSidVoiceV2 &in, SsSidVoiceV3 &out)
{
	ZeroMemory(&out, sizeof(out));
	out.counter = in.counter;
	out.frequency = in.frequency;
	out.volume = in.volume;
	out.sampleHoldDelayClock = in.sampleHoldDelayClock;
	out.envmode = (eEnvelopeMode) in.envmode;
	out.wavetype = (eWaveType)((in.control) >> 4) & 0xf;
	out.sync = in.sync;
	out.ring_mod = in.ring_mod;
	out.sustain_level = in.sustain_level;
	out.zeroTheCounter = in.zeroTheCounter;
	out.exponential_counter_period = in.exponential_counter_period;
	out.attack_value = in.attack_value;
	out.decay_value = in.decay_value;
	out.release_value = in.release_value;
	out.pulse_width_counter = in.pulse_width_counter;
	out.pulse_width_reg = in.pulse_width_reg;
	out.sampleHold = in.sampleHold;
	out.lastSample = in.lastSample;
	out.fVolSample = in.fVolSample;
	out.gate = in.gate;
	out.test = in.test;
	out.sidShiftRegister = in.sidShiftRegister;
	out.keep_zero_volume = in.keep_zero_volume;
	out.envelope_counter = in.envelope_counter;
	out.envelope_compare = in.envelope_compare;
	out.exponential_counter = in.exponential_counter;
	out.control = in.control;
	out.shifterTestCounter = in.shifterTestCounter;
	out.phaseOfShiftRegister = 0;
	out.noiseFeedbackSample1 = in.lastSample;

	// Fields added to the new version 2.
	out.nextvolume = (bit8)in.volume;
	out.samplevolume = (bit8)in.volume;
	out.next_envmode = (eEnvelopeMode)in.envmode;
	out.envmode_changing_delay = 0;
	out.envelope_count_delay = 0;
	out.exponential_count_delay = 0;
	out.next_exponential_counter_period = in.exponential_counter_period;
	out.gotNextVolume = 0;
	out.reset_envelope_counter = 0;
	out.reset_exponential_counter = 0;
	out.envelope_tick = 0;

	// Fields added to the new version 3.
	out.want_latched_envmode = 0;
	out.latched_envmode = (eEnvelopeMode)in.envmode;
	out.sidShiftRegisterFill = 0;
	out.noiseFeedbackSample1 = 0;
	out.noiseFeedbackSample2 = 0;
	out.noiseFeedbackMask0 = 0xfff;
	out.noiseFeedbackMask1 = 0x000;
	out.zeroTheShiftRegister = false;
}
