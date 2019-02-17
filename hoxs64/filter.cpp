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
	isSharedCoefficient = false;
	coef = 0;
	buf = 0;
	firLength = 0L;
	lp = hp = bp = np = peek=0;
	svfF = 0;
	svfQ = 0.01;	
	return S_OK;
}

void Filter::CleanSync()
{
	if (coef)
	{		
		if (!isSharedCoefficient)
		{
			VirtualFree(coef, 0, MEM_RELEASE);
		}
	}

	if (buf)		
	{
		VirtualFree(buf, 0, MEM_RELEASE);
	}

	coef = 0;
	buf = 0;
	firLength = 0;
	bufferPos = 0;
	bufferLength = 0;
	isSharedCoefficient = false;
}

long Filter::AllocSync(unsigned long length, unsigned long interpolation)
{
	CleanSync();
	isSharedCoefficient = false;
	length = length | 1;
	firLength = length;
	assert((firLength - 1) > interpolation);
	bufferLength = (firLength) / interpolation + 1;	
	Filter::interpolation = interpolation;
	coef = (double *)VirtualAlloc(NULL, sizeof(double) * (firLength), MEM_COMMIT, PAGE_READWRITE);	
	if (coef == 0L)
	{
		goto fail;
	}

	buf = (double *)VirtualAlloc(NULL, sizeof(double) * (bufferLength), MEM_COMMIT, PAGE_READWRITE);
	if (buf == 0L)
	{
		goto fail;
	}

	return 0;
fail:
	CleanSync();
	return 1;
}

long Filter::AllocSyncShared(unsigned long length, unsigned long interpolation, double *sharedCoef)
{
const int padding = 20;
	CleanSync();
	length = length | 1;
	firLength = length;
	assert((firLength - 1) > interpolation);
	bufferLength = (firLength) / interpolation + 1;	
	Filter::interpolation = interpolation;
	coef = sharedCoef;
	if (coef == 0L)
	{
		goto fail;
	}

	isSharedCoefficient = true;
	buf = (double *)VirtualAlloc(NULL, sizeof(double) * (bufferLength + padding), MEM_COMMIT, PAGE_READWRITE);
	if (buf == 0L)
	{
		goto fail;
	}
	
	return 0;
fail:
	CleanSync();
	return 1;
}

void Filter::CreateFIRKernel(double frequency,long samplerate)
{
unsigned long i;
double f,m;
double g;
int fpError=_FPCLASS_ND | _FPCLASS_PD | _FPCLASS_NINF | _FPCLASS_PINF;

	f = frequency * 2.0 * PI / (double) samplerate;

	// firLength should be an odd number, so m should be even.
	m = firLength - 1;
	g=0;
	for (i=0 ; i < firLength ; i++)
	{
		double n = (double)i;

		if ((i - m/2) == 0)
		{
			// The central part of the Sinc function curve.
			coef[i] = f;
		}
		else
		{
			// The Sinc function curve.
			coef[i] = ::sin(f * (i - m/2)) / (i - m/2);
		}

		// Blackman window for tapered smoothing of the Sinc function.
		//const double a0=0.42;
		//const double a1=0.5;
		//const double a2=0.08;
		coef[i] *= (0.42 - 0.5 * ::cos(2 * PI * (double)i / m) + 0.08 * ::cos(4 * PI * (double)i / m));

		// Sum all coefficients so we can make the area of the curve equal to unity.
		g += coef[i];
	}


	// Make the area of the curve equal to unity.
	for (i=0 ; i < firLength ; i++)
	{
		coef[i] /= g;
		if (_fpclass(coef[i]) & fpError)
		{
			coef[i] = 0;
		}
	}
}

/*
InterpolateNextSample2x

Summary
=======
Performs a x2 interpolation of successive samples. Two samples are produced for each input sample.

Parameters
==========

[sample]
The next input sample.

[sampleout]
A pointer to the second interpolated sample. sampleout2 must be allocated ny the caller.

[return]
The first interpolated sample.

Member Inputs
=============

[buf]
A pointer to the input sample buffer. This buffer operates as a ring buffer 
whose first sample is pointed to by [bufferPos].

[bufferPos]
Index of the first sample in the buffer.

[coef]
The filter kernel. A pointer to the coefficients,

[fircount]
The length of the filter kernel.
*/
double Filter::InterpolateNextSample2x(double sample, double *sampleout2)
{
double y,y2;
unsigned long i;
double *c;
double *b;
unsigned bufLen;
unsigned int j;
unsigned fircount;
const unsigned long interp = 2;

	// The filter kernal length is assumed to be an odd number.
	assert((firLength & 1) == 1);
	fircount = (firLength -1) / interp;
	bufLen = bufferLength;
	bufferPos = bufferPos % bufLen;
	b = buf;
	c = coef;
	j = bufferPos;
	b[j] = sample;
	j = (j + 1) % bufLen; 
	bufferPos = j;

	// The second output sample running total y2 is staggered from y by one position.
	// y2 misses the first input sample but y misses the final input sample.
	y = *c * buf[j];
	y2 = 0;
	j = (j + 1) % bufLen;
	b = &buf[j];
	for (i=0; i < fircount; i++, c++, b++, j = (j + 1) % bufLen)
	{
		assert(c >= coef && c < (coef+firLength));
		if (j == 0)
		{
			b = buf;
		}

		y2 += *c * *b;
		c++;
		assert(c >= coef && c < (coef+firLength));
		y += *c * *b;
	}

	if (j == 0)
	{
		b = buf;
	}

	assert(c >= coef && c < (coef+firLength));

	// y2 incorporates the final input sample.
	y2 += *c * sample;
	*sampleout2 = y2;
	return y;	
}


void Filter::FIR_ProcessSampleNx_IndexTo8(unsigned long index, double *sampleout)
{
double t;
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
	if (STDBUFFERSIZE > (signed int) interp)
	{
		STDBUFFERSIZE = interp;
	}

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
		{
			b = buf;
		}

		for (k = STDBUFFERSIZE-1 ; k >= 0; k--, c++)
		{
			sampleout[k] += *c * *b;
		}
	}
}

/*************************************
function: InterpolateQueuedSamples

Summary
=======
This routine interpolates the samples in the sample buffer [buf] by a factor 
[interpolation] and performs convolution with a filter kernel in [coef].

Paremeters
==========
[index]
The starting index with in the assumed zero padded samples.
The value should range from 0 to (interpolation - 1)
If index == 0 then we are starting with an actual input sample. 
If index != 0 then we starting with an assumed padding zero.

Member Inputs
=============

[buf]
A pointer to the input sample buffer. This buffer operates as a ring buffer 
whose first sample is pointed to by [bufferPos].

[bufferPos]
Index of the first sample in the buffer.

[coef]
The filter kernel. A pointer to the coefficients,

[fircount]
The length of the filter kernel.

[interpolation]
The interpolation count. interpolation should be greater than zero. A value
of 1 means there is no interpolation. An interpolation filter should 
effectively zero pad the samples. The number of padding zeros for each 
sample is: (interpolation - 1). This zero padding does not physically occur.
The zeros do not contribute to the convolution calcuation and are therefore 
skipped by indexing the filter kernel at a skip interval equal to the 
interpolation count. 

******************************************************/
double Filter::InterpolateQueuedSamples(int index)
{
double t;
double y;
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
	j = bufferPos;
	c = coef;
	if (index == 0)
	{
		t = buf[j];
		// We are starting with an actual sample.
		// Mutiply the very first kernel coefficient with our first sample.
		y =  *c * t;
	}
	else
	{
		// We are starting with a padding zero.
		y = 0;
	}

	j = (j + 1) % databuflen;
	b = &buf[j];

	// Fix up our kernel pointer c into the kernel coefficients to point to the next 
	// kernel coefficient that does not get multiplied by a padding zero.
	// If we started on a padding zero then there is [index] less distance to go
	// to the next significant kernel coefficient.
	c += interp - index;
	for (i = 0; i <= fircount; i+=1, c+=interp, b++, j = (j + 1) % databuflen)
	{
		assert(c >= coef && c < (coef+firLength));
		if (j == 0)
		{
			// Reset the ring buffer pointer to the start.
			b = buf;
		}

		// Mutiply the next non trivial kernel coefficient with the next sample in the buffer.
		y += *c * *b;
	}

	return y;
}

/*************************************
function: InterpolateQueuedSamples

Summary
=======
Store the next sample into the ring buffer.
**************************************/
void Filter::QueueNextSample(double sample)
{
	buf[bufferPos] = sample;
	bufferPos = (bufferPos + 1) % bufferLength;
}

void Filter::Set_SVF(double frequency, double samplerate, double resonance)
{
double a,b,f;

	svfF = 2 * trig.sin(PI * frequency / samplerate);
	f = 2 * trig.sin(PI * (frequency+100) / samplerate);
	a = 1 - (1 - .35) * (resonance / 15.0);
	if (f >= PI/3)
	{
		b = 2.0/f - f*0.5;
		if (a > b)
		{
			a = b;
		}

		if (f >= 1)
		{
			b = 1 / f;
			if (a > b)
			{
				a = b;
			}
		}
		
		b = 2 - f;
		if (a > b)
		{
			a = b;
		}
		
		b= (- f + sqrt(f * f + 8)) / 2;
		if (a > b)
		{
			a = b;
		}
	}

	svfQ = a;
}

void Filter::SVF_ProcessNextSample(double sample)
{
double min = 1.0e-300;

	np  = sample - svfQ * bp;
	if (fabs(np)<min)
	{
		np = 0;
	}

	lp = lp + svfF * bp;
	if (fabs(lp)<min)
	{
		lp = 0;
	}

	hp = np - lp;
	if (fabs(hp)<min)
	{
		hp = 0;
	}

	bp = svfF * hp + bp;
	if (fabs(bp)<min)
	{
		bp = 0;
	}

	if (!_finite(lp) | !_finite(bp) | !_finite(hp) | !_finite(np))
	{
		lp = bp = hp = np = 0;
	}

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
	{
		goto fail;
	}

	cosTable = (double *)GlobalAlloc(GMEM_FIXED, sizeof(double) * (resolution + 1));
	if (cosTable == 0L)
	{
		goto fail;
	}

	sinhTable = (double *)GlobalAlloc(GMEM_FIXED, sizeof(double) * (resolution + 1));
	if (sinhTable == 0L)
	{
		goto fail;
	}

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
	{
		GlobalFree(sinTable);
	}

	if (cosTable)
	{
		GlobalFree(cosTable);
	}

	if (sinhTable)
	{
		GlobalFree(sinhTable);
	}

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
	{
		return 0;
	}

	if (fabs(a) >= (2.0 * PI))
	{
		a = fmod(a, 2.0 * PI);
	}

	if (fabs(a)<0.0024)
	{
		return a;
	}

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
	{
		return 0;
	}

	if (fabs(a) >= (2.0 * PI))
	{
		a = fmod(a, 2.0 * PI);
	}

	if (fabs(a)<0.0024)
	{
		return a;
	}

	if (a >= 0)
	{
		i = (long)((double)resolution * a / (2.0 * PI));
	}
	else
	{
		i = (long)((double)resolution * -a / (2.0 * PI));
	}

	return cosTable[i];
}


double TrigTable::sinh(double a)
{
long i;
	if (resolution == 0)
	{
		return 0;
	}

	if (fabs(a) >= (2.0 * PI))
	{
		a = fmod(a, 2.0 * PI);
	}

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
