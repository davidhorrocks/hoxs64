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
#define SAMPLEHOLDRESETTIME  0x001D0000

//define SIDREADDELAY (2000000)
//define SIDREADDELAYFREQ (65000)
#define SIDREADDELAY (110000)
#define SIDREADDELAYFREQ (65000)

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

#define DSGAP1 (1L+8L)
#define DSGAP2 (8L+8L)
#define DSGAP3 (1L+8L+8L+8L)
#define DSGAP4 (8L+8L+8L+8L+8L)

#define NOISE_ZERO_FEED ((~((1U << 20) | (1 << 18) | (1 << 14) | (1 << 11) | (1 << 9) | (1 << 5) | (1 << 2) | (1 << 0))) & 0x7fffff)
#define ENVELOPE_LFSR_RESET (0x7fff)

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
	sidInternalBusByte=0;
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

	if (appStatus->m_bMaxSpeed)
	{
		return E_FAIL;
	}

	hRet = dx->pSecondarySoundBuffer->GetCurrentPosition(&soundplay_pos, &soundwrite_pos);
	if (FAILED(hRet))
	{
		return hRet;
	}

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
		bufferLockPoint =  (soundwrite_pos + 3 * bufferLockSize) % soundBufferSize;
		//return E_FAIL;
		//bufferLockPoint =  (soundplay_pos + 4 * bufferLockSize) % soundBufferSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 0;
	}
	else if (gap <= (DSGAP2 * bufferLockSize/8))
	{
		//AUDIO_QUICK means to apply a correction using the audio clock sync function to speed up the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_QUICK;
		currentAudioSyncState = 1;
	}
	else if (gap <= (DSGAP3 * bufferLockSize/8))
	{
		/*ideal condition where sound pointer is in the comfort zone*/
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 2;
	}
	else if (gap <= (DSGAP4 * bufferLockSize/8))
	{
		//AUDIO_SLOW means to apply a correction using the audio clock sync function to slow down the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_SLOW;
		currentAudioSyncState = 3;
	}
	else
	{
		/*audio is getting behind of us*/
		bufferLockPoint =  (soundwrite_pos + 4 * bufferLockSize) % soundBufferSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 4;
	}

#if defined(DEBUG)
	if (lastAudioSyncState != currentAudioSyncState)
	{
		lastAudioSyncState = currentAudioSyncState;
		switch (currentAudioSyncState)
		{
		case 0:
			OutputDebugString(TEXT("AUDIO_QUICK2\n"));
			break;
		case 1:
			OutputDebugString(TEXT("AUDIO_QUICK\n"));
			break;
		case 2:
			OutputDebugString(TEXT("AUDIO_OK\n"));
			break;
		case 3:
			OutputDebugString(TEXT("AUDIO_SLOW\n"));
			break;
		case 4:
			OutputDebugString(TEXT("AUDIO_SLOW2\n"));
			break;
		}
	}
#endif

	hRet = dx->pSecondarySoundBuffer->Lock(bufferLockPoint, bufferLockSize ,(LPVOID *)&pBuffer1 ,&bufferLen1 ,(LPVOID *)&pBuffer2 ,&bufferLen2 ,0);
	if (FAILED(hRet))
	{
		pBuffer1=NULL;
	}
	else
	{
		bufferSampleLen1 = bufferLen1 / 2;
		bufferSampleLen2 = bufferLen2 / 2;
		bufferIndex=0;
		bufferSplit=0;
		DWORD bytesWritten = 0;
		DWORD bytesToWrite;
		if (overflowBufferIndex > 0)
		{
			if (overflowBufferIndex >= bufferSampleLen1)
			{
				memcpy_s(pBuffer1, bufferLen1, pOverflowBuffer, bufferLen1);
				bytesWritten += bufferLen1;
				if (overflowBufferIndex >= (bufferSampleLen1 + bufferSampleLen2))
				{
					memcpy_s(pBuffer2, bufferLen2, &pOverflowBuffer[bufferSampleLen1], bufferLen2);
					bufferSplit = 2;
					bytesWritten += bufferLen2;
				}
				else
				{
					bytesToWrite = (overflowBufferIndex - bufferSampleLen1) * BYTES_PER_SAMPLE;
					memcpy_s(pBuffer2, bufferLen2, &pOverflowBuffer[bufferSampleLen1], bytesToWrite);
					bufferSplit = 1;
					bytesWritten += bytesToWrite;
				}

				bufferIndex = bytesWritten / BYTES_PER_SAMPLE;
			}
			else
			{
				bytesToWrite = overflowBufferIndex * BYTES_PER_SAMPLE;
				memcpy_s(pBuffer1, bufferLen1, pOverflowBuffer, bytesToWrite);
				bufferIndex = overflowBufferIndex;
				bytesWritten += bytesToWrite;
			}
		}

		overflowBufferIndex = 0;
		bufferLockPoint = (bufferLockPoint + bytesWritten) % (soundBufferSize);
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


const bit16 SID64::AdsrTable[16] = {
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
	Reset(CurrentClock, true);
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

void SIDVoice::Reset(bool poweronreset)
{
	if (poweronreset)
	{
		counter=0x555555;
	}
	shifterTestCounter=0;
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
	sidShiftRegister=0x7fffff;
	phaseOfShiftRegister = 0;
	keep_zero_volume=1;
	envelope_counter=ENVELOPE_LFSR_RESET;
	envelope_compare=SID64::AdsrTable[0];
	exponential_counter=0;
	control=0;
	lastSample = CalcWaveOutput(wavetype, lastSampleNoNoise);
}

void SID64::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock = sysclock;
	sidVolume=0;
	sidFilter=0;
	sidVoice_though_filter=0;
	sidResonance=0;
	sidFilterFrequency=0;
	sidBlock_Voice3=0;
	sidInternalBusByte = 0;
	sidReadDelay = 0;
}

void SID64::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);

	voice1.Reset(poweronreset);
	voice2.Reset(poweronreset);
	voice3.Reset(poweronreset);

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
	if (test)
	{
		t = ((sidShiftRegister >> 17) & 0x1) ^ 0x1;
		sidShiftRegister = ((sidShiftRegister << 1) | t) & 0x7fffff;
	}
	else
	{
		t = ((sidShiftRegister >> 22) ^ (sidShiftRegister >> 17)) & 0x1;
		sidShiftRegister = ((sidShiftRegister << 1) | t) & 0x7fffff;
	}
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
	if (test)
	{
		phaseOfShiftRegister = 0;
		shifterTestCounter--;
		if (shifterTestCounter<0)
		{
			shifterTestCounter=SHIFTERTESTDELAY;
			sidShiftRegister = (sidShiftRegister << 1) | 1;
		}
		affected_voice->zeroTheCounter = 0;
	}
	else
	{
		if (phaseOfShiftRegister)
		{
			switch(phaseOfShiftRegister)
			{
			case 1:
				NoiseWriteback(this->control);
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
			NoiseWriteback(this->control);
		}

		t = counter;
		c = (t + frequency) & 0x00ffffff;
		if ((c & 0x080000) && (!(t & 0x080000))) {
			phaseOfShiftRegister = 1;
		}
		affected_voice->zeroTheCounter = (affected_voice->sync !=0 && !(t & 0x0800000) && (c & 0x0800000)) != 0;
		counter = c;
	}

	bit16 feedback = ((envelope_counter >> 14) ^ (envelope_counter >> 13)) & 1;
	if (envelope_counter == envelope_compare)
	{
		envelope_counter=ENVELOPE_LFSR_RESET;
		exponential_counter++;
		if (envmode == sidATTACK || exponential_counter == exponential_counter_period)
		{
			exponential_counter = 0;			
			if (keep_zero_volume)
			{
				return;
			}

			switch (envmode)
			{
			case sidENVNONE:
				break;
			case sidATTACK:
				volume = (volume + 1) & 255;
				if (volume == 255)
				{
					envelope_compare = SID64::AdsrTable[decay_value];
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
			{
				keep_zero_volume=1;
			}
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
	else
	{
		envelope_counter = ((envelope_counter << 1) & 0x7ffe) | feedback;
	}
}

bit16 SIDVoice::CalcWaveOutput(bit8 waveType, bit16 &waveNoNoise)
{
bool msb;
DWORD dwMsb;
bit16 wave;
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
		waveNoNoise = wave;
		return wave;
	case sidSAWTOOTH:
		wave = waveNoNoise = (unsigned short) (counter >> 12);
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
		waveNoNoise = wave;
		return wave;
	case sidNOISE:
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
		wave = CalcWaveOutput(sidPULSE, waveNoNoise) & (unsigned short)sidWave_PT[(unsigned short)  ((dwMsb | (counter & 0x007fffff)) >> 12)] << 4;
		waveNoNoise = wave;
		return wave;
	case sidSAWTOOTHPULSE:
		wave = CalcWaveOutput(sidPULSE, waveNoNoise) & sidWave_PS[counter >> 12] << 4;
		waveNoNoise = wave;
		return wave;
	case sidTRIANGLESAWTOOTHPULSE:
		wave = CalcWaveOutput(sidPULSE, waveNoNoise) & (unsigned short)sidWave_PST[counter >> 12] << 4;
		waveNoNoise = wave;
		return wave;
	case sidTRIANGLESAWTOOTH:
		wave = (unsigned short)sidWave_ST[counter >> 12] << 4;
		waveNoNoise = wave;
		return wave;
	case sidWAVNONE:
		if (sampleHoldDelay > 0)
		{
			waveNoNoise = sampleHold;
			return sampleHold;
		}
		else
		{
			waveNoNoise = 0;
			return 0;
		}
	case sidNOISETRIANGLE:
		wave = CalcWaveOutput(sidTRIANGLE, waveNoNoise);
		wave &= ShiftRegisterOutput();
		return wave;
	case sidNOISESAWTOOTH:
		wave = CalcWaveOutput(sidSAWTOOTH, waveNoNoise);
		wave &= ShiftRegisterOutput();
		return wave;
	case sidNOISEPULSE:
		wave = CalcWaveOutput(sidPULSE, waveNoNoise);
		wave &= ShiftRegisterOutput();
		return wave;
	default:
		waveNoNoise = 0;
		return 0;
	}
}

bit16 SIDVoice::ShiftRegisterOutput()
{
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

bit16 SIDVoice::WaveRegister()
{	
bit16 d;
	return CalcWaveOutput(wavetype, d);
}

void SIDVoice::Modulate()
{
short sample;
	if (sampleHoldDelay>0)
	{
		sampleHoldDelay--;
	}
	lastSample = CalcWaveOutput(wavetype, lastSampleNoNoise);
	sample = ((short)(lastSample & 0xfff) - 0x800);
	fVolSample = ((double)((long)sample * (long)volume) / (255.0));
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
		{
			continue;
		}

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
		{
			fsample = fsample - lp_out; 
		}
		if (sidFilter & 0x20)
		{
			fsample = fsample - bp_out;
		}
		if (sidFilter & 0x40)
		{
			fsample = fsample - hp_out;
		}

		if (appStatus->m_bSidDigiBoost)
		{
			fsample += (double)(SIDVOLMUL * 3);
			fsample = SIDBOOST_6581 * ((fsample * (double)this->sidVolume) / (15.0));
		}
		else
		{
			fsample = SIDBOOST_8580 * ((double)(fsample * (double)this->sidVolume) / (15.0));
		}
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
			{
				return;				
			}

			fsample = fsample * (double)filterInterpolationFactor * MasterVolume;
			if (fsample > 32767.0)
			{
				fsample = 32767.0;
			}
			else if (fsample < -32767.0)
			{
				fsample = -32767.0;
			}

			dxsample = (short)(fsample);
			if (pBuffer1!=0)
			{
				WriteSample(dxsample);
			}
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
			{
				fsample = fsample - lp_out;
			}
			if (sidFilter & 0x20)
			{
				fsample = fsample - bp_out;
			}
			if (sidFilter & 0x40)
			{
				fsample = fsample - hp_out;
			}
			fsample = fsample + nofilter;
			if (appStatus->m_bSidDigiBoost)
			{
				fsample += (double)(SIDVOLMUL * 3);
				fsample = SIDBOOST_6581 * ((fsample * (double)this->sidVolume) / (15.0));
			}
			else
			{
				fsample = SIDBOOST_8580 * ((double)(fsample * (double)this->sidVolume) / (15.0));
			}

			fsample = fsample * MasterVolume;
			if (fsample > 32767.0)
			{
				fsample = 32767.0;
			}
			else if (fsample < -32767.0)
			{
				fsample = -32767.0;
			}

			dxsample = (WORD)(fsample);
			if (pBuffer1!=0)
			{
				WriteSample(dxsample);
			}
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
			sampleHoldDelay = SAMPLEHOLDRESETTIME;
			sampleHold = lastSample;
			wavetype = sidWAVNONE;
		}
	}
	if (new_control & 8)
	{
		if (test == 0)
		{
			phaseOfShiftRegister = 0;
			shifterTestCounter = SHIFTERTESTDELAY;
			if ((new_control & 0x80) != 0 && (new_control & 0x70) != 0)
			{
				NoiseWriteback(prev_control);
			}
			test=1;
		}
		
		counter=0;
	}
	else
	{
		if (test)
		{
			if ((new_control & 0x80) != 0 && (new_control & 0x70) != 0)
			{
				NoiseWriteback(prev_control);
			}
			ClockShiftRegister();
			phaseOfShiftRegister = 2;
			test=0;
		}
	}

	if (new_control & 1)
	{
		if (gate==0)
		{
			gate=1;
			envmode = sidATTACK;
			envelope_compare=SID64::AdsrTable[attack_value];
			keep_zero_volume = 0;
		}
	}
	else
	{
		if (gate!=0)
		{
			gate=0;
			envelope_compare = SID64::AdsrTable[release_value];
			envmode = sidRELEASE;
		}
	}
}

void SIDVoice::NoiseWriteback(bit8 control)
{
	if ((control & 0x80) != 0 && (control & 0x70) != 0)
	{	
		bit16 s;
		bit32 t;
		this->CalcWaveOutput((control>>4) & 0xf, s);
		t = (bit32)s;
		t = ((t & 0x010) >> 4) | 
			((t & 0x020) >> 3) |
			((t & 0x040) >> 1) |
			((t & 0x080) << 2) |
			((t & 0x100) << 3) |
			((t & 0x200) << 5) |
			((t & 0x400) << 8) |
			((t & 0x800) << 9);
		t |= this->lastSampleNoNoise;
		sidShiftRegister = (sidShiftRegister & (t | NOISE_ZERO_FEED));
	}	
}

void SIDVoice::GetState(SsSidVoiceV1 &state)
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

void SIDVoice::SetState(const SsSidVoiceV1 &state)
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
	shifterTestCounter = state.phaseOfShiftRegister;
	shifterTestCounter = state.lastSampleNoNoise;
}

void SIDVoice::UpgradeStateV0ToV1(const SsSidVoice &in, SsSidVoiceV1 &out)
{
	ZeroMemory(&out, sizeof(out));
	out.counter = in.counter;
	out.frequency = in.frequency;
	out.volume = in.volume;
	out.sampleHoldDelay = in.sampleHoldDelay;
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
	out.lastSampleNoNoise = in.lastSample;
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
	{
		ExecuteCycle(sysclock);
	}
	sidInternalBusByte = data;
	switch (address & 0x1f)
	{
	case 0x0:
		sidReadDelay = SIDREADDELAYFREQ;
		voice1.frequency = (voice1.frequency & 0xff00) | data;
		break;
	case 0x1:
		sidReadDelay = SIDREADDELAYFREQ;
		voice1.frequency = (voice1.frequency & 0x00ff) | (data<<8);
		break;
	case 0x2:
		sidReadDelay = SIDREADDELAYFREQ;
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0x0f00) | data;
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x3:
		sidReadDelay = SIDREADDELAYFREQ;
		voice1.pulse_width_reg = (voice1.pulse_width_reg & 0xff) | ((data & 0xf) <<8);
		voice1.pulse_width_counter = voice1.pulse_width_reg << 12;
		break;
	case 0x4:
		sidReadDelay = SIDREADDELAY;
		voice1.SetWave(data);
		break;
	case 0x5:
		sidReadDelay = SIDREADDELAY;
		voice1.attack_value = data >> 4;
		voice1.decay_value = data & 0xf;
		if (voice1.envmode == sidATTACK)
		{
			voice1.envelope_compare = AdsrTable[voice1.attack_value];
		}
		else if (voice1.envmode == sidDECAY)
		{
			voice1.envelope_compare = AdsrTable[voice1.decay_value];
		}
		break;
	case 0x6:
		sidReadDelay = SIDREADDELAY;
		voice1.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice1.release_value = data & 0xf;
		if (voice1.envmode == sidRELEASE)
		{
			voice1.envelope_compare = AdsrTable[voice1.release_value];
		}
		break;
	case 0x7:
		sidReadDelay = SIDREADDELAYFREQ;
		voice2.frequency = (voice2.frequency & 0xff00) | data;
		break;
	case 0x8:
		sidReadDelay = SIDREADDELAYFREQ;
		voice2.frequency = (voice2.frequency & 0x00ff) | (data<<8);
		break;
	case 0x9:
		sidReadDelay = SIDREADDELAYFREQ;
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0x0f00) | data;
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xa:
		sidReadDelay = SIDREADDELAYFREQ;
		voice2.pulse_width_reg = (voice2.pulse_width_reg & 0xff) | ((data & 0xf)<<8);
		voice2.pulse_width_counter = voice2.pulse_width_reg << 12;
		break;
	case 0xb:
		sidReadDelay = SIDREADDELAY;
		voice2.SetWave(data);
		break;
	case 0xc:
		sidReadDelay = SIDREADDELAY;
		voice2.attack_value = data >> 4;
		voice2.decay_value = data & 0xf;
		if (voice2.envmode == sidATTACK)
		{
			voice2.envelope_compare = AdsrTable[voice2.attack_value];
		}
		else if (voice2.envmode == sidDECAY)
		{
			voice2.envelope_compare = AdsrTable[voice2.decay_value];
		}
		break;
	case 0xd:
		sidReadDelay = SIDREADDELAY;
		voice2.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice2.release_value = data & 0xf;
		if (voice2.envmode == sidRELEASE)
		{
			voice2.envelope_compare = AdsrTable[voice2.release_value];
		}
		break;
	case 0xe:
		sidReadDelay = SIDREADDELAYFREQ;
		voice3.frequency = (voice3.frequency & 0xff00) | data;
		break;
	case 0xf:
		sidReadDelay = SIDREADDELAYFREQ;
		voice3.frequency = (voice3.frequency & 0x00ff) | (((bit32)data)<<8);
		break;
	case 0x10:
		sidReadDelay = SIDREADDELAY;
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0x0f00) | data;
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x11:
		sidReadDelay = SIDREADDELAY;
		voice3.pulse_width_reg = (voice3.pulse_width_reg & 0xff) | (((bit32)(data & 0xf))<<8);
		voice3.pulse_width_counter = voice3.pulse_width_reg << 12;
		break;
	case 0x12:
		sidReadDelay = SIDREADDELAY;
		voice3.SetWave(data);
		break;
	case 0x13:
		sidReadDelay = SIDREADDELAY;
		voice3.attack_value = data >> 4;
		voice3.decay_value = data & 0xf;
		if (voice3.envmode == sidATTACK)
		{
			voice3.envelope_compare = AdsrTable[voice3.attack_value];
		}
		else if (voice3.envmode == sidDECAY)
		{
			voice3.envelope_compare = AdsrTable[voice3.decay_value];
		}
		break;
	case 0x14:
		sidReadDelay = SIDREADDELAY;
		voice3.sustain_level = (((data & 0xf0) >> 4) * 17);
		voice3.release_value = data & 0xf;
		if (voice3.envmode == sidRELEASE)
		{
			voice3.envelope_compare = AdsrTable[voice3.release_value];
		}
		break;
	case 0x15:
		sidReadDelay = SIDREADDELAY;
		sidFilterFrequency = (sidFilterFrequency & 0x7f8) | (data & 7);
		SetFilter();
		break;
	case 0x16:
		sidReadDelay = SIDREADDELAY;
		sidFilterFrequency = (sidFilterFrequency & 7) | (((bit16)data << 3) & 0x7f8);
		SetFilter();
		break;
	case 0x17:
		sidReadDelay = SIDREADDELAY;
		sidVoice_though_filter = data & 0x7;
		sidResonance = (data & 0xf0) >> 4;
		SetFilter();
		break;
	case 0x18:
		sidReadDelay = SIDREADDELAY;
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
bit8 data;
	if ((address & 0x1f) == 0x1b)
	{
		sysclock--;
	}
	if (appStatus->m_bSID_Emulation_Enable)
	{
		ExecuteCycle(sysclock);
	}
	switch (address & 0x1f)
	{
	case 0x19:
		data = 255;
		sidReadDelay = SIDREADDELAYFREQ;
		break;
	case 0x1a:
		data = 255;
		sidReadDelay = SIDREADDELAYFREQ;
		break;
	case 0x1b:
		data = (bit8)(voice3.WaveRegister() >> 4);
		sidReadDelay = SIDREADDELAYFREQ;
		break;
	case 0x1c:
		data = (bit8) voice3.volume;
		sidReadDelay = SIDREADDELAYFREQ;
		break;
	default:
		if (sidReadDelay==0)
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

bit8 SID64::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
bit8 data;
	if (appStatus->m_bSID_Emulation_Enable)
	{
		ExecuteCycle(sysclock);
	}
	switch (address & 0x1f)
	{
	case 0x19:
		data = 255;
		break;
	case 0x1a:
		data = 255;
		break;
	case 0x1b:
		data = (bit8)(voice3.WaveRegister() >> 4);
		break;
	case 0x1c:
		data = (bit8) voice3.volume;
		break;
	default:
		if (sidReadDelay==0)
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

void SID64::GetState(SsSidV1 &state)
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
	state.sidInternalBusByte = sidInternalBusByte;
	state.sidReadDelay = sidReadDelay;
}

void SID64::SetState(const SsSidV1 &state)
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
	sidInternalBusByte = state.sidInternalBusByte;
	sidReadDelay = state.sidReadDelay;
}

void SID64::UpgradeStateV0ToV1(const SsSid &in, SsSidV1 &out)
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