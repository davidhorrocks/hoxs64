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

	CConfig *cfg;
	CAppStatus *appStatus;

	HRESULT Init(CConfig *cfg, CAppStatus *appStatus);

	SID64 *sid;
	DWORD counter;
	DWORD frequency;
	WORD volume;
	DWORD sampleHoldDelay;
	eEnvelopeMode envmode;
	eWaveType wavetype;
	BYTE sync;
	BYTE ring_mod;
	BYTE sustain_level;
	BYTE zeroTheCounter;
	BYTE exponential_counter_period;
	struct SIDVoice *modulator_voice;
	struct SIDVoice *affected_voice;
	DWORD attack_value;
	DWORD decay_value;
	DWORD release_value;
	DWORD pulse_width_counter;
	DWORD pulse_width_reg;
	unsigned short sampleHold;
	unsigned short lastSample;
	double fVolSample;
	//unsigned short wave;
	void Envelope();
	void Modulate();
	void SyncRecheck();
	unsigned short WaveRegister();
	unsigned short CalcWave(bit8 waveType);
	BYTE gate;
	BYTE test;
	bit32 sidShiftRegister;
	BYTE keep_zero_volume;
	bit16 envelope_counter;
	bit16 envelope_compare;
	bit8 exponential_counter;
	bit8 control;
	void ClockShiftRegister();
	void Reset();
	void SetWave(bit8 data);
	long shifterTestCounter;
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

	//IRegister
	virtual void Reset(ICLK sysclock);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);

	void ClockSid(BOOL bResample, ICLK sysclock);
	void ClockSidResample(ICLK sysclock);
	void ClockSidDownSample(ICLK sysclock);
	void WriteSample(short dxsample);

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
	BYTE bufferSplit;
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

	WORD sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	WORD sidFilterFrequency;

	BYTE sidBlock_Voice3;
	bit8 sidLastWrite;
	ICLK sidReadDelay;//Clock
	long sidSampler;

	void SetFilter();
	double GetCutOff(WORD sidfreq);
	struct SIDVoice voice1, voice2, voice3;
	void CleanUp();

};
#endif