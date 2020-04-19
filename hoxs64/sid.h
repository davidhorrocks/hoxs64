#pragma once
#include "errormsg.h"
#include "sidvoice.h"
#include "sidchip.h"

class ICia1;
class SID64;

class SID64 : public ISid64, public ErrorMsg
{
public:
	SID64();
	~SID64();
	SID64(const SID64&) = delete;
	SID64& operator=(const SID64&) = delete;
	SID64(SID64&&) = delete;
	SID64& operator=(SID64&&) = delete;

	CDX9 *dx = nullptr;
	CAppStatus *appStatus = nullptr;
	bit32 m_last_dxsample = 0;
	double MasterVolume = 0.0;
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
	long filterInterpolationFactor = 0;
	long filterDecimationFactor = 0;
	long filterKernelLength = 0;
	Filter filterPreFilterStage2Channel1;
	Filter filterPreFilterStage2Channel2;
	Filter filterPreFilterResampleChannel1;
	Filter filterPreFilterResampleChannel2;
	DWORD soundBufferByteSize = 0;
	DWORD bufferLockByteSize = 0;
	DWORD bufferByteLockPoint = 0;
	bit8 bufferSplit = 0;
	bit32 *pBuffer1 = nullptr;
	bit32 *pBuffer2 = nullptr;
	bit32 *pOverflowBuffer = nullptr;
	DWORD bufferByteLen1 = 0;
	DWORD bufferByteLen2 = 0;
	DWORD bufferSampleBlockLen1 = 0;
	DWORD bufferSampleBlockLen2 = 0;
	DWORD overflowBufferSampleLen = 0;
	DWORD bufferSampleBlockIndex = 0;
	DWORD overflowBufferBlockIndex = 0;
	long sidSampler = 0;//Used for filter
	int currentAudioSyncState = 0;
	int lastAudioSyncState = 0;
	int lastAudioGap = 0;

	DWORD GetSoundBufferLockSize(HCFG::EMUFPS fps);
	void CleanUp();
};
