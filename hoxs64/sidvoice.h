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
	SIDVoice() = default;
	~SIDVoice() = default;
	SIDVoice(const SIDVoice&) = delete;
	SIDVoice& operator=(const SIDVoice&) = delete;
	SIDVoice(SIDVoice&&) = delete;
	SIDVoice& operator=(SIDVoice&&) = delete;

	HRESULT Init(CAppStatus *appStatus);
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
	CAppStatus *appStatus = nullptr;
	ISid *pISidChip = nullptr;
	bit32 counter = 0;
	bit32 frequency = 0;
	bool gotNextVolume = false;
	bit8 nextvolume = 0;
	bit8 volume = 0;
	bit8 samplevolume = 0;
	ICLK sampleHoldDelayClock = 0;
	bool isSampleHeld = false;
	eEnvelopeMode envmode = eEnvelopeMode::sidENVNONE;
	eEnvelopeMode next_envmode = eEnvelopeMode::sidENVNONE;
	eEnvelopeMode latched_envmode = eEnvelopeMode::sidENVNONE;
	bool want_latched_envmode = false;
	bit8 envmode_changing_delay = 0;
	bit8 envelope_count_delay = 0;
	bit8 exponential_count_delay = 0;
	bool reset_envelope_counter = false;
	bool reset_exponential_counter = false;
	bit8 next_exponential_counter_period = 0;
	bool envelope_tick = false;
	eWaveType wavetype = eWaveType::sidWAVNONE;
	bit8 sync = 0;
	bit8 ring_mod = 0;
	bit8 sustain_level = 0;
	bool zeroTheCounter = true;
	bit8 exponential_counter_period = 0;
	struct SIDVoice *modulator_voice = nullptr;
	struct SIDVoice *affected_voice = nullptr;
	bit32 attack_value = 0;
	bit32 decay_value = 0;
	bit32 release_value = 0;
	bit32 pulse_width_counter = 0;
	bit32 pulse_width_reg = 0;
	bit16 sampleHold = 0;
	bit16 lastSample = 0;
	bit16 noiseFeedbackSample1 = 0;
	bit16 noiseFeedbackSample2 = 0;
	bit16 noiseFeedbackMask1 = 0;
	bit16 noiseFeedbackMask0 = 0;
	double fVolSample = 0.0;
	bit8 gate = 0;
	bit8 test = 0;
	bit32 sidShiftRegister = 0;
	bit32 sidLatchedShiftRegister = 0;
	bit32 sidShiftRegisterFill = 0;
	int phaseOfShiftRegister = 0;
	bool keep_zero_volume = true;
	bit16 envelope_counter = 0;
	bit16 envelope_compare = 0;
	bit8 exponential_counter = 0;
	bit8 control = 0;
	bit32s shifterTestCounter = 0;
	bool zeroTheShiftRegister = false;
	static const bit16 AdsrTable[16];
};
