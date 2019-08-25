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

const double Max16BitSample = 32767.0;
const double Min16BitSample = -32768.0;

SID64::SID64()
	: sid1(0), sid2(1), sid3(2), sid4(3), sid5(4), sid6(5), sid7(6), sid8(7)
{
	SetSidChipAddressMap(0, 0, 0, 0, 0, 0, 0, 0);
	appStatus = NULL;
	MasterVolume = 1.0;
	bufferLockByteSize=0;
	bufferSplit=0;
	pBuffer1=NULL;
	pBuffer2=NULL;
	pOverflowBuffer=NULL;
	
	bufferByteLen1=0;
	bufferByteLen2=0;
	bufferSampleBlockLen1=0;
	bufferSampleBlockLen2=0;
	overflowBufferSampleLen=0;
	bufferSampleBlockIndex=0;
	overflowBufferBlockIndex = 0;
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
	{
		appStatus->m_bFilterOK = false;
	}

	trig.cleanup();
	if (pOverflowBuffer)
	{
		VirtualFree(pOverflowBuffer, 0, MEM_RELEASE);
		pOverflowBuffer = 0;
	}
}

void SID64::SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID)
{
	SidChip *sids[8] = {&this->sid1, &this->sid2, &this->sid3, &this->sid4, &this->sid5, &this->sid6, &this->sid7, &this->sid8};
	sid1.sidAddress = 0xD400;
	sid1.active = true;
	for (int i = 1; i < _countof(sids); i++)
	{
		sids[i]->active = numberOfExtraSidChips >= i;
	}

	this->NumberOfExtraSidChips = numberOfExtraSidChips;
	this->AddressOfSecondSID = addressOfSecondSID;
	sid2.sidAddress = addressOfSecondSID;
	this->AddressOfThirdSID = addressOfThirdSID;
	sid3.sidAddress = addressOfThirdSID;
	this->AddressOfFourthSID = addressOfFourthSID;
	sid4.sidAddress = addressOfFourthSID;
	this->AddressOfFifthSID = addressOfFifthSID;
	sid5.sidAddress = addressOfFifthSID;
	this->AddressOfSixthSID = addressOfSixthSID;
	sid6.sidAddress = addressOfSixthSID;
	this->AddressOfSeventhSID = addressOfSeventhSID;
	sid7.sidAddress = addressOfSeventhSID;
	this->AddressOfEighthSID = addressOfEighthSID;
	sid8.sidAddress = addressOfEighthSID;
	if (addressOfSecondSID >= 0xDE00 || addressOfThirdSID >= 0xDE00 || addressOfFourthSID >= 0xDE00 || addressOfFifthSID >= 0xDE00 || addressOfSixthSID >= 0xDE00 || addressOfSeventhSID >= 0xDE00 || addressOfEighthSID >= 0xDE00)
	{
		this->SidAtDE00toDF00 = true;
	}
	else
	{
		this->SidAtDE00toDF00 = false;
	}

	for (int i = 0; i < _countof(sids); i++)
	{
		// Make the last odd sid output to both channels.
		sids[i]->forceMono = (i == numberOfExtraSidChips);
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

	if (soundwrite_pos > bufferByteLockPoint) 
	{
		gap = soundBufferByteSize - soundwrite_pos + bufferByteLockPoint;		
	}
	else
	{
		gap = bufferByteLockPoint - soundwrite_pos;
	}

	if (gap < (DSGAP1 * bufferLockByteSize/8))
	{
		/*audio is getting ahead of us*/
		bufferByteLockPoint =  (soundwrite_pos + 3 * bufferLockByteSize) % soundBufferByteSize;
		//return E_FAIL;
		//bufferByteLockPoint =  (soundplay_pos + 4 * bufferLockByteSize) % soundBufferByteSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 0;
	}
	else if (gap <= (DSGAP2 * bufferLockByteSize/8))
	{
		//AUDIO_QUICK means to apply a correction using the audio clock sync function to speed up the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_QUICK;
		currentAudioSyncState = 1;
	}
	else if (gap <= (DSGAP3 * bufferLockByteSize/8))
	{
		/*ideal condition where sound pointer is in the comfort zone*/
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 2;
	}
	else if (gap <= (DSGAP4 * bufferLockByteSize/8))
	{
		//AUDIO_SLOW means to apply a correction using the audio clock sync function to slow down the emulation to catch up with the audio.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_SLOW;
		currentAudioSyncState = 3;
	}
	else
	{
		/*audio is getting behind of us*/
		bufferByteLockPoint =  (soundwrite_pos + 4 * bufferLockByteSize) % soundBufferByteSize;
		//This is OK because we are makeing a large correction to the buffer read point.
		appStatus->m_audioSpeedStatus = HCFG::AUDIO_OK;
		currentAudioSyncState = 4;
	}

#if defined(DEBUG) && DEBUG_AUDIO_CLOCK_SYNC != 0
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

	hRet = dx->pSecondarySoundBuffer->Lock(bufferByteLockPoint, bufferLockByteSize ,(LPVOID *)&pBuffer1 ,&bufferByteLen1 ,(LPVOID *)&pBuffer2 ,&bufferByteLen2 ,0);
	if (FAILED(hRet))
	{
		pBuffer1=NULL;
	}
	else
	{
		bufferSampleBlockLen1 = bufferByteLen1 / (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
		bufferSampleBlockLen2 = bufferByteLen2 / (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
		bufferSampleBlockIndex=0;
		bufferSplit=0;
		DWORD bytesWritten = 0;
		DWORD bytesToWrite;
		if (overflowBufferBlockIndex > 0)
		{
			if (overflowBufferBlockIndex >= bufferSampleBlockLen1)
			{
				memcpy_s(pBuffer1, bufferByteLen1, pOverflowBuffer, bufferByteLen1);
				bytesWritten += bufferByteLen1;
				if (overflowBufferBlockIndex >= (bufferSampleBlockLen1 + bufferSampleBlockLen2))
				{
					memcpy_s(pBuffer2, bufferByteLen2, &pOverflowBuffer[bufferSampleBlockLen1], bufferByteLen2);
					bufferSplit = 2;
					bytesWritten += bufferByteLen2;
				}
				else
				{
					bytesToWrite = (overflowBufferBlockIndex - bufferSampleBlockLen1) * (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
					memcpy_s(pBuffer2, bufferByteLen2, &pOverflowBuffer[bufferSampleBlockLen1], bytesToWrite);
					bufferSplit = 1;
					bytesWritten += bytesToWrite;
				}

				bufferSampleBlockIndex = bytesWritten / (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
			}
			else
			{
				bytesToWrite = overflowBufferBlockIndex * (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
				memcpy_s(pBuffer1, bufferByteLen1, pOverflowBuffer, bytesToWrite);
				bufferSampleBlockIndex = overflowBufferBlockIndex;
				bytesWritten += bytesToWrite;
			}
		}

		overflowBufferBlockIndex = 0;
		bufferByteLockPoint = (bufferByteLockPoint + bytesWritten) % (soundBufferByteSize);
	}
	return hRet;
}

void SID64::UnLockSoundBuffer()
{
	if (pBuffer1)
	{
		dx->pSecondarySoundBuffer->Unlock(pBuffer1, bufferByteLen1, pBuffer2, bufferByteLen2);
		pBuffer1 = NULL;
	}
}


HRESULT SID64::Init(CAppStatus *appStatus, CDX9 *dx, HCFG::EMUFPS fps, ICia1 *cia1)
{
HRESULT hr;
	this->appStatus = appStatus;
	this->dx = dx;
	appStatus->m_bFilterOK = false;
	CleanUp();
	if (trig.init(131072L) != 0)
	{
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
	}

	if (appStatus->m_bSoundOK)
	{		
		bufferLockByteSize = GetSoundBufferLockSize(fps);
		soundBufferByteSize = dx->SoundBufferByteSize;

		//Allocate largest buffer to accommodate both EMUFPS_50 and EMUFPS_50_12
		overflowBufferSampleLen =  GetSoundBufferLockSize(HCFG::EMUFPS_50) / (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
	}
	else
	{
		bufferLockByteSize = 0;
		soundBufferByteSize = 0;
		overflowBufferSampleLen = 0;
	}

	if (overflowBufferSampleLen != 0)
	{
		DWORD numberOfBytes = overflowBufferSampleLen * (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS);
		pOverflowBuffer = (bit32 *)VirtualAlloc(NULL, numberOfBytes, MEM_COMMIT, PAGE_READWRITE);
		if (!pOverflowBuffer)
		{
			return E_OUTOFMEMORY;
		}

		ZeroMemory(pOverflowBuffer, numberOfBytes);
	}

	hr = this->sid1.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 1 init failed."));
	}

	hr = this->sid2.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 2 init failed."));
	}

	hr = this->sid3.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 3 init failed."));
	}

	hr = this->sid4.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 4 init failed."));
	}

	hr = this->sid5.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 5 init failed."));
	}

	hr = this->sid6.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 6 init failed."));
	}

	hr = this->sid7.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 7 init failed."));
	}

	hr = this->sid8.Init(appStatus, cia1);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("SID 8 init failed."));
	}

	sidSampler=-1;
	bufferByteLockPoint = 0;
	hr = InitResamplingFilters(fps);
	if (FAILED(hr))
	{
		return SetError(hr, TEXT("InitResamplingFilters Failed."));
	}

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
	bufferLockByteSize = GetSoundBufferLockSize(fps);
	return bufferLockByteSize;
}


HRESULT SID64::InitResamplingFilters(HCFG::EMUFPS fps)
{
const int INTERPOLATOR_CUTOFF_FREQUENCY = 16000;
const int INTERPOLATOR2X_CUTOFF_FREQUENCY = 14000;

	appStatus->m_bFilterOK = false;
	long interpolatedSamplesPerSecond;
	filterPreFilterResampleChannel1.CleanSync();
	filterPreFilterResampleChannel2.CleanSync();
	filterPreFilterStage2Channel1.CleanSync();
	filterPreFilterStage2Channel2.CleanSync();

	if (fps == HCFG::EMUFPS_50)
	{
		filterKernelLength = SIDRESAMPLEFIRLENGTH_50;
		filterInterpolationFactor = INTERPOLATION_FACTOR_50;
		filterDecimationFactor = DECIMATION_FACTOR_50;

		interpolatedSamplesPerSecond = PAL50CLOCKSPERSECOND * filterInterpolationFactor;

		if (filterPreFilterResampleChannel1.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
		{
			return E_OUTOFMEMORY;
		}

		if (filterPreFilterResampleChannel2.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
		{
			return E_OUTOFMEMORY;
		}

		filterPreFilterResampleChannel1.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);
		filterPreFilterResampleChannel2.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);
	}
#ifdef ALLOW_EMUFPS_50_12_MULTI
	else if (fps == HCFG::EMUFPS_50_12_MULTI)
	{
		filterInterpolationFactor = INTERPOLATION_FACTOR_50_12;
		filterDecimationFactor = DECIMATION_FACTOR_50_12;

		const int stage1Len = 1528;
		const int stage2Len = 392;
		filterPreFilterResampleChannel1.AllocSync(stage1Len, STAGE1X);
		filterPreFilterResampleChannel2.AllocSync(stage1Len, STAGE1X);
		filterPreFilterStage2Channel1.AllocSync(stage2Len, STAGE2X);
		filterPreFilterStage2Channel2.AllocSync(stage2Len, STAGE2X);

		filterPreFilterResampleChannel1.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X);
		filterPreFilterResampleChannel2.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X);
		filterPreFilterStage2Channel1.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X * STAGE2X);
		filterPreFilterStage2Channel2.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, PALCLOCKSPERSECOND * STAGE1X * STAGE2X);
	}
#endif
	else
	{
		filterKernelLength = SIDRESAMPLEFIRLENGTH_50_12;
		filterInterpolationFactor = INTERPOLATION_FACTOR_50_12;
		filterDecimationFactor = DECIMATION_FACTOR_50_12;
		interpolatedSamplesPerSecond = PALCLOCKSPERSECOND * filterInterpolationFactor;
		if (filterPreFilterResampleChannel1.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
		{
			return E_OUTOFMEMORY;
		}

		if (filterPreFilterResampleChannel2.AllocSync(filterKernelLength, filterInterpolationFactor) !=0)
		{
			return E_OUTOFMEMORY;
		}

		filterPreFilterResampleChannel1.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);
		filterPreFilterResampleChannel2.CreateFIRKernel(INTERPOLATOR_CUTOFF_FREQUENCY, interpolatedSamplesPerSecond);
	}

	if (sidSampler >= filterInterpolationFactor)
	{
		sidSampler = filterInterpolationFactor - 1;
	}
	else if (sidSampler < filterDecimationFactor)
	{
		sidSampler = -filterDecimationFactor;
	}

	sidSampler=-1;
	appStatus->m_bFilterOK = true;
	return S_OK;
}

void SID64::InitReset(ICLK sysclock, bool poweronreset)
{
	CurrentClock = sysclock;
	sid1.InitReset(sysclock, poweronreset);
	sid2.InitReset(sysclock, poweronreset);
	sid3.InitReset(sysclock, poweronreset);
	sid4.InitReset(sysclock, poweronreset);
	sid5.InitReset(sysclock, poweronreset);
	sid6.InitReset(sysclock, poweronreset);
	sid7.InitReset(sysclock, poweronreset);
	sid8.InitReset(sysclock, poweronreset);
}

void SID64::Reset(ICLK sysclock, bool poweronreset)
{
	InitReset(sysclock, poweronreset);
	sid1.Reset(sysclock, poweronreset);
	sid2.Reset(sysclock, poweronreset);
	sid3.Reset(sysclock, poweronreset);
	sid4.Reset(sysclock, poweronreset);
	sid5.Reset(sysclock, poweronreset);
	sid6.Reset(sysclock, poweronreset);
	sid7.Reset(sysclock, poweronreset);
	sid8.Reset(sysclock, poweronreset);
	appStatus->ResetSidChipAddressMap();
}

void SID64::ExecuteCycle(ICLK sysclock)
{
	if (appStatus->m_bSIDResampleMode)
	{
		if (appStatus->m_bSIDStereo)
		{
			ClockSidResampleStereo(sysclock, this->NumberOfExtraSidChips);
		}
		else
		{
			ClockSidResampleMono(sysclock, this->NumberOfExtraSidChips);
		}
	}
	else
	{
		if (appStatus->m_bSIDStereo)
		{
			ClockSidDownSampleStereo(sysclock, this->NumberOfExtraSidChips);
		}
		else
		{
			ClockSidDownSampleMono(sysclock, this->NumberOfExtraSidChips);
		}
	}
}

void SID64::ClockSidResampleStereo(ICLK sysclock, int numberOfExtraSidChips)
{
ICLKS clocks;
double fsampleChannel1;
double fsampleChannel2;
long sampleOffset;

	clocks = (ICLKS)(sysclock - CurrentClock);

	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;

		// Update SID1 chip state.
		sid1.ExecuteCycle(CurrentClock);
		if (numberOfExtraSidChips > 0)
		{
			// Update SID2 chip state.
			if (sid2.active)
			{
				sid2.ExecuteCycle(CurrentClock);
			}

			if (sid3.active)
			{
				sid3.ExecuteCycle(CurrentClock);
			}

			if (sid4.active)
			{
				sid4.ExecuteCycle(CurrentClock);
			}

			if (sid5.active)
			{
				sid5.ExecuteCycle(CurrentClock);
			}

			if (sid6.active)
			{
				sid6.ExecuteCycle(CurrentClock);
			}

			if (sid7.active)
			{
				sid7.ExecuteCycle(CurrentClock);
			}

			if (sid8.active)
			{
				sid8.ExecuteCycle(CurrentClock);
			}
		}

		if (appStatus->m_bMaxSpeed)
		{
			// Host PC sound processing is not needed when sound is turned off during "Max Speed".
			continue;
		}

		int channel1SidCount = 1;
		int channel2SidCount = 0;
		fsampleChannel1 = sid1.GetResample();
		if (numberOfExtraSidChips == 0)
		{
			fsampleChannel2 = fsampleChannel1;
		}
		else
		{
			fsampleChannel2 = 0.0;
			if (sid2.active)
			{
				fsampleChannel2 += sid2.GetResample();
				channel2SidCount++;
			}

			if (sid3.active)
			{
				if (sid3.forceMono)
				{
					double fsid3 = sid3.GetResample();
					fsampleChannel1 += fsid3;
					fsampleChannel2 += fsid3;
					channel1SidCount++;
					channel2SidCount++;
				}
				else
				{
					fsampleChannel1 += sid3.GetResample();
					channel1SidCount++;
				}
			}

			if (sid4.active)
			{
				fsampleChannel2 += sid4.GetResample();
				channel2SidCount++;
			}


			if (sid5.active)
			{
				if (sid5.forceMono)
				{
					double fsid5 = sid5.GetResample();
					fsampleChannel1 += fsid5;
					fsampleChannel2 += fsid5;
					channel1SidCount++;
					channel2SidCount++;
				}
				else
				{
					fsampleChannel1 += sid5.GetResample();
					channel1SidCount++;
				}
			}

			if (sid6.active)
			{
				fsampleChannel2 += sid6.GetResample();
				channel2SidCount++;
			}

			if (sid7.active)
			{
				if (sid7.forceMono)
				{
					double fsid7 = sid7.GetResample();
					fsampleChannel1 += fsid7;
					fsampleChannel2 += fsid7;
					channel1SidCount++;
					channel2SidCount++;
				}
				else
				{
					fsampleChannel1 += sid7.GetResample();
					channel1SidCount++;
				}
			}

			if (sid8.active)
			{
				fsampleChannel2 += sid8.GetResample();
				channel2SidCount++;
			}

			if (channel2SidCount == 0)
			{
				channel2SidCount = 1;
			}

			fsampleChannel1 = fsampleChannel1 / (double)(channel1SidCount);
			fsampleChannel2 = fsampleChannel2 / (double)(channel2SidCount);
		}

		sidSampler = sidSampler + filterInterpolationFactor;
		filterPreFilterResampleChannel1.QueueNextSample(fsampleChannel1);
		filterPreFilterResampleChannel2.QueueNextSample(fsampleChannel2);
		if (sidSampler >= 0)
		{
			sampleOffset = (filterInterpolationFactor-1)-sidSampler;
			sidSampler = sidSampler - filterDecimationFactor;

#ifdef ALLOW_EMUFPS_50_12_MULTI
			if (appStatus->m_fps == HCFG::EMUFPS_50_12_MULTI)
			{
				int offset1;
				offset1 = sampleOffset % STAGE1X;
				filterPreFilterResampleChannel1.FIR_ProcessSampleNx_IndexTo8(offset1, filterPreFilterStage2Channel1.buf);
				filterPreFilterResampleChannel2.FIR_ProcessSampleNx_IndexTo8(offset1, filterPreFilterStage2Channel2.buf);

				offset1 = sampleOffset % STAGE2X;
				fsampleChannel1 = filterPreFilterStage2Channel1.InterpolateQueuedSamples(offset1);
				fsampleChannel2 = filterPreFilterStage2Channel2.InterpolateQueuedSamples(offset1);
			}
			else
			{
#endif
				fsampleChannel1 = filterPreFilterResampleChannel1.InterpolateQueuedSamples(sampleOffset);
				fsampleChannel2 = filterPreFilterResampleChannel2.InterpolateQueuedSamples(sampleOffset);
#ifdef ALLOW_EMUFPS_50_12_MULTI
			}
#endif
			if (pBuffer1==NULL)
			{
				return;				
			}

			fsampleChannel1 = fsampleChannel1 * (double)filterInterpolationFactor * MasterVolume;
			fsampleChannel2 = fsampleChannel2 * (double)filterInterpolationFactor * MasterVolume;
			if (fsampleChannel1 > Max16BitSample)
			{
				fsampleChannel1 = Max16BitSample;
			}
			else if (fsampleChannel1 < Min16BitSample)
			{
				fsampleChannel1 = Min16BitSample;
			}

			if (fsampleChannel2 > Max16BitSample)
			{
				fsampleChannel2 = Max16BitSample;
			}
			else if (fsampleChannel2 < Min16BitSample)
			{
				fsampleChannel2 = Min16BitSample;
			}

			if (pBuffer1!=0)
			{
				WriteSample((bit16)fsampleChannel1, (bit16)fsampleChannel2);
			}
		}
	}
}

void SID64::ClockSidResampleMono(ICLK sysclock, int numberOfExtraSidChips)
{
ICLKS clocks;
double fsampleMono;
long sampleOffset;

	clocks = (ICLKS)(sysclock - CurrentClock);

	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;

		// Update SID1 chip state.
		sid1.ExecuteCycle(CurrentClock);
		if (numberOfExtraSidChips > 0)
		{
			// Update SID2 chip state.
			if (sid2.active)
			{
				sid2.ExecuteCycle(CurrentClock);
			}

			if (sid3.active)
			{
				sid3.ExecuteCycle(CurrentClock);
			}

			if (sid4.active)
			{
				sid4.ExecuteCycle(CurrentClock);
			}

			if (sid5.active)
			{
				sid5.ExecuteCycle(CurrentClock);
			}

			if (sid6.active)
			{
				sid6.ExecuteCycle(CurrentClock);
			}

			if (sid7.active)
			{
				sid7.ExecuteCycle(CurrentClock);
			}

			if (sid8.active)
			{
				sid8.ExecuteCycle(CurrentClock);
			}
		}

		if (appStatus->m_bMaxSpeed)
		{
			// Host PC sound processing is not needed when sound is turned off during "Max Speed".
			continue;
		}

		int channelSidCount = 1;
		fsampleMono = sid1.GetResample();
		if (numberOfExtraSidChips > 0)
		{
			if (sid2.active)
			{
				fsampleMono += sid2.GetResample();
				channelSidCount++;
			}

			if (sid3.active)
			{
				fsampleMono += sid3.GetResample();
				channelSidCount++;
			}

			if (sid4.active)
			{
				fsampleMono += sid4.GetResample();
				channelSidCount++;
			}

			if (sid5.active)
			{
				fsampleMono += sid5.GetResample();
				channelSidCount++;
			}

			if (sid6.active)
			{
				fsampleMono += sid6.GetResample();
				channelSidCount++;
			}

			if (sid7.active)
			{
				fsampleMono += sid7.GetResample();
				channelSidCount++;
			}

			if (sid8.active)
			{
				fsampleMono += sid8.GetResample();
				channelSidCount++;
			}
		}

		fsampleMono = fsampleMono / (double)(channelSidCount);

		// Resample filter
		sidSampler = sidSampler + filterInterpolationFactor;
		filterPreFilterResampleChannel1.QueueNextSample(fsampleMono);
		if (sidSampler >= 0)
		{
			sampleOffset = (filterInterpolationFactor-1)-sidSampler;
			sidSampler = sidSampler - filterDecimationFactor;

#ifdef ALLOW_EMUFPS_50_12_MULTI
			if (appStatus->m_fps == HCFG::EMUFPS_50_12_MULTI)
			{
				int offset1;
				offset1 = sampleOffset % STAGE1X;
				filterPreFilterResampleChannel1.FIR_ProcessSampleNx_IndexTo8(offset1, filterPreFilterStage2Channel1.buf);

				offset1 = sampleOffset % STAGE2X;
				fsampleMono = filterPreFilterStage2Channel1.InterpolateQueuedSamples(offset1);
			}
			else
			{
#endif
				fsampleMono = filterPreFilterResampleChannel1.InterpolateQueuedSamples(sampleOffset);
#ifdef ALLOW_EMUFPS_50_12_MULTI
			}
#endif
			if (pBuffer1==NULL)
			{
				return;				
			}

			fsampleMono = fsampleMono * (double)filterInterpolationFactor * MasterVolume;
			if (fsampleMono > Max16BitSample)
			{
				fsampleMono = Max16BitSample;
			}
			else if (fsampleMono < Min16BitSample)
			{
				fsampleMono = Min16BitSample;
			}

			if (pBuffer1!=0)
			{
				WriteSample((bit16)fsampleMono, (bit16)fsampleMono);
			}
		}
	}
}

void SID64::ClockSidDownSampleStereo(ICLK sysclock, int numberOfExtraSidChips)
{
ICLKS clocks;
double fsampleChannel1;
double fsampleChannel2;

	clocks = (ICLKS)(sysclock - CurrentClock);

	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;

		sid1.Envelope();
		if (numberOfExtraSidChips > 0)
		{
			if (sid2.active)
			{
				sid2.Envelope();
			}

			if (sid3.active)
			{
				sid3.Envelope();
			}

			if (sid4.active)
			{
				sid4.Envelope();
			}

			if (sid5.active)
			{
				sid5.Envelope();
			}

			if (sid6.active)
			{
				sid6.Envelope();
			}

			if (sid7.active)
			{
				sid7.Envelope();
			}

			if (sid8.active)
			{
				sid8.Envelope();
			}
		}

		if (appStatus->m_bMaxSpeed)
		{
			// Host PC sound processing is not needed when sound is turned off during "Max Speed".
			continue;
		}

		sidSampler = sidSampler + filterInterpolationFactor;
		if (sidSampler >= 0)
		{
			sidSampler = sidSampler - filterDecimationFactor;

			sid1.Modulate(CurrentClock);
			if (numberOfExtraSidChips > 0)
			{
				if (sid2.active)
				{
					sid2.Modulate(CurrentClock);
				}

				if (sid3.active)
				{
					sid3.Modulate(CurrentClock);
				}

				if (sid4.active)
				{
					sid4.Modulate(CurrentClock);
				}

				if (sid5.active)
				{
					sid5.Modulate(CurrentClock);
				}

				if (sid6.active)
				{
					sid6.Modulate(CurrentClock);
				}

				if (sid7.active)
				{
					sid7.Modulate(CurrentClock);
				}

				if (sid8.active)
				{
					sid8.Modulate(CurrentClock);
				}
			}

			if (pBuffer1==NULL)
			{
				return;
			}

			int channel1SidCount = 1;
			int channel2SidCount = 0;
			fsampleChannel1 = sid1.GetDownsample();
			if (numberOfExtraSidChips == 0)
			{
				fsampleChannel2 = fsampleChannel1;
			}
			else
			{
				fsampleChannel2 = 0.0;
				if (sid2.active)
				{
					fsampleChannel2 += sid2.GetDownsample();
					channel2SidCount++;
				}

				if (sid3.active)
				{
					if (sid3.forceMono)
					{
						double fsid3 = sid3.GetDownsample();
						fsampleChannel1 += fsid3;
						fsampleChannel2 += fsid3;
						channel1SidCount++;
						channel2SidCount++;
					}
					else
					{
						fsampleChannel1 += sid3.GetDownsample();
						channel1SidCount++;
					}
				}

				if (sid4.active)
				{
					fsampleChannel2 += sid4.GetDownsample();
					channel2SidCount++;
				}

				if (sid5.active)
				{
					if (sid5.forceMono)
					{
						double fsid5 = sid5.GetDownsample();
						fsampleChannel1 += fsid5;
						fsampleChannel2 += fsid5;
						channel1SidCount++;
						channel2SidCount++;
					}
					else
					{
						fsampleChannel1 += sid5.GetDownsample();
						channel1SidCount++;
					}
				}

				if (sid6.active)
				{
					fsampleChannel2 += sid6.GetDownsample();
					channel2SidCount++;
				}

				if (sid7.active)
				{
					if (sid7.forceMono)
					{
						double fsid7 = sid7.GetDownsample();
						fsampleChannel1 += fsid7;
						fsampleChannel2 += fsid7;
						channel1SidCount++;
						channel2SidCount++;
					}
					else
					{
						fsampleChannel1 += sid7.GetDownsample();
						channel1SidCount++;
					}
				}

				if (sid8.active)
				{
					fsampleChannel2 += sid8.GetResample();
					channel2SidCount++;
				}

				if (channel2SidCount == 0)
				{
					channel2SidCount = 1;
				}

				fsampleChannel1 = fsampleChannel1 / (double)(channel1SidCount);
				fsampleChannel2 = fsampleChannel2 / (double)(channel2SidCount);
			}

			if (fsampleChannel1 > Max16BitSample)
			{
				fsampleChannel1 = Max16BitSample;
			}
			else if (fsampleChannel1 < Min16BitSample)
			{
				fsampleChannel1 = Min16BitSample;
			}

			if (fsampleChannel2 > Max16BitSample)
			{
				fsampleChannel2 = Max16BitSample;
			}
			else if (fsampleChannel2 < Min16BitSample)
			{
				fsampleChannel2 = Min16BitSample;
			}

			if (pBuffer1!=0)
			{
				WriteSample((bit16)fsampleChannel1, (bit16)fsampleChannel2);
			}
		}
	}
}

void SID64::ClockSidDownSampleMono(ICLK sysclock, int numberOfExtraSidChips)
{
ICLKS clocks;
double fsampleSid;
double fsample;
short dxsample;

	clocks = (ICLKS)(sysclock - CurrentClock);

	for( ; clocks > 0 ; clocks--)
	{
		CurrentClock++;

		sid1.Envelope();
		if (numberOfExtraSidChips > 0)
		{
			if (sid2.active)
			{
				sid2.Envelope();
			}

			if (sid3.active)
			{
				sid3.Envelope();
			}

			if (sid4.active)
			{
				sid4.Envelope();
			}

			if (sid5.active)
			{
				sid5.Envelope();
			}

			if (sid6.active)
			{
				sid6.Envelope();
			}

			if (sid7.active)
			{
				sid7.Envelope();
			}

			if (sid8.active)
			{
				sid8.Envelope();
			}
		}

		if (appStatus->m_bMaxSpeed)
		{
			// Host PC sound processing is not needed when sound is turned off during "Max Speed".
			continue;
		}

		sidSampler = sidSampler + filterInterpolationFactor;
		if (sidSampler >= 0)
		{
			sidSampler = sidSampler - filterDecimationFactor;

			sid1.Modulate(CurrentClock);
			if (numberOfExtraSidChips > 0)
			{
				if (sid2.active)
				{
					sid2.Modulate(CurrentClock);
				}

				if (sid3.active)
				{
					sid3.Modulate(CurrentClock);
				}

				if (sid4.active)
				{
					sid4.Modulate(CurrentClock);
				}

				if (sid5.active)
				{
					sid5.Modulate(CurrentClock);
				}

				if (sid6.active)
				{
					sid6.Modulate(CurrentClock);
				}

				if (sid7.active)
				{
					sid7.Modulate(CurrentClock);
				}

				if (sid8.active)
				{
					sid8.Modulate(CurrentClock);
				}
			}

			if (pBuffer1==NULL)
			{
				return;
			}

			fsample = sid1.GetDownsample();
			if (numberOfExtraSidChips > 0)
			{
				fsampleSid = fsample;
				if (sid2.active)
				{
					fsampleSid += sid2.GetDownsample();
				}

				if (sid3.active)
				{
					fsampleSid += sid3.GetDownsample();
				}

				if (sid4.active)
				{
					fsampleSid += sid4.GetDownsample();
				}

				if (sid5.active)
				{
					fsampleSid += sid5.GetDownsample();
				}

				if (sid6.active)
				{
					fsampleSid += sid6.GetDownsample();
				}

				if (sid7.active)
				{
					fsampleSid += sid7.GetDownsample();
				}

				if (sid8.active)
				{
					fsampleSid += sid8.GetDownsample();
				}

				fsample = fsampleSid / (double)(numberOfExtraSidChips + 1);
			}

			fsample = fsample * MasterVolume;
			if (fsample > Max16BitSample)
			{
				fsample = Max16BitSample;
			}
			else if (fsample < Min16BitSample)
			{
				fsample = Min16BitSample;
			}

			dxsample = (WORD)(fsample);
			if (pBuffer1!=0)
			{
				WriteSample((bit16)dxsample, (bit16)dxsample);
			}
		}
	}
}

void SID64::WriteSample(bit16 dxsampleLeft, bit16 dxsampleRight)
{
	bit32 dxsample = ((bit32)dxsampleRight << 16) | (bit32)dxsampleLeft;
	m_last_dxsample = dxsample;
	if (bufferSplit == 0)
	{
		pBuffer1[bufferSampleBlockIndex] = dxsample;
		bufferSampleBlockIndex++;
		if (bufferSampleBlockIndex >= bufferSampleBlockLen1)
		{
			if (pBuffer2)
			{
				bufferSplit = 1;
			}
			else
			{
				bufferSplit = 2;
			}

			bufferSampleBlockIndex = 0;
		}

		bufferByteLockPoint = (bufferByteLockPoint + (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS)) % (soundBufferByteSize);
	}
	else if (bufferSplit == 1)
	{
		pBuffer2[bufferSampleBlockIndex] = dxsample;
		bufferSampleBlockIndex++;
		if (bufferSampleBlockIndex >= bufferSampleBlockLen2)
		{
			bufferSplit = 2;
			bufferSampleBlockIndex = 0;
		}

		bufferByteLockPoint = (bufferByteLockPoint + (SID_SOUND_BYTES_PER_SAMPLE * SID_SOUND_NUMBER_OF_CHANNELS)) % (soundBufferByteSize);
	}
	else if (bufferSplit == 2)
	{
		if (overflowBufferBlockIndex < overflowBufferSampleLen)
		{
			pOverflowBuffer[overflowBufferBlockIndex] = dxsample;
			overflowBufferBlockIndex++;
		}
	}
}

void SID64::SoundHalt(bit32 value)
{
	if (pOverflowBuffer)
	{
		for (DWORD i = 0 ; i < overflowBufferSampleLen ; i++)
		{
			((bit32 *)pOverflowBuffer)[i] = value;
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
	sid1.SetCurrentClock(sysclock);
	sid2.SetCurrentClock(sysclock);
	sid3.SetCurrentClock(sysclock);
	sid4.SetCurrentClock(sysclock);
	sid5.SetCurrentClock(sysclock);
	sid6.SetCurrentClock(sysclock);
	sid7.SetCurrentClock(sysclock);
	sid8.SetCurrentClock(sysclock);
}

void SID64::PreventClockOverflow()
{
	sid1.PreventClockOverflow(CurrentClock);
	sid2.PreventClockOverflow(CurrentClock);
	sid3.PreventClockOverflow(CurrentClock);
	sid4.PreventClockOverflow(CurrentClock);
	sid5.PreventClockOverflow(CurrentClock);
	sid6.PreventClockOverflow(CurrentClock);
	sid7.PreventClockOverflow(CurrentClock);
	sid8.PreventClockOverflow(CurrentClock);
}

bit8 SID64::ReadRegister(bit16 address, ICLK sysclock)
{
	if (appStatus->m_bSID_Emulation_Enable)
	{
		ExecuteCycle(sysclock);
	}

	if (this->NumberOfExtraSidChips == 0)
	{
		return sid1.ReadRegister(address, sysclock);
	}
	else
	{
		bit16 maskaddress = address & 0xDFE0;
		if (maskaddress == 0xD400)
		{
			return sid1.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSecondSID && sid2.active)
		{
			return sid2.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfThirdSID && sid3.active)
		{
			return sid3.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfFourthSID && sid4.active)
		{
			return sid4.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfFifthSID && sid5.active)
		{
			return sid5.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSixthSID && sid6.active)
		{
			return sid6.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSeventhSID && sid7.active)
		{
			return sid7.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfEighthSID && sid8.active)
		{
			return sid8.ReadRegister(address, sysclock);
		}
		else
		{
			return 0;
		}
	}
}

bit8 SID64::ReadRegister_no_affect(bit16 address, ICLK sysclock)
{
	if (appStatus->m_bSID_Emulation_Enable)
	{
		ExecuteCycle(sysclock);
	}

	if (this->NumberOfExtraSidChips == 0)
	{
		return sid1.ReadRegister_no_affect(address, sysclock);
	}
	else
	{
		bit16 maskaddress = address & 0xDFE0;
		if (maskaddress == 0xD400)
		{
			return sid1.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSecondSID && sid2.active)
		{
			return sid2.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfThirdSID && sid3.active)
		{
			return sid3.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfFourthSID && sid4.active)
		{
			return sid4.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfFifthSID && sid5.active)
		{
			return sid5.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSixthSID && sid6.active)
		{
			return sid6.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfSeventhSID && sid7.active)
		{
			return sid7.ReadRegister(address, sysclock);
		}
		else if (maskaddress == this->AddressOfEighthSID && sid8.active)
		{
			return sid8.ReadRegister(address, sysclock);
		}
		else
		{
			return 0;
		}
	}
}

void SID64::WriteRegister(bit16 address, ICLK sysclock, bit8 data)
{
	if (appStatus->m_bSID_Emulation_Enable)
	{
		ExecuteCycle(sysclock);
	}

	if (this->NumberOfExtraSidChips == 0)
	{
		sid1.WriteRegister(address, sysclock, data);
	}
	else
	{
		bit16 maskaddress = address & 0xDFE0;
		if (maskaddress == 0xD400)
		{
			sid1.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfSecondSID && sid2.active)
		{
			sid2.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfThirdSID && sid3.active)
		{
			sid3.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfFourthSID && sid4.active)
		{
			sid4.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfFifthSID && sid5.active)
		{
			sid5.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfSixthSID && sid6.active)
		{
			sid6.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfSeventhSID && sid7.active)
		{
			sid7.WriteRegister(address, sysclock, data);
		}
		else if (maskaddress == this->AddressOfEighthSID && sid8.active)
		{
			sid8.WriteRegister(address, sysclock, data);
		}
	}
}
