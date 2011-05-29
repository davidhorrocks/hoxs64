#include <windows.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "filter.h"

TrigTable trig;

Filter::Filter()
{
	Init();
}

Filter::~Filter()
{
	CleanSync();
}

HRESULT Filter::Init()
{
	int i;

	m_bCoefNeedClean = false;
	coef = 0;
	buf = 0;
	firLength = 0L;
	lp = hp = bp = np = peek=0;
	svfF = 0;
	svfQ = 0.01;
	
	for (i=0 ; i < FILTERLENGTH ; i++)
	{
		num[i]=0;
		den[i]=0;
		num_set[i]=0;
		den_set[i]=0;
		input[i]=0;
		output[i]=0;
	}
	ZeroMemory(history, sizeof(double) * FILTERHISTORYLENGTH);
	findex = 0;
	iirfilterchange = 0;
	mintransient = 0;
	return S_OK;
}


double Filter::ProcessSample(double sample)
{
int i,j,k,vindex;
double y;
int m;

	m = (FILTERLENGTH);
	if (iirfilterchange)
	{
		iirfilterchange = 0;
		if (mintransient)
		{
			for (i=0 ; i < FILTERLENGTH ; i++)
			{
				input_init[i] = 0;
				output_init[i] = 0;
			}

			vindex = findex;
			for (k=0 ; k < FILTERHISTORYLENGTH ; k++)
			{

				vindex = (vindex + m - 1) % m;
				j = vindex;

				input_init[j] = history[k];
				y = num_set[0] * history[k];
				j = (j + 1) % m;
				for (i=1 ; i < FILTERLENGTH ; i++, j = (j + 1) % m)
				{
					y = y + num_set[i] * input_init[j] - den_set[i] * output_init[j];
				}
				if (!_finite(y))
					y = 0;
				output_init[vindex] = y;
			}
			for (i=0 ; i < FILTERLENGTH ; i++)
			{
				num[i] = num_set[i];
				den[i] = den_set[i];
				input[i] = input_init[i];
				output[i] = output_init[i];
			}
			findex = vindex;
		}
		else
		{
			for (i=0 ; i < FILTERLENGTH ; i++)
			{
				num[i] = num_set[i];
				den[i] = den_set[i];
			}
		}
	}


	if (mintransient)
	{
		MoveMemory(history, history +1, (FILTERHISTORYLENGTH -1) * sizeof(double));
		history[FILTERHISTORYLENGTH-1] = sample;
	}

	findex = (findex + m - 1) % m;
	j = findex;

	input[j] = sample;

	y = num[0] * sample;
	j = (j + 1) % m;
	for (i=1 ; i < FILTERLENGTH ; i++, j = (j + 1) % m)
	{
		y = y + num[i] * input[j] - den[i] * output[j];
	}

	if (!_finite(y))
		y = 0;
	output[findex] = y;
	return y;
}

void Filter::Set_LowPass(double frequency, long samplerate, double qFactor)
{
double a0,a1,a2,b0,b1,b2,w0,alpha;
double sinw0,cosw0;
double sinhq;

	w0 = frequency * 2.0 * PI / (double) samplerate;
	cosw0 = trig.cos(w0);
	sinw0 = trig.sin(w0);
	sinhq = trig.sinh(1.0 / (2.0 * qFactor));

	//alpha = sinw0 / (2.0 * qFactor);
	alpha = sinw0 * sinhq;

	b0 = (1.0 - cosw0)/2.0;
	b1 = 1.0 - cosw0;
	b2 = (1.0 - cosw0)/2.0;
	
	a0 = (1.0 + alpha);
	a1 = -2.0 * cosw0;
	a2 = (1.0 - alpha);

	num_set[0] = b0 / a0;
	num_set[1] = b1 / a0;
	num_set[2] = b2 / a0;

	den_set[0] = 0;
	den_set[1] = a1 / a0;
	den_set[2] = a2 / a0;

	iirfilterchange = 1;
}

void Filter::Set_HiPass(double frequency, long samplerate, double qFactor)
{
double a0,a1,a2,b0,b1,b2,w0,alpha;
double sinw0,cosw0;
double sinhq;

	w0 = frequency * 2.0 * PI / (double) samplerate;
	cosw0 = trig.cos(w0);
	sinw0 = trig.sin(w0);
	sinhq = trig.sinh(1.0 / (2.0 * qFactor));
	alpha = sinw0 * sinhq;

	b0 = (1.0 + cosw0)/2.0;
	b1 = -(1.0 + cosw0);
	b2 = (1.0 + cosw0)/2.0;

	a0 = 1.0 + alpha;
	a1 = -2.0*cosw0;
	a2 = 1.0 - alpha;

	num_set[0] = b0 / a0;
	num_set[1] = b1 / a0;
	num_set[2] = b2 / a0;

	den_set[0] = 0;
	den_set[1] = a1 / a0;
	den_set[2] = a2 / a0;

	iirfilterchange = 1;
}

void Filter::Set_BandPass(double frequency, long samplerate, double qFactor)
{
double a0,a1,a2,b0,b1,b2,w0,alpha;
double sinw0,cosw0;
double sinhq;
double k,r,bw;
k,r,bw;

	w0 = frequency * 2.0 * PI / (double) samplerate;
	w0=w0/1;
	cosw0 = trig.cos(w0);
	sinw0 = trig.sin(w0);

	sinhq = trig.sinh(1.0 / (2.0 * qFactor));
	alpha = sinw0 * sinhq;

	b0 = alpha;
	b1 = 0;
	b2 = -alpha;

	a0 = 1 + alpha;
	a1 = -2*cosw0;
	a2 = 1 - alpha;

	num_set[0] = b0 / a0;
	num_set[1] = b1 / a0;
	num_set[2] = b2 / a0;

	den_set[0] = 0;
	den_set[1] = a1 / a0;
	den_set[2] = a2 / a0;

	iirfilterchange = 1;
}

void Filter::Set_Resonance(double frequency,long samplerate, double qFactor,double dBgain)
{
double a0,a1,a2,b0,b1,b2,w0,alpha,A;
double sinw0,cosw0;
double sinhq;

	w0 = frequency * 2.0 * PI / (double) samplerate;
	cosw0 = trig.cos(w0);
	sinw0 = trig.sin(w0);
	sinhq = trig.sinh(1.0 / (2.0 * qFactor));
	
	alpha = sinw0 / (2.0 * qFactor);
	
	A = sqrt(pow(10, dBgain / 20));

	b0 = 1 + alpha * A;
	b1 = -2*cosw0;
	b2 = 1 - alpha * A;

	a0 = 1 + alpha / A;
	a1 = -2*cosw0;
	a2 = 1 - alpha / A;

	num_set[0] = b0 / a0;
	num_set[1] = b1 / a0;
	num_set[2] = b2 / a0;

	den_set[0] = 0;
	den_set[1] = a1 / a0;
	den_set[2] = a2 / a0;

	iirfilterchange = 1;
}

void Filter::Set_HighShelf(double frequency,long samplerate, double slope,double dBgain)
{
double a0,a1,a2,b0,b1,b2,w0,beta,A;
double sinw0,cosw0;

	w0 = frequency * 2.0 * PI / (double) samplerate;
	cosw0 = trig.cos(w0);
	sinw0 = trig.sin(w0);

	A = sqrt(pow(10, dBgain / 20));
	beta = sqrt( (A*A + 1)/slope - (A-1)*(A-1));

	b0 =    A*( (A+1) + (A-1)*cosw0 + beta*sinw0);
	b1 = -2*A*( (A-1) + (A+1)*cosw0);
	b2 =    A*( (A+1) + (A-1)*cosw0 - beta*sinw0);
	a0 =        (A+1) - (A-1)*cosw0 + beta*sinw0;
	a1 =    2*( (A-1) - (A+1)*cosw0);
	a2 =        (A+1) - (A-1)*cosw0 - beta*sinw0;

	num_set[0] = b0 / a0;
	num_set[1] = b1 / a0;
	num_set[2] = b2 / a0;

	den_set[0] = 0;
	den_set[1] = a1 / a0;
	den_set[2] = a2 / a0;
	iirfilterchange = 1;
}

void Filter::CleanSync()
{
	if (coef)
	{		
		if (m_bCoefNeedClean)
			VirtualFree(coef, 0, MEM_RELEASE);
	}
	if (buf)		
		VirtualFree(buf, 0, MEM_RELEASE);

	coef = 0;
	buf = 0;
	firLength = 0;
	bufferPos=0;
	bufferLength = 0;
	m_bCoefNeedClean = false;
}

long Filter::AllocSync(unsigned long length, unsigned long interpolation)
{
	CleanSync();
	m_bCoefNeedClean = true;
	length = length | 1;
	firLength = length;
	assert((firLength - 1) > interpolation);
	bufferLength = (firLength - 1) / interpolation + 1;	
	Filter::interpolation = interpolation;

	

	coef = (double *)VirtualAlloc(NULL, sizeof(double) * (firLength), MEM_COMMIT, PAGE_READWRITE);	
	if (coef == 0L)
		goto fail;
	buf = (double *)VirtualAlloc(NULL, sizeof(double) * (bufferLength), MEM_COMMIT, PAGE_READWRITE);
	if (buf == 0L)
		goto fail;

	return 0;
fail:
	CleanSync();
	return 1;
}

long Filter::AllocSync(unsigned long length, unsigned long interpolation, double *sharedCoef)
{
const int padding = 20;
	CleanSync();
	length = length | 1;

	firLength = length;
	assert((firLength - 1) > interpolation);
	bufferLength = (firLength - 1) / interpolation;	
	Filter::interpolation = interpolation;

	coef = sharedCoef;
	if (coef == 0L)
		goto fail;
	buf = (double *)VirtualAlloc(NULL, sizeof(double) * (bufferLength + padding), MEM_COMMIT, PAGE_READWRITE);
	if (buf == 0L)
		goto fail;

	m_bCoefNeedClean = true;
	return 0;
fail:
	CleanSync();
	return 1;
}

extern DWORD gCurrentClock;

double *Filter::GetCoefficient()
{
	return coef;
}

void Filter::CreateFIRKernel(double frequency,long samplerate)
{
unsigned long i;
double f,m;
double g;
int fpError=_FPCLASS_ND | _FPCLASS_PD | _FPCLASS_NINF | _FPCLASS_PINF;

	fc = f = frequency * 2.0 * PI / (double) samplerate;

	m = firLength;
	g=0;
	for (i=0 ; i < firLength ; i++)
	{
		if ((i - firLength/2) == 0)
			coef[i] = f;
		else
			coef[i] = ::sin(f * (i - m/2)) / (i - m/2);

		coef[i] *= (0.42 - 0.5 * ::cos(2 * PI * (double)(i) / m) + 0.08 * ::cos(4 * PI * (double)i / m));
		g += coef[i];
	}
	for (i=0 ; i < firLength ; i++)
	{
		coef[i] /= g;
		if (_fpclass(coef[i]) & fpError)
			coef[i] = 0;

	}
}

double Filter::FIR_ProcessSample(double sample)
{
double t,y;
unsigned long i;
double *c;
double *b;
double mid;
double d;

	b = buf;
	c = coef;
	t = *b;
	MoveMemory(b, b +1, (firLength -2) * sizeof(double));

	mid = buf[firLength/2 -1];
	d =0;
	y = *c * t;

	c++;
	for (i=1 ; i < firLength-1 ; i++ , c++ ,b++)
	{
		y += *c * *b;
	}

	y += *c * sample;
	*b = sample;
	return y;	
}

double Filter::FIR_ProcessSample2x(double sample, double *sampleout2)
{
double t,y,y2;
unsigned long i;
double *c;
double *b;
unsigned bufcopylen;
unsigned fircount;

	bufcopylen = (firLength - 1) / 2 -1;
	fircount = (firLength -1 - 2) / 2;
	b = buf;
	c = coef;
	t = *b;
	MoveMemory(b, b +1, bufcopylen * sizeof(double));

	y = *c * t;
	y2 = 0;
	c++;
	for (i=0 ; i < fircount ; i++ , c++ ,b++)
	{
		y2 += *c * *b;
		c++;
		y += *c * *b;
	}
	y2 += *c * sample;
	c++;
	y += *c * sample;
	*b = sample;

	*sampleout2 = y2;
	return y;	
}


void Filter::FIR_ProcessSampleNx_IndexTo8(unsigned long index, double *sampleout)
{
double t;//,y;//,y2;
unsigned long i;
double *c;
double *b;
unsigned fircount;
int j,k,m;
unsigned databuflen;
unsigned long interp;
int STDBUFFERSIZE = 9;

	interp = interpolation;
	fircount = (firLength -1 -interp) / interp;
	databuflen = bufferLength;

	//assert(STDBUFFERSIZE <= interp);
	if (STDBUFFERSIZE > (signed int) interp)
		STDBUFFERSIZE = interp;

	j= bufferPos;
	t = buf[j];
	c = coef;

	m = (index / interp) + 1;
	j = (j + m) % databuflen;
	b = &buf[j];
	index = index % interp;

	ZeroMemory(sampleout, sizeof(double) * STDBUFFERSIZE);
	index++;
	for (k=STDBUFFERSIZE-index-1; k>=0; k--, c++)
	{
		sampleout[k] = *c * t;
	}

	c += (interp - STDBUFFERSIZE);
	for (i=0 ; i <= fircount ; i++ , c += (interp-STDBUFFERSIZE), b++, j = (j + 1) % databuflen)
	{
		assert(c >= coef && c < (coef+firLength+STDBUFFERSIZE));
		assert(b >= buf && b < (buf+firLength));
		if (j==0)
			b = buf;

		for (k = STDBUFFERSIZE-1 ; k >= 0; k--, c++)
			sampleout[k] += *c * *b;
	}

}

void Filter::FIR_ProcessSample7x(double sample, double *sampleout)
{
double t;
unsigned long i;
double *c;
double *b;
unsigned bufcopylen;
unsigned fircount;

	bufcopylen = (firLength - 1) / 7 -1;
	fircount = (firLength -1 -7) / 7;
	b = buf;
	c = coef;
	t = *b;
	MoveMemory(b, b +1, bufcopylen * sizeof(double));

	sampleout[0] = *c * t;
	sampleout[1] = 0;
	sampleout[2] = 0;
	sampleout[3] = 0;
	sampleout[4] = 0;
	sampleout[5] = 0;
	sampleout[6] = 0;
	c++;
	for (i=0 ; i < fircount ; i++ , c++ ,b++)
	{
		sampleout[6] += *c * *b;
		c++;
		sampleout[5] += *c * *b;
		c++;
		sampleout[4] += *c * *b;
		c++;
		sampleout[3] += *c * *b;
		c++;
		sampleout[2] += *c * *b;
		c++;
		sampleout[1] += *c * *b;
		c++;
		sampleout[0] += *c * *b;
	}
	sampleout[6] += *c * sample;
	c++;
	sampleout[5] += *c * sample;
	c++;
	sampleout[4] += *c * sample;
	c++;
	sampleout[3] += *c * sample;
	c++;
	sampleout[2] += *c * sample;
	c++;
	sampleout[1] += *c * sample;
	c++;
	sampleout[0] += *c * sample;
	*b = sample;
}

double Filter::fir_process_sampleNx_index(int index)
{
double t,y;
unsigned long i;
double *c;
double *b;
unsigned fircount;
int j;
unsigned databuflen;
unsigned long interp;

	interp = interpolation;
	fircount = (firLength -1 -interp) / interp;
	databuflen = bufferLength;

	j= bufferPos;
	t = buf[j];
	c = coef;
	j = (j + 1) % databuflen;
	b = &buf[j];

	if (index == 0)
		y =  *c * t;
	else
		y = 0;
	c+= interp - index;
	for (i=0 ; i <= fircount ; i++ , c+=interp ,b++, j = (j + 1) % databuflen)
	{
		assert(c >= coef && c < (coef+firLength));
		assert(b >= buf && b < (buf+firLength));
		if (j==0)
			b = buf;
		y += *c * *b;
	}
	return y;
}

void Filter::fir_buffer_sampleNx(double sample)
{
	buf[bufferPos] = sample;
	bufferPos = (bufferPos + 1) % bufferLength;
}

void Filter::Set_SVF(double frequency, double samplerate,double resonance)
{
double a,b,f;

	svfF = 2 * trig.sin(PI * frequency / samplerate);
	f = 2 * trig.sin(PI * (frequency+100) / samplerate);

	a = 1 - (1 - .35) * (resonance / 15.0);

	if (f >= PI/3)
	{
	
		b = 2.0/f - f*0.5;
		if (a > b)
			a = b;

		if (f >= 1)
		{
			b = 1 / f;
			if (a > b)
				a = b;
		}
		
		b = 2 - f;
		if (a > b)
			a = b;
		
		b= (- f + sqrt(f * f + 8)) / 2;
		if (a > b)
			a = b;
	}
	svfQ = a;
}

void Filter::SVF_ProcessSample(double sample)
{
double min = 1.0e-300;

	np  = sample - svfQ * bp;
	if (fabs(np)<min)
		np = 0;
	lp = lp + svfF * bp;
	if (fabs(lp)<min)
		lp = 0;
	hp = np - lp;
	if (fabs(hp)<min)
		hp = 0;
	bp = svfF * hp + bp;
	if (fabs(bp)<min)
		bp = 0;

	if (!_finite(lp) | !_finite(bp) | !_finite(hp) | !_finite(np))
		lp = bp = hp = np = 0;

	peek = lp - bp;
}

TrigTable::TrigTable() : sinTable(0L) , cosTable(0L), sinhTable(0L), resolution(0)
{
}

long TrigTable::init(long resolution)
{
double a;
long i;

	cleanup();

	sinTable = (double *)GlobalAlloc(GMEM_FIXED, sizeof(double) * (resolution + 1));
	if (sinTable == 0L)
		goto fail;

	cosTable = (double *)GlobalAlloc(GMEM_FIXED, sizeof(double) * (resolution + 1));
	if (cosTable == 0L)
		goto fail;

	sinhTable = (double *)GlobalAlloc(GMEM_FIXED, sizeof(double) * (resolution + 1));
	if (sinhTable == 0L)
		goto fail;

	this->resolution = resolution;

	for (i=0 ; i <= resolution ; i++)
	{
		a = 2.0 * PI * (double)i / (double)resolution;
		sinTable[i] = ::sin(a);
		cosTable[i] = ::cos(a);
		sinhTable[i] = ::sinh(a);
	}
	
	this->resolution = resolution;
	return 0;
fail:
	cleanup();
	return -1;
}

void TrigTable::cleanup()
{
	if (sinTable)
		GlobalFree(sinTable);
	if (cosTable)
		GlobalFree(cosTable);
	if (sinhTable)
		GlobalFree(sinhTable);

	sinTable = 0L;
	cosTable = 0L;
	sinhTable = 0L;
	resolution = 0;
}

TrigTable::~TrigTable()
{
	cleanup();
}


double TrigTable::sin(double a)
{
long i;
	if (resolution == 0)
		return 0;

	if (fabs(a) >= (2.0 * PI))
		a = fmod(a, 2.0 * PI);

	if (fabs(a)<0.0024)
		return a;
	if (a >= 0)
	{
		i = (long)((double)resolution * a / (2.0 * PI));
		return sinTable[i];
	}
	else
	{
		i = (long)((double)resolution * -a / (2.0 * PI));
		return -sinTable[i];
	}
}

double TrigTable::cos(double a)
{
long i;
	if (resolution == 0)
		return 0;

	if (fabs(a) >= (2.0 * PI))
		a = fmod(a, 2.0 * PI);

	if (fabs(a)<0.0024)
		return a;

	if (a >= 0)
		i = (long)((double)resolution * a / (2.0 * PI));
	else
		i = (long)((double)resolution * -a / (2.0 * PI));
	return cosTable[i];
}


double TrigTable::sinh(double a)
{
long i;
	if (resolution == 0)
		return 0;

	if (fabs(a) >= (2.0 * PI))
		a = fmod(a, 2.0 * PI);

	if (a >= 0)
	{
		i = (long)((double)resolution * a / (2.0 * PI));
		return sinhTable[i];
	}
	else
	{
		i = (long)((double)resolution * -a / (2.0 * PI));
		return -sinhTable[i];
	}
}
