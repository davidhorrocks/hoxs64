#pragma once

#include "sidvoice.h"

class SidChip : public ISid
{
public:
	SidChip(int id);
	~SidChip();
	HRESULT Init(CAppStatus *appStatus, ICia1 *cia1);
	void InitReset(ICLK sysclock, bool poweronreset);

	//IRegister
	virtual void Reset(ICLK sysclock, bool poweronreset);
	virtual void ExecuteCycle(ICLK sysclock);
	virtual bit8 ReadRegister(bit16 address, ICLK sysclock);
	virtual void WriteRegister(bit16 address, ICLK sysclock, bit8 data);
	virtual bit8 ReadRegister_no_affect(bit16 address, ICLK sysclock);
	virtual ICLK GetCurrentClock();
	virtual void SetCurrentClock(ICLK sysclock);

	//ISid
	virtual bit8 Get_PotX();
	virtual void Set_PotX(ICLK sysclock, bit8 data);
	virtual bit8 Get_PotY();
	virtual void Set_PotY(ICLK sysclock, bit8 data);

	double GetResample();
	double GetDownsample();
	void Modulate(ICLK sysclock);
	void Envelope();
	void PreventClockOverflow(ICLK sysclock);
	void GetState(SsSidV4 &state);
	void SetState(const SsSidV4 &state);

	static void UpgradeStateV0ToV1(const SsSid &in, SsSidV1 &out);
	static void UpgradeStateV1ToV2(const SsSidV1 &in, SsSidV2 &out);
	static void UpgradeStateV2ToV3(const SsSidV2 &in, SsSidV3 &out);
	static void UpgradeStateV3ToV4(const SsSidV3 &in, SsSidV4 &out);

private:
	int id;
	bit16 sidAddress;
	bool active;
	bool forceMono;
	CAppStatus *appStatus;
	bit8 sidVolume;
	bit8 sidFilter;
	bit8 sidVoice_though_filter;
	bit8 sidResonance;
	bit16 sidFilterFrequency;
	bit8 sidPotX;
	bit8 sidPotY;
	bool sidBlock_Voice3;
	bit8 sidInternalBusByte;
	ICLK sidReadDelayClock;
	ICia1 *cia1;
	SIDVoice voice1;
	SIDVoice voice2;
	SIDVoice voice3;
	Filter filterUpSample2xSample;
	Filter svfilterForResample;
	Filter svfilterForDownSample;
	
	void SetFilter();
	double GetCutOff(bit16 sidfreq);
	HRESULT InitSidFilter();
	friend class SID64;
};

