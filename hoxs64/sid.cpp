#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include "boost2005.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "savestate.h"
#include "register.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "filter.h"
#include "sid.h"
#include "sidwavetable.h"

#define SIDBOOST_8580 2.6
#define SIDBOOST_6581 2.0
#define SIDVOLMUL (1730)
#define SHIFTERTESTDELAY (0x49300)
#define SAMPLEHOLDTIME 0x00000fff
#define SAMPLEHOLDRESETTIME 0x001fffff


#define SIDINTERPOLATE2XFIRLENGTH (50)

//Resample from 982800 to 44100L
#define SIDRESAMPLEFIRLENGTH_50 (428 +1)
#define DECIMATION_FACTOR_50 (156)
#define INTERPOLATION_FACTOR_50 (7)

//Resample from 985248 to 44100L
#define SIDRESAMPLEFIRLENGTH_50_12 (74900 +1)
#define DECIMATION_FACTOR_50_12 (27368)
#define INTERPOLATION_FACTOR_50_12 (1225)

#define STAGE1X  25
#define STAGE2X  49

#define DSGAP1 (1L+8L) //1
#define DSGAP2 (8L+8L)
#define DSGAP3 (1L+8L+8L+8L) //2
#define DSGAP4 (8L+8L+8L+8L+8L)

SID64::SID64()
{
	appStatus = NULL;
	MasterVolume = 1.0;
	voice1.sid = this;
	voice2.sid = this;
	voice3.sid = this;
	//m_soundplay_pos=0;
	//m_soundwrite_pos=0;
	bufferLockSize=0;
	bufferSplit=0;
	pBuffer1=NULL;
	pBuffer2=NULL;
	pOverflowBuffer=NULL;
	
	bufferLen1=0;
	bufferLen2=0;
	bufferSampleLen1=0;
	bufferSampleLen2=0;
	overflowBufferSampleLen=0;
	bufferIndex=0;
	overflowBufferIndex = 0;

	sidVolume=0;
	sidFilter=0;
	sidVoice_though_filter=0;
	sidResonance=0;
	sidFilterFrequency=0;
	sidBlock_Voice3=0;
	sidLastWrite=0;
	sidReadDelay=0;
	sidSampler=0;

	m_last_dxsample = 0;
}

SID64::~SID64()
{
	CleanUp();
}

void SID64::CleanUp()
{
	if (appStatus)
		appStatus->m_bFilterOK = false;
	trig.cleanup();
	if (pOverflowBuffer)
	{
		VirtualFree(pOverflowBuffer, 0, MEM_RELEASE);
		pOverflowBuffer = 0;
	}
}

HRESULT SID64::LockSoundBuffer()
{
HRESULT hRet;
DWORD gap;
DWORD soundplay_pos;
DWORD soundwrite_pos;

	hRet = dx->pSecondarySoundBuffer->GetCurrentPosition(&soundplay_pos, &soundwrite_pos);
	if (FAILED(hRet))
		return hRet;

	if (soundwrite_pos > bufferLockPoint) 
	{
		gap = soundBufferSize - soundwrite_pos + bufferLockPoint;		
	}
	else
	{
		gap = bufferLockPoint - soundwrite_pos;
	}

	if (gap < (DSGAP1 * bufferLockSize/8))
	{
		/*audio is getting ahead of us*/
		bufferLockPoint =  (soundwrite_pos + 4 * bufferLockSize) % soundBufferSize;
		//return E_FAIL;
		//bufferLockPoint =  (soundplay_pos + 4 * bufferLockSize) % soundBufferSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
	}
	else if (gap <= (DSGAP2 * bufferLockSize/8))
	{
		//AUDIO_QUICK means to apply a correction using the audio clock sync function to speed up the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_QUICK;
	}
	else if (gap <= (DSGAP3 * bufferLockSize/8))
	{
		/*ideal condition where sound pointer is in the comfort zone*/
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
	}
	else if (gap <= (DSGAP4 * bufferLockSize/8))
	{
		//AUDIO_SLOW means to apply a correction using the audio clock sync function to slow down the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_SLOW;
	}
	else
	{
		/*audio is getting behind of us*/
		bufferLockPoint =  (soundwrite_pos + 4 * bufferLockSize) % soundBufferSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;

	}

	hRet = dx->pSecondarySoundBuffer->Lock(bufferLockPoint, bufferLockSize ,(LPVOID *)&pBuffer1 ,&bufferLen1 ,(LPVOID *)&pBuffer2 ,&bufferLen2 ,0);
	if (FAILED(hRet))
		pBuffer1=NULL;
	else
	{
		bufferSampleLen1 = bufferLen1 / 2;
		bufferSampleLen2 = bufferLen2 / 2;
		bufferIndex=0;
		bufferSplit=0;
		if (overflowBufferIndex>0)
		{
			if (overflowBufferIndex>bufferSampleLen1)
			{
				memcpy_s(pBuffer1, bufferLen1, pOverflowBuffer, bufferLen1);
				if (overflowBufferIndex > (bufferSampleLen1+bufferSampleLen2))
				{
					memcpy_s(pBuffer2, bufferLen2, &pOverflowBuffer[bufferSampleLen1], bufferLen2);
					bufferSplit = 2;
				}
				else
				{
					memcpy_s(pBuffer2, bufferLen2, &pOverflowBuffer[bufferSampleLen1], (overflowBufferIndex-bufferSampleLen1) * 2);
					bufferSplit = 1;
				}
			}
			else
			{
				memcpy_s(pBuffer1, bufferLen1, pOverflowBuffer, overflowBufferIndex * 2);
				bufferIndex = overflowBufferIndex;
			}
		}
		overflowBufferIndex = 0;
	}
	return hRet;
}

void SID64::UnLockSoundBuffer()
{
	if (pBuffer1)
	{
		dx->pSecondarySoundBuffer->Unlock(pBuffer1, bufferLen1, pBuffer2, bufferLen2);
		pBuffer1 = NULL;
	}
}


const WORD SID64::sidAttackRate[16]=
{
/*0*/	9,
/*1*/	32,
/*2*/	63,
/*3*/	95,
/*4*/	149,
/*5*/	220,
/*6*/	267,
/*7*/	313,
/*8*/	392,
/*9*/	977,
/*a*/	1954,
/*b*/	3126,
/*c*/	3907,
/*d*/	11720,
/*e*/	19532,
/*f*/	31251
};

HRESULT SID64::Init(CAppStatus *appStatus, CDX9 *dx, HCFG::EMUFPS fps)
{
HRESULT hr;
	this->appStatus = appStatus;
	this->dx = dx;
			
	appStatus->m_bFilterOK = false;

	CleanUp();


	if (appStatus->m_bSoundOK)
	{		
		bufferLockSize = GetSoundBufferLockSize(fps);
		soundBufferSize = dx->SoundBufferSize;

		//Allocate largest buffer to accommodate both EMUFPS_50 and EMUFPS_50_12
		overflowBufferSampleLen =  GetSoundBufferLockSize(HCFG::EMUFPS_50) / BYTES_PER_SAMPLE;
	}
	else
	{
		bufferLockSize = 0;
		soundBufferSize = 0;
		overflowBufferSampleLen = 0;
	}

	if (overflowBufferSampleLen != 0)
	{
		pOverflowBuffer = (LPWORD)VirtualAlloc(NULL, overflowBufferSampleLen * BYTES_PER_SAMPLE, MEM_COMMIT, PAGE_READWRITE);
		if (!pOverflowBuffer)
			return E_OUTOFMEMORY;
		ZeroMemory(pOverflowBuffer, overflowBufferSampleLen * BYTES_PER_SAMPLE);
	}

	voice1.Init(appStatus);
	voice2.Init(appStatus);
	voice3.Init(appStatus);

	if (trig.init(131072L ) !=0)
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));

	sidSampler=-1;
	bufferLockPoint = 0;

	hr = InitResamplingFilters(fps);
	if (FAILED(hr))
		return SetError(hr, TEXT("InitResamplingFilters Failed."));


	voice1.modulator_voice = &voice3;
	voice2.modulator_voice = &voice1;
	voice3.modulator_voice = &voice2;

	voice3.affected_voice = &voice1;
	voice2.affected_voice = &voice3;
	voice1.affected_voice = &voice2;
	Reset(CurrentClock);
	return S_OK;
}

DWORD SID64::GetSoundBufferLockSize(HCFG::EMUFPS fps)
{
	if (fps == HCFG::EMUFPS_50)
	{
		return (DWORD)ceil((double)(dx->SoundBytesPerSecond) / (double)PAL50FRAMESPERSECOND);
	}
	else
	{
		return (DWORD)ceil((double)(dx->SoundBytesPerSecond) / ((double)PALCLOCKSPERSECOND / ((double)PAL_LINES_PER_FRAME * (double)PAL_CLOCKS_PER_LINE)));
	}
}

DWORD SID64::UpdateSoundBufferLockSize(HCFG::EMUFPS fps)
{
	bufferLockSize = GetSoundBufferLockSize(fps);
	return bufferLockSize;
}


HRESULT SID64::InitResamplingFilters(HCFG::EMUFPS fps)
{
const int INTERPOLATOR_CUTOFF_FREQUENCY = 16000;
const int INTERPOLATOR2X_CUTOFF_FREQUENCY = 14000;

	appStatus->m_bFilterOK = false;
	long interpolatedSamplesPerSecond;
	filterPreFilterResample.CleanSync();
	filterPreFilterStage2.CleanSync();

	if (fps == HCFG::EMUFPS_50)
	{
		filterKernelLength = SIDRESAMPLEFIRLENGTH_50;
		filterInterpolationFactor = INTERPOLATION_FACTOR_50;
		filterDecimationFactor = DECIMATION_FACTOR_50;

		interpolatedSamplesPerSecond = PAL50CLOCKSPERSECOND * filterInterpolationFactor;

		if (filterPreFilterResample.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
			return E_OUTOFMEMORY;

		filterPreFilterResample.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);

	}
#ifdef ALLOW_EMUFPS_50_12_MULTI
	else if (fps == HCFG::EMUFPS_50_12_MULTI)
	{
		filterInterpolationFactor = INTERPOLATION_FACTOR_50_12;
		filterDecimationFactor = DECIMATION_FACTOR_50_12;

		filterPreFilterResample.AllocSync(1528, STAGE1X);
		filterPreFilterStage2.AllocSync(392, STAGE2X);

		filterPreFilterResample.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X);
		filterPreFilterStage2.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X * STAGE2X);
	}
#endif
	else
	{
		filterKernelLength = SIDRESAMPLEFIRLENGTH_50_12;
		filterInterpolationFactor = INTERPOLATION_FACTOR_50_12;
		filterDecimationFactor = DECIMATION_FACTOR_50_12;

		interpolatedSamplesPerSecond = PALCLOCKSPERSECOND * filterInterpolationFactor;

		if (filterPreFilterResample.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
			return E_OUTOFMEMORY;

		filterPreFilterResample.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);
	}

	if (filterUpSample2xSample.AllocSync(SIDINTERPOLATE2XFIRLENGTH, 2) !=0)
		return E_OUTOFMEMORY;
	filterUpSample2xSample.CreateFIRKernel(INTERPOLATOR2X_CUTOFF_FREQUENCY, SAMPLES_PER_SEC * 2);// Twice the sampling frequency.
	

	if (sidSampler >= filterInterpolationFactor)
		sidSampler = filterInterpolationFactor - 1;
	else if (sidSampler < filterDecimationFactor)
		sidSampler = -filterDecimationFactor;

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

	sidSampler=-1;
	appStatus->m_bFilterOK = true;
	return S_OK;
}

void SIDVoice::Reset()
{
	shifterTestCounter=0;
	counter=0;
	volume=0;
	sampleHoldDelay = 0;
	exponential_counter_period=1;
	frequency=0;
	sustain_level=0;
	attack_value=0;
	decay_value=0;
	release_value=0;
	ring_mod=0;
	sync=0;
	zeroTheCounter=0;
	wavetype=sidWAVNONE;
	envmode=sidRELEASE;
	pulse_width_reg=0;
	pulse_width_counter=0;


	gate=0;
	test=0;
	sidShiftRegister=0x7ffff8;
	keep_zero_volume=1;
	envelope_counter=0;
	envelope_compare=0;
	exponential_counter=0;
	control=0;
	lastSample = CalcWave(wavetype);
}

void SID64::InitReset(ICLK sysclock)
{
	CurrentClock = sysclock;
	sidVolume=0;
	sidFilter=0;
	sidVoice_though_filter=0;
	sidResonance=0;
	sidFilterFrequency=0;
	sidBlock_Voice3=0;
	sidLastWrite = 0;
	sidReadDelay = 0;
}

void SID64::Reset(ICLK sysclock)
{
	InitReset(sysclock);

	voice1.Reset();
	voice2.Reset();
	voice3.Reset();

	SetFilter();
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
	t = ((sidShiftRegister >> 22) ^ (sidShiftRegister >> 17)) & 0x1;
	sidShiftRegister <<= 1;
	sidShiftRegister = sidShiftRegister & 0x7fffff;
	sidShiftRegister = sidShiftRegister | t;
}


void SIDVoice::SyncRecheck()
{
	if (zeroTheCounter!=0 && modulator_voice->zeroTheCounter==0)
	{
		counter = 0;
	}	
}

void SIDVoice::Envelope()
{
bit32 t, c;
bit8 ctl;
	if (test)
	{
		shifterTestCounter--;
		if (shifterTestCounter<0)
		{
			shifterTestCounter=SHIFTERTESTDELAY;
			sidShiftRegister = (sidShiftRegister << 1) | 4;
		}
		affected_voice->zeroTheCounter = 0;
	}
	else
	{
		ctl = control;
		if ((ctl & 0x80)!=0 && (ctl & 0x70)!=0)
		{
			sidShiftRegister = sidShiftRegister & 0xAED76B;
		}

		t = counter;
		c = (t + frequency) & 0x00ffffff;
		if ((c & 0x080000) && (!(t & 0x080000))) {
			ClockShiftRegister();
		}
		affected_voice->zeroTheCounter = (affected_voice->sync !=0 && !(t & 0x0800000) && (c & 0x0800000)) != 0;
		counter = c;
	}

	envelope_counter++;
	if ((envelope_counter & 0x7fff) == envelope_compare)
	{
		envelope_counter=0;

		exponential_counter++;
		if (envmode == sidATTACK || exponential_counter == exponential_counter_period)
		{
			exponential_counter = 0;
			
			if (keep_zero_volume)
				return;
			
			switch (envmode)
			{
			case sidENVNONE:
				break;
			case sidATTACK:
				volume = (volume + 1) & 255;
				if (volume == 255)
				{
					envelope_compare = SID64::sidAttackRate[decay_value];
					envmode = sidDECAY;
				}
				break;
			case sidDECAY:
				if (volume != sustain_level)
				{
					volume = (volume - 1) & 255;
				}
				break;
			case sidSUSTAIN:
				break;
			case sidRELEASE:
				volume = (volume - 1) & 255;
				break;
			}
			if (volume == 0)
				keep_zero_volume=1;
		}
		switch (volume)
		{
		case 0x00:
			exponential_counter_period = 0x01;
			break;
		case 0x06:
			exponential_counter_period = 0x1e;
			break;
		case 0x0e:
			exponential_counter_period = 0x10;
			break;
		case 0x1a:
			exponential_counter_period = 0x08;
			break;
		case 0x36:
			exponential_counter_period = 0x04;
			break;
		case 0x5d:
			exponential_counter_period = 0x02;
			break;
		case 0xff:
			exponential_counter_period = 0x01;
			break;
		}
	}
}

unsigned short SIDVoice::CalcWave(bit8 waveType)
{
bool msb;
DWORD dwMsb;

	switch (waveType)
	{
	case sidTRIANGLE:
		if (ring_mod)
			msb = ((modulator_voice->counter ^ counter) & 0x00800000) != 0;
		else
			msb = (counter & 0x00800000) != 0;

		if (msb)
			return (unsigned short)  (((~counter) & 0x007fffff) >> 11);
		else
			return (unsigned short)  ((counter & 0x007fffff) >> 11);
	case sidSAWTOOTH:
		return (unsigned short) (counter >> 12);
	case sidPULSE:
		if(test)
		{
			return 0x0fff;
		}
		else
		{
			if ((counter>>12) >= pulse_width_reg)
			{
				return 0x0fff;
			}
			else
			{
				return 0x0000;
			}
		}
	case sidNOISE:
		 return ((unsigned short)(
			((sidShiftRegister & 0x400000) >> 11) |
			((sidShiftRegister & 0x100000) >> 10) |
			((sidShiftRegister & 0x010000) >> 7) |
			((sidShiftRegister & 0x002000) >> 5) |
			((sidShiftRegister & 0x000800) >> 4) |
			((sidShiftRegister & 0x000080) >> 1) |
			((sidShiftRegister & 0x000010) << 1) |
			((sidShiftRegister & 0x000004) << 2)
			));
	case sidTRIANGLEPULSE:
		if (ring_mod)
			dwMsb = ((modulator_voice->counter ^ counter) & 0x00800000);
		else
			dwMsb = (counter & 0x00800000);

		return CalcWave(sidPULSE) & (unsigned short)sidWave_PT[(unsigned short)  ((dwMsb | (counter & 0x007fffff)) >> 12)] << 4;
	case sidSAWTOOTHPULSE:
		return CalcWave(sidPULSE) & sidWave_PS[counter >> 12] << 4;
	case sidTRIANGLESAWTOOTHPULSE:
		return CalcWave(sidPULSE) & (unsigned short)sidWave_PST[counter >> 12] << 4;
	case sidTRIANGLESAWTOOTH:
		return (unsigned short)sidWave_ST[counter >> 12] << 4;
	case sidWAVNONE:
		if (sampleHoldDelay > 0)
		{
			return sampleHold;
		}
		else
		{
			return 0;
		}
	default:
		return 0;
	}
}

unsigned short SIDVoice::WaveRegister()
{	
	return CalcWave(wavetype);
}

void SIDVoice::Modulate()
{
short sample;
	if (sampleHoldDelay>0)
		sampleHoldDelay--;
	lastSample = CalcWave(wavetype);
	sample = ((short)(lastSample & 0xfff) - 0x800);
	if (appStatus->m_bSidDigiBoost)
	{
		fVolSample = ((double)((long)sample * (long)volume) / (255.0));
		fVolSample += (double)SIDVOLMUL;
		fVolSample = SIDBOOST_6581 * ((fVolSample * (double)sid->sidVolume) / (15.0));
	}
	else
	{
		fVolSample = SIDBOOST_8580 * ((double)((long)sample * (long)volume * (long)sid->sidVolume) / (255.0 * 15.0));
	}
}

void SID64::ExecuteCycle(ICLK sysclock)
{
	if (appStatus->m_bSIDResampleMode)
		ClockSidResample(sysclock);
	else
		ClockSidDownSample(sysclock);
}

void SID64::ClockSid(BOOL bResample, ICLK sysclock)
{
	if (bResample)
		ClockSidResample(sysclock);
	else
		ClockSidDownSample(sysclock);
}

void SID64::ClockSidResample(ICLK sysclock)
{
ICLKS clocks;
double prefilter;
double nofilter;
double voice3nofilter;
double fsample;
double hp_out;
double bp_out;
double lp_out;
short dxsample;
long sampleOffset;


	clocks = (ICLKS)(sysclock - CurrentClock);
	if (clocks >= (ICLKS)sidReadDelay)
		sidReadDelay = 0;
	else
		sidReadDelay -=clocks;
	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;
		voice1.Envelope();
		voice2.Envelope();
		voice3.Envelope();

		voice1.SyncRecheck();
		voice2.SyncRecheck();
		voice3.SyncRecheck();

		if (appStatus->m_bMaxSpeed)
			continue;

		voice1.Modulate();
		voice2.Modulate();
		voice3.Modulate();

		if (sidBlock_Voice3)
		{
			voice3nofilter = 0;
		}
		else
		{
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
		fsample = nofilter;
		svfilterForResample.SVF_ProcessSample(prefilter);
		lp_out = svfilterForResample.lp;
		bp_out = svfilterForResample.bp;
		hp_out = svfilterForResample.hp;
		if (sidFilter & 0x10)
			fsample = fsample - lp_out; 

		if (sidFilter & 0x20)
			fsample = fsample - bp_out;

		if (sidFilter & 0x40)
			fsample = fsample - hp_out;

		sidSampler = sidSampler + filterInterpolationFactor;
		filterPreFilterResample.fir_buffer_sampleNx(fsample);
		if (sidSampler >= 0)
		{
			sampleOffset = (filterInterpolationFactor-1)-sidSampler;
			sidSampler = sidSampler - filterDecimationFactor;

#ifdef ALLOW_EMUFPS_50_12_MULTI
			if (appStatus->m_fps == HCFG::EMUFPS_50_12_MULTI)
			{
				int offset1;
				offset1 = sampleOffset % STAGE1X;
				filterPreFilterResample.FIR_ProcessSampleNx_IndexTo8(offset1, filterPreFilterStage2.buf);

				offset1 = sampleOffset % STAGE2X;
				fsample = filterPreFilterStage2.fir_process_sampleNx_index(offset1);
			}
			else
			{
#endif
				fsample = filterPreFilterResample.fir_process_sampleNx_index(sampleOffset);
#ifdef ALLOW_EMUFPS_50_12_MULTI
			}
#endif

			if (pBuffer1==NULL)
				return;				
			
			fsample = fsample * (double)filterInterpolationFactor * MasterVolume;
			//fsample = fsample * SIDBOOST;

			if (fsample > 32767.0)
				fsample = 32767.0;
			else if (fsample < -32767.0)
				fsample = -32767.0;

			dxsample = (short)(fsample);

			if (pBuffer1!=0)
				WriteSample(dxsample);
		}
	}
}

void SID64::ClockSidDownSample(ICLK sysclock)
{
ICLKS clocks;
double prefilter;
double nofilter;
double voice3nofilter;
double fsample,fsample2;
double hp_out;
double bp_out;
double lp_out;
short dxsample;

	clocks = (ICLKS)(sysclock - CurrentClock);
	if (clocks >= (ICLKS)sidReadDelay)
		sidReadDelay = 0;
	else
		sidReadDelay -=clocks;
	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;
		voice1.Envelope();
		voice2.Envelope();
		voice3.Envelope();

		voice1.SyncRecheck();
		voice2.SyncRecheck();
		voice3.SyncRecheck();

		if (appStatus->m_bMaxSpeed)
			continue;

		sidSampler = sidSampler + filterInterpolationFactor;
		if (sidSampler >= 0)
		{
			sidSampler = sidSampler - filterDecimationFactor;

			voice1.Modulate();
			voice2.Modulate();
			voice3.Modulate();

			if (pBuffer1==NULL)
				return;
			
			if (sidBlock_Voice3)
				voice3nofilter = 0;
			else
				voice3nofilter = voice3.fVolSample;

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
			svfilterForDownSample.SVF_ProcessSample(filterUpSample2xSample.FIR_ProcessSample2x(2.0 * prefilter, &fsample2));
			svfilterForDownSample.SVF_ProcessSample(fsample2);

			fsample = 0.0;
			lp_out = svfilterForDownSample.lp;
			bp_out = svfilterForDownSample.bp;
			hp_out = svfilterForDownSample.hp;
			if (sidFilter & 0x10)
				fsample = fsample - lp_out;

			if (sidFilter & 0x20)
				fsample = fsample - bp_out;

			if (sidFilter & 0x40)
				fsample = fsample - hp_out;
			
			fsample = fsample + nofilter;

			fsample = fsample * MasterVolume;
			//fsample = fsample * SIDBOOST;

			if (fsample > 32767.0)
				fsample = 32767.0;
			else if (fsample < -32767.0)
				fsample = -32767.0;

			dxsample = (WORD)(fsample * MasterVolume);

			if (pBuffer1!=0)
				WriteSample(dxsample);
		}
	}
}

void SID64::WriteSample(short dxsample)
{
	m_last_dxsample = dxsample;
	if (bufferSplit == 0)
	{
		pBuffer1[bufferIndex] = dxsample;
		bufferIndex++;
		if (bufferIndex >= bufferSampleLen1)
		{
			if (pBuffer2)
			{
				bufferSplit = 1;
			}
			else
			{
				bufferSplit = 2;
			}
			bufferIndex = 0;
		}
		bufferLockPoint = (bufferLockPoint + BYTES_PER_SAMPLE) % (soundBufferSize);
	}
	else if (bufferSplit == 1)
	{
		pBuffer2[bufferIndex] = dxsample;
		bufferIndex++;
		if (bufferIndex >= bufferSampleLen2)
		{
			bufferSplit = 2;
			bufferIndex = 0;
		}
		bufferLockPoint = (bufferLockPoint + BYTES_PER_SAMPLE) % (soundBufferSize);
	}
	else if (bufferSplit == 2)
	{
		if (overflowBufferIndex < overflowBufferSampleLen)
		{
			pOverflowBuffer[overflowBufferIndex] = dxsample;
			overflowBufferIndex++;
		}
	}
}

void SIDVoice::SetWave(bit8 data)
{
bit32 t;
	if (data & 8)
	{
		if (test == 0)
		{
			shifterTestCounter = SHIFTERTESTDELAY;
			t = ((sidShiftRegister >> 19) & 0x2) ^ 0x2;
			sidShiftRegister = (sidShiftRegister & 0xfffffd) | t;
		}

		test=1;
		counter=0;
	}
	else
	{
		if (test)
			ClockShiftRegister();
		test=0;
	}

	if (data & 1)
	{
		if (gate==0)
		{
			gate=1;
			envmode = sidATTACK;
			envelope_compare=SID64::sidAttackRate[attack_value];
			keep_zero_volume = 0;
		}
	}
	else
	{
		if (gate!=0)
		{
			gate=0;
			envelope_compare = SID64::sidAttackRate[release_value];
			envmode = sidRELEASE;
		}
	}
	sync=(data & 2);
	ring_mod=(data & 4);
	if ((control ^ data) & 0xf0)//control changing
	{
		switch (data >> 4)
		{
		case 0x0:
			if (wavetype!=sidWAVNONE)
			{
				sampleHoldDelay = SAMPLEHOLDRESETTIME;
				sampleHold = lastSample;
				wavetype = sidWAVNONE;
			}
			break;
		case 0x1:
			wavetype = sidTRIANGLE;
			break;
		case 0x2:
			wavetype = sidSAWTOOTH;
			break;
		case 0x3:
			wavetype = sidTRIANGLESAWTOOTH;
			break;
		case 0x4:
			wavetype = sidPULSE;
			break;
		case 0x5://
			wavetype = sidTRIANGLEPULSE;
			break;
		case 0x6:
			wavetype = sidSAWTOOTHPULSE;
			break;
		case 0x7:
			wavetype = sidTRIANGLESAWTOOTHPULSE;
			break;
		case 0x8:
			wavetype = sidNOISE;
			break;
		case 0x9:
			wavetype = sidNOISETRIANGLE;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xa:
			wavetype = sidNOISESAWTOOTH;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xb:
			wavetype = sidNOISESTRIANGLESAWTOOTH;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xc:
			wavetype = sidNOISEPULSE;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xd:
			wavetype = sidNOISETRIANGLEPULSE;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xe:
			wavetype = sidNOISESAWTOOTHPULSE;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		case 0xf://NPST
			wavetype = sidNOISETRIANGLESAWTOOTHPULSE;
			if (!test)
				sidShiftRegister = sidShiftRegister & 0xAED76B;
			break;
		}
	}
	control = data;
}

void SIDVoice::GetState(SsSidVoice &state)
{
	state.counter = counter;
	state.frequency = frequency;
	state.volume = volume;
	state.sampleHoldDelay = sampleHoldDelay;
	state.envmode = envmode;
	state.wavetype = wavetype;
	state.sync = sync;
	state.ring_mod = ring_mod;
	state.sustain_level = sustain_level;
	state.zeroTheCounter = zeroTheCounter;
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
	state.keep_zero_volume = keep_zero_volume;
	state.envelope_counter = envelope_counter;
	state.envelope_compare = envelope_compare;
	state.exponential_counter = exponential_counter;
	state.control = control;
	state.shifterTestCounter = shifterTestCounter;
}

void SIDVoice::SetState(const SsSidVoice &state)
{
	counter = state.counter;
	frequency = state.frequency;
	volume = state.volume;
	sampleHoldDelay = state.sampleHoldDelay;
	envmode = (eEnvelopeMode) state.envmode;
	wavetype = (eWaveType) state.wavetype;
	sync = state.sync;
	ring_mod = state.ring_mod;
	sustain_level = state.sustain_level;
	zeroTheCounter = state.zeroTheCounter;
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
	keep_zero_volume = state.keep_zero_volume;
	envelope_counter = state.envelope_counter;
	envelope_compare = state.envelope_compare;
	exponential_counter = state.exponential_counter;
	control = state.control;
	shifterTestCounter = state.shifterTestCounter;
}

double SID64::GetCutOff(bit16 sidfreq)
{
	return (double)sidfreq * (5.8) + 30.0;
}

void SID64::SetFilter()
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
		svfilterForDownSample.Set_SVF(cutoff, 2 * SAMPLES_PER_SEC, sidResonance);
	}

}

void SID64::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (appStatus->m_bSID_Emulation_Enable)
		ExecuteCycle(sysclock);

	sidLastWrite = data;
	sidReadDelay = 2000000;
	switch (address & 0x1f)
	{
	case 0x0:
		voice1.frequency = (voice1.frequency & 0xff00) | data;
		break;
	case 0x1:
		voice1.frequency = (voice1.frequency & 0x00ff) | (data<<8);
		break;
	case 0x2:
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0x0f00) | data;
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x3:
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0xff) | ((data & 0xf) <<8);
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x4:
		voice1.SetWave(data);
		break;
	case 0x5:
		voice1.attack_value = data >> 4;
		voice1.decay_value = data & 0xf;
		if (voice1.envmode == sidATTACK)
			voice1.envelope_compare = sidAttackRate[voice1.attack_value];
		else if (voice1.envmode == sidDECAY)
			voice1.envelope_compare = sidAttackRate[voice1.decay_value];
		break;
	case 0x6:
		voice1.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice1.release_value = data & 0xf;
		if (voice1.envmode == sidRELEASE)
			voice1.envelope_compare = sidAttackRate[voice1.release_value];
		break;
	case 0x7:
		voice2.frequency = (voice2.frequency & 0xff00) | data;
		break;
	case 0x8:
		voice2.frequency = (voice2.frequency & 0x00ff) | (data<<8);
		break;
	case 0x9:
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0x0f00) | data;
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xa:
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0xff) | ((data & 0xf)<<8);
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xb:
		voice2.SetWave(data);
		break;
	case 0xc:
		voice2.attack_value = data >> 4;
		voice2.decay_value = data & 0xf;
		if (voice2.envmode == sidATTACK)
			voice2.envelope_compare = sidAttackRate[voice2.attack_value];
		else if (voice2.envmode == sidDECAY)
			voice2.envelope_compare = sidAttackRate[voice2.decay_value];
		break;
	case 0xd:
		voice2.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice2.release_value = data & 0xf;
		if (voice2.envmode == sidRELEASE)
			voice2.envelope_compare = sidAttackRate[voice2.release_value];
		break;
	case 0xe:
		voice3.frequency = (voice3.frequency & 0xff00) | data;
		break;
	case 0xf:
		voice3.frequency = (voice3.frequency & 0x00ff) | (data<<8);
		break;
	case 0x10:
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0x0f00) | data;
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x11:
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0xff) | ((data & 0xf)<<8);
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x12:
		voice3.SetWave(data);
		break;
	case 0x13:
		voice3.attack_value = data >> 4;
		voice3.decay_value = data & 0xf;
		if (voice3.envmode == sidATTACK)
			voice3.envelope_compare = sidAttackRate[voice3.attack_value];
		else if (voice3.envmode == sidDECAY)
			voice3.envelope_compare = sidAttackRate[voice3.decay_value];
		break;
	case 0x14:
		voice3.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice3.release_value = data & 0xf;
		if (voice3.envmode == sidRELEASE)
			voice3.envelope_compare = sidAttackRate[voice3.release_value];
		break;
	case 0x15:
		sidFilterFrequency = (sidFilterFrequency & 0x7f8) | (data & 7);
		SetFilter();
		break;
	case 0x16:
		sidFilterFrequency = (sidFilterFrequency & 7) | (((bit16)data << 3) & 0x7f8);
		SetFilter();
		break;
	case 0x17:
		sidVoice_though_filter = data & 0x7;
		sidResonance = (data & 0xf0) >> 4;
		SetFilter();
		break;
	case 0x18:
		sidVolume = data & 0xf;
		sidFilter = (data & 0x70);
		if ((data & 0x80) !=0)
			sidBlock_Voice3 = 1;
		else
			sidBlock_Voice3 = 0;
		break;
	}
}


bit8 SID64::ReadRegister(bit16 address, ICLK sysclock)
{
	if ((address & 0x1f) == 0x1b)
		sysclock--;
	if (appStatus->m_bSID_Emulation_Enable)
		ExecuteCycle(sysclock);
	switch (address & 0x1f)
	{
	case 0x19:
		return 255;
	case 0x1a:
		return 255;
	case 0x1b:
		return (bit8)(voice3.WaveRegister() >> 4);
	case 0x1c:			
		return (bit8) voice3.volume;
	default:
		if (sidReadDelay==0)
			return 0;
		else
			return sidLastWrite;
	}
}

bit8 SID64::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	if (appStatus->m_bSID_Emulation_Enable)
		ExecuteCycle(sysclock);
	return ReadRegister(address, sysclock);
}

void SID64::SoundHalt(short value)
{
	if (pOverflowBuffer)
	{
		for (DWORD i = 0 ; i < overflowBufferSampleLen ; i++)
		{
			((WORD *)pOverflowBuffer)[i] = value;
		}
	}
}

ICLK SID64::GetCurrentClock()
{
	return CurrentClock;
}

void SID64::SetCurrentClock(ICLK sysclock)
{
	CurrentClock = sysclock;
}

void SID64::GetState(SsSid &state)
{
	ZeroMemory(&state, sizeof(state));
	voice1.GetState(state.voice1);
	voice2.GetState(state.voice2);
	voice3.GetState(state.voice3);
	state.CurrentClock = CurrentClock;
	state.sidVolume = sidVolume;
	state.sidFilter = sidFilter;
	state.sidVoice_though_filter = sidVoice_though_filter;
	state.sidResonance = sidResonance;
	state.sidFilterFrequency = sidFilterFrequency;
	state.sidBlock_Voice3 = sidBlock_Voice3;
	state.sidLastWrite = sidLastWrite;
	state.sidReadDelay = sidReadDelay;
}

void SID64::SetState(const SsSid &state)
{
	voice1.SetState(state.voice1);
	voice2.SetState(state.voice2);
	voice3.SetState(state.voice3);
	CurrentClock = state.CurrentClock;
	sidVolume = state.sidVolume;
	sidFilter = state.sidFilter;
	sidVoice_though_filter = state.sidVoice_though_filter;
	sidResonance = state.sidResonance;
	sidFilterFrequency = state.sidFilterFrequency;
	sidBlock_Voice3 = state.sidBlock_Voice3;
	sidLastWrite = state.sidLastWrite;
	sidReadDelay = state.sidReadDelay;
}
