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

	double ProcessSample(double sample);
	double FIR_ProcessSample(double sample);
	double FIR_ProcessSample2x(double sample, double *sampleout2);
	void FIR_ProcessSample7x(double sample, double *sampleout);


	void FIR_ProcessSampleNx_IndexTo8(unsigned long index, double *sampleout);

	double fir_process_sampleNx_index(int index);
	void fir_buffer_sampleNx(double sample);

	long AllocSync(unsigned long length, unsigned long interpolation);
	long AllocSync(unsigned long length, unsigned long interpolation, double *sharedCoef);
	void CreateFIRKernel(double frequency,long samplerate);
	void CleanSync();

	void Set_SVF(double frequency, double samplerate, double resonance);
	void SVF_ProcessSample(double sample);

	double lp,hp,bp,np,peek;

	BOOL mintransient;

	double *GetCoefficient();

	double *buf;
	unsigned long bufferLength;

private:
	int iirfilterchange;
	double num_set[FILTERLENGTH];
	double den_set[FILTERLENGTH];

	double input_init[FILTERLENGTH];
	double output_init[FILTERLENGTH];

	double num[FILTERLENGTH];
	double den[FILTERLENGTH];
	double output[FILTERLENGTH];
	double input[FILTERLENGTH];

	double history[FILTERHISTORYLENGTH];

	unsigned char findex;

	double *coef;

	int bufferPos;
	bool m_bCoefNeedClean;
	
	unsigned long firLength;

	unsigned long interpolation;

	double svfQ,svfF;
};


#endif
