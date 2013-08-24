#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "bits.h"
#include "carray.h"
#include "mlist.h"
#include "huff.h"
#include "FDI.h"
#include "crc.h"


/***********************************************************************************************************************
FDIData Class
***********************************************************************************************************************/
FDIData::FDIData()
{
	data = 0;
}

FDIData::~FDIData()
{
	if (data)
	{
		GlobalFree(data);
		data = 0;
	}
}

/***********************************************************************************************************************
FDIStreamsHeader Class
***********************************************************************************************************************/
FDIStreamsHeader::FDIStreamsHeader()
{
	aveData=0;
	minData=0;
	maxData=0;
	idxData=0;
	numPulses=0;
	aveSize=0;
	aveCompression=0;
	minSize=0;
	minCompression=0;
	maxSize=0;
	maxCompression=0;
	idxSize=0;
	idxCompression=0;
}

FDIStreamsHeader::~FDIStreamsHeader()
{
	if (aveData)
	{
		GlobalFree(aveData);
		aveData=0;
	}
	if (minData)
	{
		GlobalFree(minData);
		minData=0;
	}
	if (maxData)
	{
		GlobalFree(maxData);
		maxData=0;
	}
	if (idxData)
	{
		GlobalFree(idxData);
		idxData=0;
	}
}

/***********************************************************************************************************************
FDIStream Class
***********************************************************************************************************************/
FDIStream::FDIStream()
{
	data=0;
	highBitNumber=0;
	lowBitNumber=0;
};

FDIStream::~FDIStream()
{
	if (data)
	{
		GlobalFree(data);
		data = 0;
	}
}
