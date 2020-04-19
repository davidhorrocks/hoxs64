#pragma once
constexpr int FILTERLENGTH = 3;
constexpr int FILTERHISTORYLENGTH = 32;
constexpr double PI = 3.1415926535897932384626433832795;

class TrigTable
{
public:
	TrigTable();
	~TrigTable();
	double sin(double a);
	double cos(double a);
	double sinh(double a);	
	void cleanup();
	long init(long resolution);
private:
	double *sinTable;
	double *cosTable;
	double *sinhTable;	
	double resolution;
};

extern TrigTable trig;

class Filter
{

public:
	Filter();
	~Filter();
	Filter(const Filter&) = delete;
	Filter& operator=(const Filter&) = delete;
	Filter(Filter&&) = delete;
	Filter& operator=(Filter&&) = delete;
	HRESULT Init();

	double InterpolateNextSample2x(double sample, double *sampleout2);
	void FIR_ProcessSampleNx_IndexTo8(unsigned long index, double *sampleout);
	double InterpolateQueuedSamples(int index);
	void QueueNextSample(double sample);
	long AllocSync(unsigned long length, unsigned long interpolation);
	long AllocSyncShared(unsigned long length, unsigned long interpolation, double *sharedCoef);
	void CreateFIRKernel(double frequency,long samplerate);
	void CleanSync();
	void Set_SVF(double frequency, double samplerate, double resonance);
	void SVF_ProcessNextSample(double sample);

	double lp = 0.0;
	double hp = 0.0;
	double bp = 0.0;
	double np = 0.0;
	double peek = 0.0;
	double *buf = nullptr;
private:
	double *coef = nullptr;
	int bufferPos = 0;
	bool isSharedCoefficient = false;	
	unsigned long firLength = 0;
	unsigned long interpolation = 0;
	double svfQ = 0;
	double svfF = 0;
	unsigned long bufferLength=0;
};
