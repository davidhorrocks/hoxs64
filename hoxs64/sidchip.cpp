#include <windows.h>
#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "savestate.h"
#include "register.h"
#include "hconfig.h"
#include "appstatus.h"
#include "filter.h"
#include "sidchip.h"

#define SIDREADDELAY (110000)
#define SIDREADDELAYFREQ (65000)
#define SIDBOOST_8580 2.6
#define SIDBOOST_6581 2.0
#define SIDVOLMUL (1730)

SidChip::SidChip(int id)
{
	this->id = id;
	sidAddress = 0xD400 + id * 0x20;
	forceMono = false;
	appStatus = NULL;
	sidVolume=0;
	sidFilter=0;
	sidVoice_though_filter=0;
	sidResonance=0;
	sidFilterFrequency=0;
	sidPotX=255;
	sidPotY=255;
	sidBlock_Voice3=false;
	sidInternalBusByte=0;
	sidReadDelayClock=0;

	// Specify ring modulation relationship
	voice1.modulator_voice = &voice3;
	voice2.modulator_voice = &voice1;
	voice3.modulator_voice = &voice2;

	// Specify hard sync relationship
	voice3.affected_voice = &voice1;
	voice2.affected_voice = &voice3;
	voice1.affected_voice = &voice2;

	// Allow the voice objects to see the SID chip current clock cycle.
	voice1.pISidChip = this;
	voice2.pISidChip = this;
	voice3.pISidChip = this;
}

SidChip::~SidChip()
{
	filterUpSample2xSample.CleanSync();
}

HRESULT SidChip::Init(CAppStatus *appStatus, ICia1 *cia1)
{
HRESULT hr;
	this->appStatus = appStatus;
	hr = this->voice1.Init(appStatus);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = this->voice2.Init(appStatus);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = this->voice3.Init(appStatus);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = InitSidFilter();
	return hr;
}

HRESULT SidChip::InitSidFilter()
{
const int INTERPOLATOR2X_CUTOFF_FREQUENCY = 14000;
const int SIDINTERPOLATE2XFIRLENGTH = 50;

	filterUpSample2xSample.CleanSync();
	if (filterUpSample2xSample.AllocSync(SIDINTERPOLATE2XFIRLENGTH, 2) !=0)
	{
		return E_OUTOFMEMORY;
	}

	filterUpSample2xSample.CreateFIRKernel(INTERPOLATOR2X_CUTOFF_FREQUENCY, SID_SOUND_SAMPLES_PER_SEC * 2);// Twice the sampling frequency.	

	svfilterForResample.lp = 0.0;
	svfilterForResample.hp = 0.0;
	svfilterForResample.bp = 0.0;
	svfilterForResample.np = 0.0;
	svfilterForResample.peek = 0.0;

	svfilterForDownSample.lp = 0.0;
	svfilterForDownSample.hp = 0.0;
	svfilterForDownSample.bp = 0.0;
	svfilterForDownSample.np = 0.0;
	svfilterForDownSample.peek = 0.0;

	SetFilter();
	return S_OK;
}

double SidChip::GetCutOff(bit16 sidfreq)
{
	return (double)sidfreq * (5.8) + 30.0;
}

void SidChip::SetFilter()
{
double cutoff;

	cutoff = GetCutOff(sidFilterFrequency);
	if (appStatus->m_bSIDResampleMode)
	{
		if (appStatus->m_fps == HCFG::EMUFPS_50)
		{
			svfilterForResample.Set_SVF(cutoff, PAL50CLOCKSPERSECOND, sidResonance);
		}
		else
		{
			svfilterForResample.Set_SVF(cutoff, PALCLOCKSPERSECOND, sidResonance);
		}
	}
	else
	{
		//The down sample version is feed a doubled sample rate to keep the filter stable below the stable PI/3 cutoff frequency.
		svfilterForDownSample.Set_SVF(cutoff, 2 * SID_SOUND_SAMPLES_PER_SEC, sidResonance);
	}

}

void SidChip::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock = sysclock;
	sidVolume=0;
	sidFilter=0;
	sidVoice_though_filter=0;
	sidResonance=0;
	sidFilterFrequency=0;
	sidPotX=255;
	sidPotY=255;
	sidBlock_Voice3=false;
	sidInternalBusByte = 0;
	sidReadDelayClock = sysclock;
}

void SidChip::ExecuteCycle(ICLK sysclock)
{
	this->CurrentClock = sysclock;

	// Update the SID waveform.
	voice1.Modulate();
	voice2.Modulate();
	voice3.Modulate();

	// Update the SID envelope volume.
	voice1.Envelope();
	voice2.Envelope();
	voice3.Envelope();

	// Handle SID Sync
	voice1.SyncRecheck();
	voice2.SyncRecheck();
	voice3.SyncRecheck();
}

void SidChip::Modulate(ICLK sysclock)
{
	this->CurrentClock = sysclock;

	// Update the SID waveform.
	voice1.Modulate();
	voice2.Modulate();
	voice3.Modulate();
}

void SidChip::Envelope()
{
	// Update the SID envelope volume.
	voice1.Envelope();
	voice2.Envelope();
	voice3.Envelope();

	// Handle SID Sync
	voice1.SyncRecheck();
	voice2.SyncRecheck();
	voice3.SyncRecheck();
}

void SidChip::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);

	voice1.Reset(sysclock, poweronreset);
	voice2.Reset(sysclock, poweronreset);
	voice3.Reset(sysclock, poweronreset);

	SetFilter();
}

double SidChip::GetResample()
{
double prefilter;
double nofilter;
double voice3nofilter;
double fsample;
double hp_out;
double bp_out;
double lp_out;

	if (sidBlock_Voice3)
	{
		// Block unfiltered voice 3.
		// Voice 3 is still allowed through the SID filters.
		voice3nofilter = 0;
	}
	else
	{
		// Enable unfiltered voice 3.
		// Voice 3 may either be filtered or unfilered.
		voice3nofilter = voice3.fVolSample;
	}

	switch (sidVoice_though_filter)
	{
	case 0:
		prefilter = 0;
		nofilter = voice1.fVolSample + voice2.fVolSample + voice3nofilter;
		break;
	case 1:
		prefilter = voice1.fVolSample;
		nofilter = voice2.fVolSample + voice3nofilter;
		break;
	case 2:
		prefilter = voice2.fVolSample;
		nofilter =voice1.fVolSample + voice3nofilter;
		break;
	case 3:
		prefilter = voice1.fVolSample + voice2.fVolSample;
		nofilter = voice3nofilter;
		break;
	case 4:
		prefilter = voice3.fVolSample;
		nofilter = voice1.fVolSample + voice2.fVolSample;
		break;
	case 5:
		prefilter = voice1.fVolSample + voice3.fVolSample;
		nofilter = voice2.fVolSample;
		break;
	case 6:
		prefilter = voice2.fVolSample + voice3.fVolSample;
		nofilter = voice1.fVolSample;
		break;
	case 7:
		prefilter = voice1.fVolSample + voice2.fVolSample + voice3.fVolSample;
		nofilter = 0;
		break;
	}

	// Process a sample through the SID filters.
	fsample = nofilter;
	svfilterForResample.SVF_ProcessNextSample(prefilter);
	lp_out = svfilterForResample.lp;
	bp_out = svfilterForResample.bp;
	hp_out = svfilterForResample.hp;

	// SID low pass filter
	if (sidFilter & 0x10)
	{
		fsample = fsample - lp_out; 
	}

	// SID band pass filter
	if (sidFilter & 0x20)
	{
		fsample = fsample - bp_out;
	}

	// SID high pass filter
	if (sidFilter & 0x40)
	{
		fsample = fsample - hp_out;
	}

	if (appStatus->m_bSidDigiBoost)
	{
		// Fake a DC offset for 3 voices
		fsample += (double)(SIDVOLMUL * 3);

		// Scale the sample with the SID volume.
		fsample = SIDBOOST_6581 * ((fsample * (double)this->sidVolume) / (15.0));
	}
	else
	{
		// Scale the sample with the SID volume.
		fsample = SIDBOOST_8580 * ((double)(fsample * (double)this->sidVolume) / (15.0));
	}

	return fsample;
}

double SidChip::GetDownsample()
{
double prefilter;
double nofilter;
double voice3nofilter;
double fsample;
double fsample2;
double hp_out;
double bp_out;
double lp_out;

	if (sidBlock_Voice3)
	{
		// Block unfiltered voice 3.
		// Voice 3 is still allowed through the SID filters.
		voice3nofilter = 0;
	}
	else
	{
		// Enable unfiltered voice 3.
		// Voice 3 may either be filtered or unfilered.
		voice3nofilter = voice3.fVolSample;
	}

	switch (sidVoice_though_filter)
	{
	case 0:
		prefilter = 0;
		nofilter = voice1.fVolSample + voice2.fVolSample + voice3nofilter;
		break;
	case 1:
		prefilter = voice1.fVolSample;
		nofilter = voice2.fVolSample + voice3nofilter;
		break;
	case 2:
		prefilter = voice2.fVolSample;
		nofilter =voice1.fVolSample + voice3nofilter;
		break;
	case 3:
		prefilter = voice1.fVolSample + voice2.fVolSample;
		nofilter = voice3nofilter;
		break;
	case 4:
		prefilter = voice3.fVolSample;
		nofilter = voice1.fVolSample + voice2.fVolSample;
		break;
	case 5:
		prefilter = voice1.fVolSample + voice3.fVolSample;
		nofilter = voice2.fVolSample;
		break;
	case 6:
		prefilter = voice2.fVolSample + voice3.fVolSample;
		nofilter = voice1.fVolSample;
		break;
	case 7:
		prefilter = voice1.fVolSample + voice2.fVolSample + voice3.fVolSample;
		nofilter = 0;
		break;
	}

	// Process a sample through the SID filters.
	svfilterForDownSample.SVF_ProcessNextSample(filterUpSample2xSample.InterpolateNextSample2x(2.0 * prefilter, &fsample2));
	svfilterForDownSample.SVF_ProcessNextSample(fsample2);

	fsample = nofilter;
	lp_out = svfilterForDownSample.lp;
	bp_out = svfilterForDownSample.bp;
	hp_out = svfilterForDownSample.hp;

	// SID low pass filter
	if (sidFilter & 0x10)
	{
		fsample = fsample - lp_out;
	}

	// SID band pass filter
	if (sidFilter & 0x20)
	{
		fsample = fsample - bp_out;
	}

	// SID high pass filter
	if (sidFilter & 0x40)
	{
		fsample = fsample - hp_out;
	}

	if (appStatus->m_bSidDigiBoost)
	{
		// Fake a DC offset for 3 voices
		fsample += (double)(SIDVOLMUL * 3);

		// Scale the sample with the SID volume.
		fsample = SIDBOOST_6581 * ((fsample * (double)this->sidVolume) / (15.0));
	}
	else
	{
		// Scale the sample with the SID volume.
		fsample = SIDBOOST_8580 * ((double)(fsample * (double)this->sidVolume) / (15.0));
	}

	return fsample;
}

void SidChip::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{	
	sidInternalBusByte = data;
	switch (address & 0x1f)
	{
	case 0x0:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice1.frequency = (voice1.frequency & 0xff00) | data;
		break;
	case 0x1:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice1.frequency = (voice1.frequency & 0x00ff) | (data<<8);
		break;
	case 0x2:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0x0f00) | data;
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x3:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0xff) | ((data & 0xf) <<8);
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x4:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice1.SetWave(data);
		break;
	case 0x5:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice1.attack_value = data >> 4;
		voice1.decay_value = data & 0xf;
		if (voice1.envmode == sidATTACK)
		{
			voice1.envelope_compare = SIDVoice::AdsrTable[voice1.attack_value];
		}
		else if (voice1.envmode == sidDECAY)
		{
			voice1.envelope_compare = SIDVoice::AdsrTable[voice1.decay_value];
		}
		break;
	case 0x6:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice1.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice1.release_value = data & 0xf;
		if (voice1.envmode == sidRELEASE)
		{
			voice1.envelope_compare = SIDVoice::AdsrTable[voice1.release_value];
		}
		break;
	case 0x7:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice2.frequency = (voice2.frequency & 0xff00) | data;
		break;
	case 0x8:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice2.frequency = (voice2.frequency & 0x00ff) | (data<<8);
		break;
	case 0x9:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0x0f00) | data;
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xa:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0xff) | ((data & 0xf)<<8);
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xb:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice2.SetWave(data);
		break;
	case 0xc:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice2.attack_value = data >> 4;
		voice2.decay_value = data & 0xf;
		if (voice2.envmode == sidATTACK)
		{
			voice2.envelope_compare = SIDVoice::AdsrTable[voice2.attack_value];
		}
		else if (voice2.envmode == sidDECAY)
		{
			voice2.envelope_compare = SIDVoice::AdsrTable[voice2.decay_value];
		}
		break;
	case 0xd:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice2.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice2.release_value = data & 0xf;
		if (voice2.envmode == sidRELEASE)
		{
			voice2.envelope_compare = SIDVoice::AdsrTable[voice2.release_value];
		}
		break;
	case 0xe:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice3.frequency = (voice3.frequency & 0xff00) | data;
		break;
	case 0xf:
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		voice3.frequency = (voice3.frequency & 0x00ff) | (((bit32)data)<<8);
		break;
	case 0x10:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0x0f00) | data;
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x11:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0xff) | (((bit32)(data & 0xf))<<8);
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x12:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice3.SetWave(data);
		break;
	case 0x13:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice3.attack_value = data >> 4;
		voice3.decay_value = data & 0xf;
		if (voice3.envmode == sidATTACK)
		{
			voice3.envelope_compare = SIDVoice::AdsrTable[voice3.attack_value];
		}
		else if (voice3.envmode == sidDECAY)
		{
			voice3.envelope_compare = SIDVoice::AdsrTable[voice3.decay_value];
		}
		break;
	case 0x14:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		voice3.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice3.release_value = data & 0xf;
		if (voice3.envmode == sidRELEASE)
		{
			voice3.envelope_compare = SIDVoice::AdsrTable[voice3.release_value];
		}
		break;
	case 0x15:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		sidFilterFrequency = (sidFilterFrequency & 0x7f8) | (data & 7);
		SetFilter();
		break;
	case 0x16:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		sidFilterFrequency = (sidFilterFrequency & 7) | (((bit16)data << 3) & 0x7f8);
		SetFilter();
		break;
	case 0x17:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		sidVoice_though_filter = data & 0x7;
		sidResonance = (data & 0xf0) >> 4;
		SetFilter();
		break;
	case 0x18:
		sidReadDelayClock = sysclock + SIDREADDELAY;
		sidVolume = data & 0xf;
		sidFilter = (data & 0x70);
		if ((data & 0x80) !=0)
		{
			sidBlock_Voice3 = true;
		}
		else
		{
			sidBlock_Voice3 = false;
		}
		break;
	}
}

bit8 SidChip::ReadRegister(bit16 address, ICLK sysclock)
{
bit8 data;

	switch (address & 0x1f)
	{
	case 0x19:
		data = this->Get_PotX();
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		break;
	case 0x1a:
		data = this->Get_PotY();
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		break;
	case 0x1b:
		if (this->voice3.wavetype == sidWAVNONE && (ICLKS)(this->CurrentClock - voice3.sampleHoldDelayClock) >= 0)
		{
			data = 0;
		}
		else
		{
			data = (bit8)(voice3.lastSample >> 4);
		}

		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		break;
	case 0x1c:
		data = (bit8) voice3.samplevolume;
		sidReadDelayClock = sysclock + SIDREADDELAYFREQ;
		break;
	default:
		if ((ICLKS)(sysclock - sidReadDelayClock) >= 0)
		{
			data = 0;
		}
		else
		{
			data = sidInternalBusByte;
		}

		break;
	}

	sidInternalBusByte = data;
	return data;
}

bit8 SidChip::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
bit8 data;

	switch (address & 0x1f)
	{
	case 0x19:
		data = this->Get_PotX();
		break;
	case 0x1a:
		data = this->Get_PotY();
		break;
	case 0x1b:
		data = (bit8)(voice3.lastSample >> 4);
		break;
	case 0x1c:
		data = (bit8) voice3.samplevolume;
		break;
	default:
		if ((ICLKS)(sysclock - sidReadDelayClock) >= 0)
		{
			data = 0;
		}
		else
		{
			data = sidInternalBusByte;
		}

		break;
	}

	return data;
}

bit8 SidChip::Get_PotX()
{
	return this->sidPotX;
}

void SidChip::Set_PotX(ICLK sysclock, bit8 data)
{
	this->sidPotX=data;
}

bit8 SidChip::Get_PotY()
{
	return this->sidPotY;
}

void SidChip::Set_PotY(ICLK sysclock, bit8 data)
{
	this->sidPotY=data;
}

ICLK SidChip::GetCurrentClock()
{
	return CurrentClock;
}

void SidChip::SetCurrentClock(ICLK sysclock)
{
	CurrentClock = sysclock;
}

void SidChip::PreventClockOverflow(ICLK sysclock)
{
	const ICLKS CLOCKSYNCBAND_NEAR = PAL_5_MINUTES;
	const ICLKS CLOCKSYNCBAND_FAR = OVERFLOWSAFTYTHRESHOLD;
	ICLK ClockBehindNear = sysclock - CLOCKSYNCBAND_NEAR;
	if ((ICLKS)(sysclock - this->sidReadDelayClock) >= CLOCKSYNCBAND_FAR)
	{
		this->sidReadDelayClock = ClockBehindNear;
	}

	voice1.PreventClockOverflow(sysclock);
	voice2.PreventClockOverflow(sysclock);
	voice3.PreventClockOverflow(sysclock);
}

void SidChip::GetState(SsSidV4 &state)
{
	ZeroMemory(&state, sizeof(state));
	voice1.GetState(state.voice1);
	voice2.GetState(state.voice2);
	voice3.GetState(state.voice3);
	state.SidNumber = id;
	state.SidAddress = sidAddress;
	state.CurrentClock = CurrentClock;
	state.sidVolume = sidVolume;
	state.sidFilter = sidFilter;
	state.sidVoice_though_filter = sidVoice_though_filter;
	state.sidResonance = sidResonance;
	state.sidFilterFrequency = sidFilterFrequency;
	state.sidBlock_Voice3 = sidBlock_Voice3 ? 1 : 0;
	state.sidInternalBusByte = sidInternalBusByte;
	state.sidReadDelayClock = sidReadDelayClock;
}

void SidChip::SetState(const SsSidV4 &state)
{
	id = state.SidNumber;
	sidAddress = state.SidAddress;
	CurrentClock = state.CurrentClock;
	voice1.SetState(state.CurrentClock, state.voice1);
	voice2.SetState(state.CurrentClock, state.voice2);
	voice3.SetState(state.CurrentClock, state.voice3);
	sidVolume = state.sidVolume;
	sidFilter = state.sidFilter;
	sidVoice_though_filter = state.sidVoice_though_filter;
	sidResonance = state.sidResonance;
	sidFilterFrequency = state.sidFilterFrequency;
	sidBlock_Voice3 = state.sidBlock_Voice3 != 0;
	sidInternalBusByte = state.sidInternalBusByte;
	sidReadDelayClock = state.sidReadDelayClock;
	SetFilter();
}

void SidChip::UpgradeStateV0ToV1(const SsSid &in, SsSidV1 &out)
{
	ZeroMemory(&out, sizeof(out));	
	SIDVoice::UpgradeStateV0ToV1(in.voice1, out.voice1);
	SIDVoice::UpgradeStateV0ToV1(in.voice2, out.voice2);
	SIDVoice::UpgradeStateV0ToV1(in.voice3, out.voice3);
	out.CurrentClock = in.CurrentClock;
	out.sidVolume = in.sidVolume;
	out.sidFilter = in.sidFilter;
	out.sidVoice_though_filter = in.sidVoice_though_filter;
	out.sidResonance = in.sidResonance;
	out.sidFilterFrequency = in.sidFilterFrequency;
	out.sidBlock_Voice3 = in.sidBlock_Voice3;
	out.sidInternalBusByte = in.sidInternalBusByte;
	out.sidReadDelay = in.sidReadDelay;
}

void SidChip::UpgradeStateV1ToV2(const SsSidV1 &in, SsSidV2 &out)
{
	ZeroMemory(&out, sizeof(out));	
	SIDVoice::UpgradeStateV1ToV2(in.voice1, out.voice1);
	SIDVoice::UpgradeStateV1ToV2(in.voice2, out.voice2);
	SIDVoice::UpgradeStateV1ToV2(in.voice3, out.voice3);
	out.CurrentClock = in.CurrentClock;
	out.sidVolume = in.sidVolume;
	out.sidFilter = in.sidFilter;
	out.sidVoice_though_filter = in.sidVoice_though_filter;
	out.sidResonance = in.sidResonance;
	out.sidFilterFrequency = in.sidFilterFrequency;
	out.sidBlock_Voice3 = in.sidBlock_Voice3;
	out.sidInternalBusByte = in.sidInternalBusByte;
	out.sidReadDelay = in.sidReadDelay;
}

void SidChip::UpgradeStateV2ToV3(const SsSidV2 &in, SsSidV3 &out)
{
	ZeroMemory(&out, sizeof(out));	
	SIDVoice::UpgradeStateV2ToV3(in.voice1, out.voice1);
	SIDVoice::UpgradeStateV2ToV3(in.voice2, out.voice2);
	SIDVoice::UpgradeStateV2ToV3(in.voice3, out.voice3);
	out.CurrentClock = in.CurrentClock;
	out.sidVolume = in.sidVolume;
	out.sidFilter = in.sidFilter;
	out.sidVoice_though_filter = in.sidVoice_though_filter;
	out.sidResonance = in.sidResonance;
	out.sidFilterFrequency = in.sidFilterFrequency;
	out.sidBlock_Voice3 = in.sidBlock_Voice3;
	out.sidInternalBusByte = in.sidInternalBusByte;
	out.sidReadDelay = in.sidReadDelay;
}

void SidChip::UpgradeStateV3ToV4(const SsSidV3 &in, SsSidV4 &out)
{
	ZeroMemory(&out, sizeof(out));	
	SIDVoice::UpgradeStateV2ToV3(in.voice1, out.voice1);
	SIDVoice::UpgradeStateV2ToV3(in.voice2, out.voice2);
	SIDVoice::UpgradeStateV2ToV3(in.voice3, out.voice3);
	out.CurrentClock = in.CurrentClock;
	out.sidVolume = in.sidVolume;
	out.sidFilter = in.sidFilter;
	out.sidVoice_though_filter = in.sidVoice_though_filter;
	out.sidResonance = in.sidResonance;
	out.sidFilterFrequency = in.sidFilterFrequency;
	out.sidBlock_Voice3 = in.sidBlock_Voice3;
	out.sidInternalBusByte = in.sidInternalBusByte;
	out.sidReadDelayClock = in.sidReadDelay + in.CurrentClock;
	out.SidAddress = 0xD400;
	out.SidNumber = 0;
}
