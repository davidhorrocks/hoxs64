#ifndef __SID_H__
#define __SID_H__

typedef enum 
{
	sidENVNONE = 0,
	sidATTACK = 1,
	sidDECAY = 2,
	sidSUSTAIN = 3,
	sidRELEASE = 4
} eEnvelopeMode;


typedef enum 
{
	sidWAVNONE = 0,
	sidTRIANGLE = 1,
	sidSAWTOOTH = 2,
	sidPULSE = 4,
	sidNOISE = 8,
	sidTRIANGLEPULSE = sidPULSE + sidTRIANGLE,
	sidSAWTOOTHPULSE = sidPULSE + sidSAWTOOTH,
	sidTRIANGLESAWTOOTHPULSE = sidPULSE + sidSAWTOOTH + sidTRIANGLE,
	sidTRIANGLESAWTOOTH = sidSAWTOOTH + sidTRIANGLE,
	sidNOISETRIANGLE = sidNOISE + sidTRIANGLE,
	sidNOISESAWTOOTH = sidNOISE + sidSAWTOOTH,
	sidNOISEPULSE = sidNOISE + sidPULSE,
	sidNOISETRIANGLEPULSE = sidNOISE + sidPULSE + sidTRIANGLE,
	sidNOISESAWTOOTHPULSE = sidNOISE + sidPULSE + sidSAWTOOTH,
	sidNOISESTRIANGLESAWTOOTH = sidNOISE + sidSAWTOOTH + sidTRIANGLE,
	sidNOISETRIANGLESAWTOOTHPULSE = sidNOISE + sidPULSE + sidSAWTOOTH + sidTRIANGLE
} eWaveType;

class SID64;
struct SIDVoice
{
	~SIDVoice();
	HRESULT Init(CAppStatus *appStatus);
	void Envelope();
	void Modulate();
	void SyncRecheck();
	bit16 ShiftRegisterOutput();
	bit16 WaveRegister();
	bit16 CalcWaveOutput(bit8 waveType, bit16 &waveNoNoise);
	void ClockShiftRegister();
	void NoiseWriteback(bit8 control);
	void Reset(bool hardreset);
	void SetWave(bit8 new_control);
	void GetState(SsSidVoiceV1 &state);
	void SetState(const SsSidVoiceV1 &state);
	static void UpgradeStateV0ToV1(const SsSidVoice &in, SsSidVoiceV1 &out);
	CAppStatus *appStatus;
	SID64 *sid;
	bit32 counter;
	bit32 frequency;
	bit16 volume;
	bit32 sampleHoldDelay;
	eEnvelopeMode envmode;
	eWaveType wavetype;
	bit8 sync;
	bit8 ring_mod;
	bit8 sustain_level;
	bit8 zeroTheCounter;
	bit8 exponential_counter_period;
	struct SIDVoice *modulator_voice;
	struct SIDVoice *affected_voice;
	bit32 attack_value;
	bit32 decay_value;
	bit32 release_value;
	bit32 pulse_width_counter;
	bit32 pulse_width_reg;
	bit16 sampleHold;
	bit16 lastSample;
	bit16 lastSampleNoNoise;
	double fVolSample;
	bit8 gate;
	bit8 test;
	bit32 sidShiftRegister;
	bit32 sidLatchedShiftRegister;
	int phaseOfShiftRegister;
	bit8 keep_zero_volume;
	bit16 envelope_counter;
	bit16 envelope_compare;
	bit8 exponential_counter;
	bit8 control;
	bit32s shifterTestCounter;
};

class SID64 : public IRegister, public ErrorMsg
{
public:
	SID64();
	~SID64();
	CDX9 *dx;
	CAppStatus *appStatus;
	HRESULT Init(CAppStatus *, CDX9 *, HCFG::EMUFPS fps);
	HRESULT InitResamplingFilters(HCFG::EMUFPS fps);
	DWORD UpdateSoundBufferLockSize(HCFG::EMUFPS fps);


	double MasterVolume;

	HRESULT LockSoundBuffer();
	void UnLockSoundBuffer();
	void SoundHalt(short value);

	void InitReset(ICLK sysclock, bool poweronreset);
	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);
	void ClockSid(BOOL bResample, ICLK sysclock);
	void ClockSidResample(ICLK sysclock);
	void ClockSidDownSample(ICLK sysclock);
	void WriteSample(short dxsample);
	void GetState(SsSidV1 &state);
	void SetState(const SsSidV1 &state);
	static void UpgradeStateV0ToV1(const SsSid &in, SsSidV1 &out);

	friend struct SIDVoice;
	short m_last_dxsample;
private:
	DWORD GetSoundBufferLockSize(HCFG::EMUFPS fps);
	long filterInterpolationFactor;
	long filterDecimationFactor;
	long filterKernelLength;

	static const WORD SID64::sidAttackRate[16];
	static const bit16 SID64::AdsrTable[16];	

	Filter filterPreFilterStage2;

	Filter filterPreFilterResample;
	Filter filterUpSample2xSample;
	Filter svfilterForResample;
	Filter svfilterForDownSample;

	DWORD soundBufferSize;
	DWORD bufferLockSize;
	DWORD bufferLockPoint;
	bit8 bufferSplit;
	LPWORD pBuffer1;
	LPWORD pBuffer2;
	LPWORD pOverflowBuffer;

	DWORD bufferLen1;
	DWORD bufferLen2;
	DWORD bufferSampleLen1;
	DWORD bufferSampleLen2;
	DWORD overflowBufferSampleLen;

	DWORD bufferIndex;
	DWORD overflowBufferIndex;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	BYTE sidBlock_Voice3;
	bit8 sidInternalBusByte;
	ICLK sidReadDelay;
	long sidSampler;//Used for filter

	void SetFilter();
	double GetCutOff(bit16 sidfreq);
	struct SIDVoice voice1, voice2, voice3;
	void CleanUp();
	
};
#endif