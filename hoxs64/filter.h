#ifndef __FILTER_H__
#define __FILTER_H__

const int FILTERLENGTH = 3;
const int FILTERHISTORYLENGTH = 32;
const double PI = 3.1415926535897932384626433832795;

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

	double lp,hp,bp,np,peek;
	double *buf;
private:
	double *coef;
	int bufferPos;
	bool isSharedCoefficient;	
	unsigned long firLength;
	unsigned long interpolation;
	double svfQ,svfF;
	unsigned long bufferLength;
};
#endif
