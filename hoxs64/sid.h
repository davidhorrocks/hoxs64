#pragma once

#include "sidvoice.h"
#include "sidchip.h"

class ICia1;
class SID64;

class SID64 : public ISid64, public ErrorMsg
{
public:
	SID64();
	~SID64();
	CDX9 *dx;
	CAppStatus *appStatus;
	bit32 m_last_dxsample;
	double MasterVolume;
	SidChip sid1;
	SidChip sid2;
	SidChip sid3;
	SidChip sid4;
	SidChip sid5;
	SidChip sid6;
	SidChip sid7;
	SidChip sid8;

	HRESULT Init(CAppStatus *, CDX9 *, HCFG::EMUFPS fps, ICia1 *cia1);
	HRESULT InitResamplingFilters(HCFG::EMUFPS fps);
	void SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID);
	DWORD UpdateSoundBufferLockSize(HCFG::EMUFPS fps);
	HRESULT LockSoundBuffer();
	void UnLockSoundBuffer();
	void SoundHalt(bit32 value);
	void InitReset(ICLK sysclock, bool poweronreset);	

	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	void ClockSidResampleStereo(ICLK sysclock, int numberOfExtraSidChips);
	void ClockSidResampleMono(ICLK sysclock, int numberOfExtraSidChips);
	void ClockSidDownSampleStereo(ICLK sysclock, int numberOfExtraSidChips);
	void ClockSidDownSampleMono(ICLK sysclock, int numberOfExtraSidChips);
	void WriteSample(bit16 dxsampleLeft, bit16 dxsampleRight);
	void PreventClockOverflow();

	friend struct SIDVoice;
private:
	long filterInterpolationFactor;
	long filterDecimationFactor;
	long filterKernelLength;
	Filter filterPreFilterStage2Channel1;
	Filter filterPreFilterStage2Channel2;
	Filter filterPreFilterResampleChannel1;
	Filter filterPreFilterResampleChannel2;
	DWORD soundBufferByteSize;
	DWORD bufferLockByteSize;
	DWORD bufferByteLockPoint;
	bit8 bufferSplit;
	bit32 *pBuffer1;
	bit32 *pBuffer2;
	bit32 *pOverflowBuffer;
	DWORD bufferByteLen1;
	DWORD bufferByteLen2;
	DWORD bufferSampleBlockLen1;
	DWORD bufferSampleBlockLen2;
	DWORD overflowBufferSampleLen;
	DWORD bufferSampleBlockIndex;
	DWORD overflowBufferBlockIndex;
	long sidSampler;//Used for filter
	int currentAudioSyncState;
	int lastAudioSyncState;
	int lastAudioGap;

	DWORD GetSoundBufferLockSize(HCFG::EMUFPS fps);
	void CleanUp();
};
