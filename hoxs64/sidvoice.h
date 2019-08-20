#pragma once

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

struct SIDVoice
{
	~SIDVoice();
	HRESULT Init(CAppStatus *appStatus);
	void ExecuteCycle(ICLK sysclock);
	void Envelope();
	bit8 GetNextVolume();
	void Modulate();
	void SyncRecheck();
	bit16 ShiftRegisterOutput();
	bit16 CalcWaveOutput(bit8 waveType, bit16 &noiseFeedbackWave, bit16 &mask0, bit16 &mask1);
	void ClockShiftRegister();
	void NoiseWriteback(bit8 control, bit16 sample, bit16 mask0, bit16 mask1);
	void Reset(ICLK sysclock, bool hardreset);
	void SetWave(bit8 new_control);
	void UpdateNextEnvMode();
	void PreventClockOverflow(ICLK sysclock);
	void GetState(SsSidVoiceV3 &state);
	void SetState(ICLK CurrentClock, const SsSidVoiceV3 &state);
	static void UpgradeStateV0ToV1(const SsSidVoice &in, SsSidVoiceV1 &out);
	static void UpgradeStateV1ToV2(const SsSidVoiceV1 &in, SsSidVoiceV2 &out);
	static void UpgradeStateV2ToV3(const SsSidVoiceV2 &in, SsSidVoiceV3 &out);
	CAppStatus *appStatus;
	ISid *pISidChip;
	bit32 counter;
	bit32 frequency;
	bool gotNextVolume;
	bit8 nextvolume;
	bit8 volume;
	bit8 samplevolume;
	ICLK sampleHoldDelayClock;
	bool isSampleHeld;
	eEnvelopeMode envmode;
	eEnvelopeMode next_envmode;
	eEnvelopeMode latched_envmode;
	bool want_latched_envmode;
	bit8 envmode_changing_delay;
	bit8 envelope_count_delay;
	bit8 exponential_count_delay;
	bool reset_envelope_counter;
	bool reset_exponential_counter;
	bit8 next_exponential_counter_period;
	bool envelope_tick;
	eWaveType wavetype;
	bit8 sync;
	bit8 ring_mod;
	bit8 sustain_level;
	bool zeroTheCounter;
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
	bit16 noiseFeedbackSample1;
	bit16 noiseFeedbackSample2;
	bit16 noiseFeedbackMask1;
	bit16 noiseFeedbackMask0;
	double fVolSample;
	bit8 gate;
	bit8 test;
	bit32 sidShiftRegister;
	bit32 sidLatchedShiftRegister;
	bit32 sidShiftRegisterFill;
	int phaseOfShiftRegister;
	bool keep_zero_volume;
	bit16 envelope_counter;
	bit16 envelope_compare;
	bit8 exponential_counter;
	bit8 control;
	bit32s shifterTestCounter;
	bool zeroTheShiftRegister;

	static const bit16 AdsrTable[16];
};
