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
	sidPULSE = 3,
	sidNOISE = 4,
	sidTRIANGLEPULSE = 5,
	sidSAWTOOTHPULSE = 6,
	sidTRIANGLESAWTOOTHPULSE = 7,
	sidTRIANGLESAWTOOTH = 9,
	sidNOISETRIANGLE = 10,
	sidNOISESAWTOOTH = 11,
	sidNOISEPULSE = 12,
	sidNOISETRIANGLEPULSE = 13,
	sidNOISESAWTOOTHPULSE = 14,
	sidNOISESTRIANGLESAWTOOTH = 15,
	sidNOISETRIANGLESAWTOOTHPULSE = 16
} eWaveType;

class SID64;
struct SIDVoice
{
	~SIDVoice();
	HRESULT Init(CConfig *cfg, CAppStatus *appStatus);
	void Envelope();
	void Modulate();
	void SyncRecheck();
	unsigned short WaveRegister();
	unsigned short CalcWave(bit8 waveType);
	void ClockShiftRegister();
	void Reset();
	void SetWave(bit8 data);
	void GetState(SsSidVoice &state);
	void SetState(const SsSidVoice &state);

	CConfig *cfg;
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
	unsigned short sampleHold;
	unsigned short lastSample;
	double fVolSample;
	bit8 gate;
	bit8 test;
	bit32 sidShiftRegister;
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
	CConfig *cfg;
	CAppStatus *appStatus;
	HRESULT Init(CConfig *, CAppStatus *, CDX9 *, HCFG::EMUFPS fps);
	HRESULT InitResamplingFilters(HCFG::EMUFPS fps);
	DWORD UpdateSoundBufferLockSize(HCFG::EMUFPS fps);


	double MasterVolume;

	HRESULT LockSoundBuffer();
	void UnLockSoundBuffer();
	void SoundHalt(short value);

	void InitReset(ICLK sysclock);
	//IRegister
	virtual void Reset(ICLK sysclock);
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
	void GetState(SsSid &state);
	void SetState(const SsSid &state);

	friend struct SIDVoice;
	short m_last_dxsample;
private:
	DWORD GetSoundBufferLockSize(HCFG::EMUFPS fps);
	long filterInterpolationFactor;
	long filterDecimationFactor;
	long filterKernelLength;

	static const WORD SID64::sidAttackRate[16];

	Filter filterPreFilterStage2;
	Filter filterNoFilterStage2;

	Filter filterPreFilterResample;
	Filter filterNoFilterResample;
	Filter filterUpSample2xSample;
	Filter svfilter;

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

	DWORD m_soundplay_pos;
	DWORD m_soundwrite_pos;

	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;

	BYTE sidBlock_Voice3;
	bit8 sidLastWrite;
	ICLK sidReadDelay;
	long sidSampler;//Used for filter

	void SetFilter();
	double GetCutOff(bit16 sidfreq);
	struct SIDVoice voice1, voice2, voice3;
	void CleanUp();
	
};
#endif