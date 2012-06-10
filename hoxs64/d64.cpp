#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "carray.h"
#include "mlist.h"
#include "bits.h"
#include "util.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "register.h"
#include "errormsg.h"
#include "huff.h"
#include "crc.h"
#include "defines.h"
#include "d64.h"


//bit8 D64_extract_binary_nibble (bit8 *src,unsigned long i);
//long D64_GCR_to_Binary(bit8 *src, bit8 *dest, unsigned long length);

const bit8 GCRDISK::gcr_table[16]=
{
	0x0a,0x0b,0x12,0x13,0x0e,0x0f,0x16,0x17,
	0x09,0x19,0x1a,0x1b,0x0d,0x1d,0x1e,0x15
};

const bit8 GCRDISK::gcr_reverse_table[32]=
{
	0xff,/*00*/
	0xff,/*01*/
	0xff,/*02*/
	0xff,/*03*/
	0xff,/*04*/
	0xff,/*05*/
	0xff,/*06*/
	0xff,/*07*/
	0xff,/*08*/
	0x08,/*09*/
	0x00,/*0a*/
	0x01,/*0b*/
	0xff,/*0c*/
	0x0c,/*0d*/
	0x04,/*0e*/
	0x05,/*0f*/
	0xff,/*10*/
	0xff,/*11*/
	0x02,/*12*/
	0x03,/*13*/
	0xff,/*14*/
	0x0f,/*15*/
	0x06,/*16*/
	0x07,/*17*/
	0xff,/*18*/
	0x09,/*19*/
	0x0a,/*1a*/
	0x0b,/*1b*/
	0xff,/*1c*/
	0x0d,/*1d*/
	0x0e,/*1e*/
	0xff/*1f*/
};


const bit8 GCRDISK::D64_BAM[256]=
{
	0x12,0x01,0x41,0x00,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x15,0xFF,0xFF,0x1F,0x15,0xFF,0xFF,0x1F,
	0x11,0xFC,0xFF,0x07,0x13,0xFF,0xFF,0x07,
	0x13,0xFF,0xFF,0x07,0x13,0xFF,0xFF,0x07,
	0x13,0xFF,0xFF,0x07,0x13,0xFF,0xFF,0x07,
	0x13,0xFF,0xFF,0x07,0x12,0xFF,0xFF,0x03,
	0x12,0xFF,0xFF,0x03,0x12,0xFF,0xFF,0x03,
	0x12,0xFF,0xFF,0x03,0x12,0xFF,0xFF,0x03,
	0x12,0xFF,0xFF,0x03,0x11,0xFF,0xFF,0x01,
	0x11,0xFF,0xFF,0x01,0x11,0xFF,0xFF,0x01,
	0x11,0xFF,0xFF,0x01,0x11,0xFF,0xFF,0x01,
	0x48,0x4F,0x58,0x53,0x36,0x34,0xA0,0xA0,    //HOXS64  
	0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,
	0xA0,0xA0,0x36,0x34,0xA0,0x32,0x41,0xA0,    //64 2A
	0xA0,0xA0,0xA0,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

//Decodes 5 GCR bits to a binary nibble (4 bits)
//src: Pointer to GCR data.
//i: bit wise index into the GCR data to start decoding.
//return: return a byte whose low nibble is the decoded result. The high nibble is zero on success.
//if the 5 bits in the source buffer indexed by i is invalid GCR then the function returns 0xff
bit8 GCRDISK::D64_extract_binary_nibble (bit8 *src,unsigned long i)
{
if ((i % 8) <= 3)
	return gcr_reverse_table[(    (src[i/8L] >> (3 - (i % 8)))     &   0x1f )];
else
	return gcr_reverse_table
	[
		(    (src[i/8L] << ((i % 8) - 3))     &   0x1f ) |
		(    (src[1+(i/8L)] >> (11-(i % 8)))  &  0x1f  )
	];
}

//Decodes GCR data into a byte buffer.
//src: Pointer to GCR data.
//dest: Pointer to buffer to receive the decoded result.
//length: The number of bits to convert. length must be a multiple of 5 bits
//return: returns -1 on success. If an invalid GCR 5 bit group is found 
//then that 5 bit group is converted to 0x0 and the bit index 
//into src of the invalid data is returned.
long GCRDISK::D64_GCR_to_Binary(bit8 *src, bit8 *dest, unsigned long length)
{
long status;
unsigned long i,j,k;
bit8 t;

	status=-1;
	for (i=0,k=0 ; i<length ; i+=5,k++)
	{
		t=D64_extract_binary_nibble(src,i);
		if (status==0 && t==0xff)
		{
			status=i;
			t=0;
		}
		j=k>>1;
		if (k & 1)
			dest[j] = dest[j] | (t & 0xf);
		else
			dest[j] = (t << 4) & 0xf0;
		
	}
	return status;
}

//Encodes binary data into GCR format.
//src: Pointer to source data buffer.
//dest: Pointer to buffer that will receive the GCR encoded data.
//length is the number of bytes to convert.
void GCRDISK::D64_Binary_to_GCR(bit8 *src, bit8 *dest, long length)
{
long l;
#define hi_nib(i) gcr_table[(src[i] & 0xf0) >> 4]
#define lo_nib(i) gcr_table[(src[i] & 0xf)]

	l=length/4;

	while (l>0)
	{
		dest[0] = hi_nib(0) << 3;
		dest[0] |= (lo_nib(0) >> 2);
	
		dest[1] = lo_nib(0) << 6;
		dest[1] |= (hi_nib(1) << 1);
		dest[1] |= (lo_nib(1) >> 4);

		dest[2] = lo_nib(1) << 4;
		dest[2] |= (hi_nib(2) >> 1);

		dest[3] = hi_nib(2) << 7;
		dest[3] |= (lo_nib(2) << 2);
		dest[3] |= (hi_nib(3) >> 3);

		dest[4] = hi_nib(3) << 5;
		dest[4] |= (lo_nib(3));
		l--;
		dest+=5;
		src+=4;

	}
	switch (length & 3)
	{
	case 1:
		dest[0] = hi_nib(0) << 3;
		dest[0] |= (lo_nib(0) >> 2);
		dest[1] = lo_nib(0) << 6;
		break;
	case 2:
		dest[0] = hi_nib(0) << 3;
		dest[0] |= (lo_nib(0) >> 2);
	
		dest[1] = lo_nib(0) << 6;
		dest[1] |= (hi_nib(1) << 1);
		dest[1] |= (lo_nib(1) >> 4);

		dest[2] = lo_nib(1) << 4;
		break;
	case 3:
		dest[0] = hi_nib(0) << 3;
		dest[0] |= (lo_nib(0) >> 2);
	
		dest[1] = lo_nib(0) << 6;
		dest[1] |= (hi_nib(1) << 1);
		dest[1] |= (lo_nib(1) >> 4);

		dest[2] = lo_nib(1) << 4;
		dest[2] |= (hi_nib(2) >> 1);

		dest[3] = hi_nib(2) << 7;
		dest[3] |= (lo_nib(2) << 2);
		break;
	}
}


const struct D64_Track_Info GCRDISK::D64_info[42]=
{
	/* 0 */{21,0x00000,D64_TRACK_SIZE_1_17,3,21,0.0},
	/* 1 */{21,0x01500,D64_TRACK_SIZE_1_17,3,42,45.540390014648438},
	/* 2 */{21,0x02a00,D64_TRACK_SIZE_1_17,3,63,91.083435058593750},
	/* 3 */{21,0x03f00,D64_TRACK_SIZE_1_17,3,84,36.623184204101563},
	/* 4 */{21,0x05400,D64_TRACK_SIZE_1_17,3,105,82.177154541015625},
	/* 5 */{21,0x06900,D64_TRACK_SIZE_1_17,3,126,27.718986511230469},
	/* 6 */{21,0x07e00,D64_TRACK_SIZE_1_17,3,147,73.272232055664062},
	/* 7 */{21,0x09300,D64_TRACK_SIZE_1_17,3,168,18.801788330078125},
	/* 8 */{21,0x0a800,D64_TRACK_SIZE_1_17,3,189,64.342239379882813},
	/* 9 */{21,0x0bd00,D64_TRACK_SIZE_1_17,3,210,9.8845901489257812},
	/* 10 */{21,0x0d200,D64_TRACK_SIZE_1_17,3,231,55.424179077148438},
	/* 11 */{21,0x0e700,D64_TRACK_SIZE_1_17,3,252,0.96739196777343750},
	/* 12 */{21,0x0fc00,D64_TRACK_SIZE_1_17,3,273,46.519104003906250},
	/* 13 */{21,0x11100,D64_TRACK_SIZE_1_17,3,294,92.050186157226562},
	/* 14 */{21,0x12600,D64_TRACK_SIZE_1_17,3,315,37.601028442382813},
	/* 15 */{21,0x13b00,D64_TRACK_SIZE_1_17,3,336,83.146011352539062},
	/* 16 */{21,0x15000,D64_TRACK_SIZE_1_17,3,357,28.695968627929687},
	/* 17 */{19,0x16500,D64_TRACK_SIZE_18_24,2,376,70.550125122070313},
	/* 18 */{19,0x17800,D64_TRACK_SIZE_18_24,2,395,30.125610351562500},
	/* 19 */{19,0x18b00,D64_TRACK_SIZE_18_24,2,414,82.574050903320313},
	/* 20 */{19,0x19e00,D64_TRACK_SIZE_18_24,2,433,42.189514160156250},
	/* 21 */{19,0x1b100,D64_TRACK_SIZE_18_24,2,452,94.623962402343750},
	/* 22 */{19,0x1c400,D64_TRACK_SIZE_18_24,2,471,54.215164184570312},
	/* 23 */{19,0x1d700,D64_TRACK_SIZE_18_24,2,490,13.823242187500000},
	/* 24 */{18,0x1ea00,D64_TRACK_SIZE_24_30,1,508,83.750137329101563},
	/* 25 */{18,0x1fc00,D64_TRACK_SIZE_24_30,1,526,55.237503051757812},
	/* 26 */{18,0x20e00,D64_TRACK_SIZE_24_30,1,544,26.732643127441406},
	/* 27 */{18,0x22000,D64_TRACK_SIZE_24_30,1,562,84.666854858398438},
	/* 28 */{18,0x23200,D64_TRACK_SIZE_24_30,1,580,56.164382934570312},
	/* 29 */{18,0x24400,D64_TRACK_SIZE_24_30,1,598,27.433013916015625},
	/* 30 */{17,0x25600,D64_TRACK_SIZE_31_35,0,615,55.618392944335937},
	/* 31 */{17,0x26700,D64_TRACK_SIZE_31_35,0,632,37.806076049804688},
	/* 32 */{17,0x27800,D64_TRACK_SIZE_31_35,0,649,19.985908508300781},
	/* 33 */{17,0x28900,D64_TRACK_SIZE_31_35,0,666,83.405975341796875},
	/* 34 */{17,0x29a00,D64_TRACK_SIZE_31_35,0,683,70.061660766601563},
	/* 35 */{17,0x2ab00,D64_TRACK_SIZE_31_35,0,700,30.0},
	/* 36 */{17,0x2bc00,D64_TRACK_SIZE_31_35,0,717,82.0},
	/* 37 */{17,0x2cd00,D64_TRACK_SIZE_31_35,0,734,42.0},
	/* 38 */{17,0x2de00,D64_TRACK_SIZE_31_35,0,751,94.0},
	/* 39 */{17,0x2ef00,D64_TRACK_SIZE_31_35,0,768,54.0},
	/* 40 */{17,0x30000,D64_TRACK_SIZE_31_35,0,785,13.0},
	/* 41 */{17,0x31100,D64_TRACK_SIZE_31_35,0,802,83.0}
};

#define d64_gapheader 11
#define d64_synclength 5
#define d64_gapsector 6
#define d64_sector_header_size 8
#define d64_sector_datablock_size 256
#define D64_SECTOR_SIZE 260

struct sector_header
{
	bit8 header_id;
	bit8 check_eor; 
	bit8 sector;
	bit8 track;
	bit8 disk_id1;
	bit8 disk_id2;
	bit8 pad1;
	bit8 pad2;
};


struct sector_data
{
	bit8 header_id;
	bit8 data[256];
	bit8 checksum; 
	bit8 pad1;
	bit8 pad2;
};

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
	//nextStream = 0;
};

FDIStream::~FDIStream()
{
	if (data)
	{
		GlobalFree(data);
		data = 0;
	}
	/*
	if (nextStream)
	{
		delete nextStream;
		nextStream = 0;
	}
	*/
}

/***********************************************************************************************************************
GCRDISK Class
***********************************************************************************************************************/
GCRDISK::GCRDISK()
{
int i;
	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		trackSize[i] = 0;
		trackData[i] = 0;
		m_rawTrackData[i] = 0;
		speedZone[i] = 0;
	}

	d64Errors=0;
	m_pD64Binary=0;
	m_d64TrackCount=0;
	m_d64_protectOff=0;
	mhevtQuit = 0;
}

GCRDISK::~GCRDISK()
{
	Clean();
}


void GCRDISK::JumpBits(bit8 trackNumber,bit16 &byteIndex,bit8 &bitIndex, bit32 bitCount)
{
bit32 startBitPos;
bit32 endBitPos;

	if (trackSize[trackNumber]==0)
		return;
	startBitPos = byteIndex * 8 + bitIndex;
	endBitPos = (startBitPos + bitCount) % trackSize[trackNumber];
	byteIndex = (bit16)(endBitPos / 8);
	bitIndex = (bit8)endBitPos & 7;
}

HRESULT GCRDISK::SeekSync(bit8 trackNumber, bit16 byteIndex, bit8 bitIndex, bit32 bitScanLimit, bit8 *headerByte, bit16 *newByteIndex, bit8 *newBitIndex, bit32* jumpedBitCount)
{
bit32 bitsScannedCounter;
bit32 sync_count;
bit8 data;
bool bSyncFound;
	
	*jumpedBitCount = 0;
	*headerByte = 0;
	bSyncFound=false;
	bitsScannedCounter=0;
	sync_count=0;

	while (bitsScannedCounter < bitScanLimit)
	{
		if (IsEventQuitSignalled())
			return E_FAIL;

		data = GetByte(trackNumber, byteIndex, bitIndex);
		if ( (signed char)data < 0)
		{
			sync_count++;
			if (sync_count >= 10)
				bSyncFound = true;
		}
		else
		{
			sync_count = 0;
			if (bSyncFound)
			{
				*headerByte = data;
				*newByteIndex = byteIndex;
				*newBitIndex = bitIndex;
				return S_OK;
			}
			bSyncFound = false;
		}			
		JumpBits(trackNumber, byteIndex, bitIndex, 1);
		bitsScannedCounter++;
		(*jumpedBitCount)++;
	}
	//No Sync with specified header byte found. 
	return S_FALSE;		
}


void GCRDISK::CopyRawData(bit8 *buffer, bit8 trackNumber,bit16 byteIndex,bit8 bitIndex, bit16 count)
{
int i;
	for (i=0 ; i < count ; i++)
	{
		buffer[i] = GetByte(trackNumber, byteIndex, bitIndex);
		JumpBits(trackNumber, byteIndex, bitIndex, 8);
	}
}

//HRESULT GCRDISK::ConvertGCRToD64(bit32 tracks)
//{
//bit8 sec,tr;
//long r,st;
//struct sector_header sec_header;
//struct sector_data sec_data;
//
//	st=S_OK;
//	memset(m_pD64Binary, 0x0, MAX_D64_SIZE);
//	for(tr = 0 ; tr < tracks ; tr++)
//	{
//		for(sec=0 ; sec < D64_info[tr].sector_count ; sec++)
//		{
//			r = LoadSector(tr, sec, &sec_header, &sec_data);
//			if (r == S_OK)
//				CopyMemory(&m_pD64Binary[D64_info[tr].file_offset + (256L * sec)], &sec_data.data[0], 256);
//			else
//				st = E_FAIL;
//			if (IsEventQuitSignalled())
//				return E_FAIL;
//		}
//	}
//
//	return st;
//}

HRESULT GCRDISK::ConvertGCRToD64(bit32 tracks)
{
bit8 tr;
long r;
HRESULT st;
struct sector_header sec_header;
struct sector_data sec_data;
bit16 byteIndex;
bit8 bitIndex;
HRESULT hr;
const bit32 MAXBYTESTOSCAN = G64_MAX_BYTES_PER_TRACK * 2;
const bit32 MAXBITSTOSCAN = MAXBYTESTOSCAN * 8L;
bit32 bitCounter, jumpedBitCount;
bit8 buffer[325];
bit8 g64TrackNumber;
bit16 bytesToCopy;
bit8 maxSectorsOnThisTrack;
bit8 headerByte;

	if (tracks > D64_MAX_TRACKS)
		tracks = D64_MAX_TRACKS;
	st = S_OK;
	memset(m_pD64Binary, 0x0, MAX_D64_SIZE);
	for(tr = 0; tr < tracks; tr++)
	{
		g64TrackNumber = tr * 2;
retryhalftrack:
		ClearD64LoadStatus();
		byteIndex = 0;
		bitIndex = 0;
		bitCounter = 0;
		maxSectorsOnThisTrack = (bit8) D64_info[tr].sector_count;

		while (bitCounter < MAXBITSTOSCAN)
		{
			if (IsEventQuitSignalled())
				return E_FAIL;

			hr = SeekSync(g64TrackNumber, byteIndex, bitIndex, MAXBITSTOSCAN - bitCounter, &headerByte, &byteIndex, &bitIndex, &jumpedBitCount);
			if (FAILED(hr))
				return hr;
			bitCounter += jumpedBitCount;
			if (hr != S_OK)
				continue;
			if (headerByte != 0x52)
				continue;

headerfound:
			bytesToCopy = 10;
			CopyRawData(buffer, g64TrackNumber, byteIndex, bitIndex, bytesToCopy);
			bitCounter = bitCounter + bytesToCopy * 8;

			r = D64_GCR_to_Binary(buffer, (bit8 *) &sec_header, bytesToCopy * 8);
			if (r >= 0)
				continue;//invalid GCR
			
			if (sec_header.sector > maxSectorsOnThisTrack)
				continue;

			if (sec_header.track != (tr + 1))
				continue;

			if (get_D64LoadStatus(sec_header.sector) == GCRDISK::OK)
				continue;

			if (bitCounter >= MAXBITSTOSCAN)
				break;
			hr = SeekSync(g64TrackNumber, byteIndex, bitIndex, MAXBITSTOSCAN - bitCounter, &headerByte, &byteIndex, &bitIndex, &jumpedBitCount);
			if (FAILED(hr))
				return hr;
			bitCounter += jumpedBitCount;
			if (hr != S_OK)
				continue;

			if (headerByte == 0x52)
				goto headerfound;
			
			if (headerByte != 0x55)
				continue;

			bytesToCopy = 325;
			CopyRawData(buffer, g64TrackNumber, byteIndex, bitIndex, bytesToCopy);
			bitCounter = bitCounter + bytesToCopy * 8;
			r = D64_GCR_to_Binary(buffer, (bit8 *)&sec_data, bytesToCopy*8);
			if (r >= 0) 
				set_D64LoadStatus(sec_header.sector, GCRDISK::Bad);
			else
				set_D64LoadStatus(sec_header.sector, GCRDISK::OK);

			CopyMemory(&m_pD64Binary[D64_info[tr].file_offset + (256L * sec_header.sector)], &sec_data.data[0], 256);

			if (IsD64LoadStatusOKForD64Track(tr))
				break;
		}
		if (IsEventQuitSignalled())
			return E_FAIL;
		if (!IsD64LoadStatusOKForD64Track(tr))
		{
			int countOfSectorsOK = CountOfLoadStatus(GCRDISK::OK);
			if ((g64TrackNumber & 1) == 0 && countOfSectorsOK == 0)
			{
				g64TrackNumber++;
				if (g64TrackNumber < G64_MAX_TRACKS)
					goto retryhalftrack;
			}
			st = E_FAIL;
		}
	}

	return st;
}

//HRESULT GCRDISK::LoadSector(bit8 track_num,
//							bit8 sector_num,
//							struct sector_header *sec_header,
//							struct sector_data *sec_data)
//{
//bit32 c;
//long r;
//bit8 g64TrackNumber;
//bit16 byteIndex;
//bit8 bitIndex;
//bit8 buffer[325];
//HRESULT hr;
//
//	
//	g64TrackNumber = track_num * 2;
//
//	byteIndex=0;
//	bitIndex=0;
//	c=0;
//	while (c < 200)
//	{
//		hr = SeekSync(g64TrackNumber, 0x52, byteIndex, bitIndex, G64_MAX_BYTES_PER_TRACK * 2, &byteIndex, &bitIndex);
//		if (hr) return hr;
//		CopyRawData(buffer, g64TrackNumber, byteIndex, bitIndex, 10);
//		r = D64_GCR_to_Binary(buffer, (bit8 *)sec_header, 10*8);
//		if (r >= 0) return E_FAIL;
//		if (sec_header->sector == sector_num)
//		{
//			hr = SeekSync(g64TrackNumber, 0x55, byteIndex, bitIndex, G64_MAX_BYTES_PER_TRACK * 2, &byteIndex, &bitIndex);
//			if (hr) return hr;
//			
//
//			CopyRawData(buffer, g64TrackNumber, byteIndex, bitIndex, 325);
//			r = D64_GCR_to_Binary(buffer, (bit8 *)sec_data, 325*8);
//			if (r >= 0) return E_FAIL;
//			return S_OK;
//		}
//		c++;
//		if (IsEventQuitSignalled())
//			return E_FAIL;
//	}
//	return E_FAIL;
//}

void GCRDISK::ClearD64LoadStatus()
{
	for(int i = 0; i < _countof(d64SectorLoadStatus); i++)
	{
		d64SectorLoadStatus[i] = GCRDISK::Missing;
	}
}

GCRDISK::LoadState GCRDISK::get_D64LoadStatus(bit8 sector)
{
	assert(sector <= _countof(d64SectorLoadStatus));
	return d64SectorLoadStatus[sector];
}

void GCRDISK::set_D64LoadStatus(bit8 sector, GCRDISK::LoadState state)
{
	assert(sector <= _countof(d64SectorLoadStatus));
	d64SectorLoadStatus[sector] = state;
}

bool GCRDISK::IsD64LoadStatusOKForD64Track(bit8 track)
{
	if (track >= D64_MAX_TRACKS)
		return false;
	int sectorCount = D64_info[track].sector_count;
	if (sectorCount > D64_MAX_SECTORS)
		sectorCount = D64_MAX_SECTORS;

	assert(sectorCount <= _countof(d64SectorLoadStatus));

	for(int i = 0; i < sectorCount; i++)
	{
		if (d64SectorLoadStatus[i] != GCRDISK::OK)
		{
			return false;
		}
	}
	return true;
}

bit8 GCRDISK::CountOfLoadStatus(GCRDISK::LoadState state)
{
bit8 c = 0;
	for(int i = 0; i < _countof(d64SectorLoadStatus); i++)
	{
		if (d64SectorLoadStatus[i] == state)
			c++;
	}
	return c;
}

void GCRDISK::Clean()
{
int i;
	if (m_pD64Binary)
	{
		GlobalFree(m_pD64Binary);
		m_pD64Binary=0;
	}

	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		if (trackData[i])
			GlobalFree(trackData[i]);
		if (m_rawTrackData[i])
			GlobalFree(m_rawTrackData[i]);

		trackSize[i] = 0;
		trackData[i] = 0;
		m_rawTrackData[i] = 0;

		if (speedZone[i])
			GlobalFree(speedZone[i]);
		speedZone[i] = 0;
	}
}

HRESULT GCRDISK::Init()
{
int i;

	m_pD64Binary = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, MAX_D64_SIZE);
	if (m_pD64Binary == 0)
		goto fail;

	for (i=0 ; i < G64_MAX_TRACKS ; i++)
	{
		trackData[i] = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, G64_MAX_BYTES_PER_TRACK);
		if (trackData[i] == 0)
			goto fail;

		m_rawTrackData[i] = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, DISK_RAW_TRACK_SIZE);
		if (m_rawTrackData[i] == 0)
			goto fail;

		trackSize[i] = 0;
		speedZone[i] = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, (G64_MAX_BYTES_PER_TRACK + 3) / 4);
		if (speedZone[i] == 0)
			goto fail;
	}
	return S_OK;
fail:
	Clean();
	return SetError(E_FAIL,TEXT("Out of memory."));
}

bit16 GCRDISK::GetD64TrackSize(bit8 track)
{
	if (track >= D64_MAX_TRACKS)
		track = D64_MAX_TRACKS - 1;

	return (bit16)(D64_info[track].gcr_byte_count);
}

bit8 GCRDISK::GetD64SpeedZone(bit8 track)
{
	if (track >= D64_MAX_TRACKS)
		track = D64_MAX_TRACKS - 1;

	return D64_info[track].defaultSpeed;
}

bit8 GCRDISK::GetSpeedZone(bit8 trackNumber, bit16 byteIndex)
{
bit8 t,s;
	t = speedZone[trackNumber][byteIndex/4];
	s = (3 - (byteIndex & 3)) * 2;
	t = (t >> s) & 3;
	return t;
}


void GCRDISK::SetSpeedZone(bit8 trackNumber, bit16 byteIndex, bit8 speed)
{
bit8 t,s,mask;

	speed = speed & 3;

	t = speedZone[trackNumber][byteIndex/4];

	s = (3 - (byteIndex & 3)) * 2;

	mask = 3 << s;
	speed = speed << s;

	speedZone[trackNumber][byteIndex/4] =  t & ~mask | speed;
}


void GCRDISK::ConvertGCRtoRAW(bit8 trackNumber)
{
unsigned long i,j,len;
bit8 speed;
bit8 byte;
unsigned long sourceSize;
unsigned long sourceCounter;
unsigned long nextRawCounter;
unsigned long rawSize;
unsigned long bits;
const bool bAllowTrackStretch = true;
const bool bAllowTrackShrink = true;

	rawSize = DISK_RAW_TRACK_SIZE * 16;
	ZeroMemory(m_rawTrackData[trackNumber], DISK_RAW_TRACK_SIZE);

	sourceSize = 0;
	len = (trackSize[trackNumber] + 7) / 8;
	bits = 8;
	for (i=0 ; i < len ; i++)
	{
		if (i >= trackSize[trackNumber] / 8)
			bits = trackSize[trackNumber] & 7;

		speed = GetSpeedZone(trackNumber, (unsigned short) i);

		//TEST
		//G64 speed 3 translates to writing a 0 to the VIA speed pins
		speed = ~speed & 3;

		//four clocks per bit per 16 clocks (16Mhz clock) if the speed at VIA2 PortB 6-7 is zero
		sourceSize = sourceSize + (16 - speed) * 4 * bits;
	}

	bits = 8;
	sourceCounter = 0;
	for (i=0 ; i < len ; i++)
	{
		if (i >= trackSize[trackNumber] / 8)
			bits = trackSize[trackNumber] & 7;

		speed = GetSpeedZone(trackNumber, (unsigned short)i);

		speed = ~speed & 3;

		speed = (16 - speed) * 4;
		byte = trackData[trackNumber][i];
		for (j=0 ; j < bits ; j++)
		{
			if (((bAllowTrackStretch && sourceSize < rawSize) || (bAllowTrackShrink && sourceSize > rawSize)))
			{
				nextRawCounter = (unsigned long)(((double)sourceCounter / (double)sourceSize) * (double)rawSize);
			}
			else 
			{
				nextRawCounter = sourceCounter;				
			}
			if (nextRawCounter >= rawSize) 
				break;
			//assert((nextRawCounter/16) < DISK_RAW_TRACK_SIZE);
			if ((signed char)byte < 0)
			{
				m_rawTrackData[trackNumber][nextRawCounter/16] = ((bit8)nextRawCounter & 0xf) + 1;
			}
			byte <<= 1;
			sourceCounter += speed;
		}
		if (nextRawCounter >= rawSize) 
			break;
	}
}


void GCRDISK::ConvertRAWtoGCR()
{
bit8 tr;
	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		if (IsEventQuitSignalled())
			return;
		ConvertRAWtoGCR(tr);
	}
}

void GCRDISK::ConvertRAWtoGCR(bit8 trackNumber)
{
bit8 defaultSpeed;
long i;
long delayCounter,delay,firstDelay;
bit8 byte,calcSpeed;
long clockGapDefaultSpeed;
const long clockGap0 = ((16-0) *4);
const long clockGap1 = ((16-1) *4);
const long clockGap2 = ((16-2) *4);
const long clockGap3 = ((16-3) *4);
long gapDefaultSpeed;
long bitCount,m;
bit8 frame;
long frameIndex;
bit8 quickBits;
bit8 aClockGap[4];

	aClockGap[0] = clockGap0;
	aClockGap[1] = clockGap1;
	aClockGap[2] = clockGap2;
	aClockGap[3] = clockGap3;

	defaultSpeed = GetD64SpeedZone(trackNumber/2);

	clockGapDefaultSpeed = aClockGap[defaultSpeed];

	ZeroMemory(trackData[trackNumber], G64_MAX_BYTES_PER_TRACK);
	for (i = 0 ; i < G64_MAX_BYTES_PER_TRACK ; i++)
		SetSpeedZone(trackNumber, (bit16)i, defaultSpeed); 

	bool bFoundFirstPulse = false;
	firstDelay = 0;
	delayCounter = 0;
	quickBits=0;
	frame=0;
	frameIndex=0;
	byte=0;
	for (i = 0 ; i <= DISK_RAW_TRACK_SIZE ; i++)
	{
		if (i == DISK_RAW_TRACK_SIZE)
			//we will perform the wrap-a-round of zeros during this loop.
			delay = firstDelay;
		else
			delay = GetDisk16(trackNumber, i);

		if (delay == 0 && i != DISK_RAW_TRACK_SIZE)
		{
			//a value of zero delay here really means that 16 clocks elapsed with no disk pulse.
			delayCounter += 16;
		}
		else
		{			
			//if we are performing the wrap-a-round of zeros then a zero delay here would mean that there are no more clocks left at the end of the track.
			delayCounter += delay;

			//round to the nearest bit.
			//FIXME probably not accurate. I suspect we should round to the nearest quarter bit.
			gapDefaultSpeed = delayCounter / clockGapDefaultSpeed;
			m = delayCounter % clockGapDefaultSpeed;
			if (m >= clockGapDefaultSpeed/2)
				gapDefaultSpeed++;

			bitCount = gapDefaultSpeed;
			calcSpeed = defaultSpeed;

			//if we are performing the wrap-a-round of zeros then do not add any more 1 bits.
			if (i != DISK_RAW_TRACK_SIZE)
			{
				if (bitCount==0)
				{
					if (quickBits==0)
					{
						bitCount=1;
						quickBits=1;
					}
				}
				else 
					quickBits = 0;
			}

			if (!bFoundFirstPulse)
			{
				//the delays at both the start and end of the track are important for a calculating a correct wrap-a-round of zeros
				bFoundFirstPulse = true;
				firstDelay = delayCounter - bitCount * clockGapDefaultSpeed;
			}

			while (bitCount>0)
			{
				byte <<= 1;
				//if we are performing the wrap-a-round of zeros then do not add any more 1 bits.
				if (bitCount==1 && i != DISK_RAW_TRACK_SIZE)
					byte |= 1;

				frame++;
				if (frame >= 8)
				{
					frame=0;
					trackData[trackNumber][frameIndex] = byte;
					SetSpeedZone(trackNumber, (bit16)frameIndex, calcSpeed);
					frameIndex++;
					if (frameIndex >= G64_MAX_BYTES_PER_TRACK)
						goto done;
				}
				bitCount--;
			}
			delayCounter = 16 - delay;
		}
	}
done:
	if (frame!=0)
		//right pad the last byte with zeros
		trackData[trackNumber][frameIndex] = byte << (8 - frame);

	trackSize[trackNumber] = frameIndex * 8 + frame;
}


bit8 GCRDISK::GetSectorErrorCode(bit8 *d64Binary, bit16 errorBytes, bit8 trackNumber, bit8 sectorNumber)
{
bit32 i;
	if (errorBytes == 683)
		i = D64_info[35].file_offset;
	else if (errorBytes == 768)
		i = D64_info[40].file_offset;
	else
		return 1;

	if (trackNumber>=40)
		return 1;

	i += D64_info[trackNumber].sector_total - D64_info[trackNumber].sector_count + sectorNumber;
	return d64Binary[i];

}

void GCRDISK::MakeGCRImage(bit8 *d64Binary, bit32 tracks, bit16 errorBytes, bool bAlignD64Tracks)
{
bit32 i,j;
bit8 *pb,*pGcr,*p,sum,id1,id2,sec,tr,tr2;
bit8 d64_sector_binary[D64_SECTOR_SIZE];
bit8 tempGcrBuffer[260*5/4];
bit8 c;
bit8 sectorError;
bit8 id1x,id2x;
int wi = 0;
int trackByteLen;

	for(tr = 0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		memset(trackData[tr], 0x55, G64_MAX_BYTES_PER_TRACK);
		trackSize[tr] = GetD64TrackSize(tr/2) * 8;

		for (i=0 ; i < G64_MAX_BYTES_PER_TRACK ; i++)
			SetSpeedZone(tr, (bit16)i, GetD64SpeedZone(tr/2));

	}
	id1x = d64Binary[D64_info[17].file_offset + 162];
	id2x = d64Binary[D64_info[17].file_offset + 163];
	for(tr = 0 ; tr < (tracks * 2) ; tr++)
	{
		tr2 = tr >> 1;
		if ((tr & 1) != 0)
			continue;

		trackByteLen = trackSize[tr] / 8;
		pGcr = &trackData[tr][0];

		if (bAlignD64Tracks)
			wi = 0;
		else
			wi = ((int)(((double)(trackSize[tr]/8)) * (D64_info[tr/2].track_stagger/100.0))) % (trackSize[tr]/8);
	
		for(c=0 ; c < D64_info[tr2].sector_count ; c++)
		{
			/*sector interleave*/
			sec=c;
			if (errorBytes!=0)
				sectorError = GetSectorErrorCode(d64Binary, errorBytes, tr2, sec);
			else
				sectorError = 1;

			if (sectorError!=3)
			{
				for (j = 0 ; j < d64_synclength; j++, wi=(wi+1) % trackByteLen)
					pGcr[wi] = 0xff;
			}
			else
			{
				for (j = 0 ; j < d64_synclength; j++, wi=(wi+1) % trackByteLen)
					pGcr[wi] = 0x00;
			}

			p= &d64_sector_binary[0];
			if (sectorError!=2)
				p[0] = 0x8;
			else
				p[0] = 0x0;

			if (sectorError!=0xb)
			{
				id1 = id1x;
				id2 = id2x;
			}
			else
			{
				id1 = id1x ^ ~(sec+30);
				id2 = id2x ^ ~(sec-30);
			}

			if (sectorError!=9)
				p[1] = sec ^ (tr2+1) ^ id2 ^ id1;
			else
				p[1] = ~(sec ^ (tr2+1) ^ id2 ^ id1);
			p[2] = sec;
			p[3] = (tr2+1);
			p[4] = id2;
			p[5] = id1;
			p[6] = 0xf;
			p[7] = 0xf;

			D64_Binary_to_GCR(p, &tempGcrBuffer[0], 8);
			for (j = 0 ; j < 10; j++, wi=(wi+1) % trackByteLen)
				pGcr[wi] = tempGcrBuffer[j];

			//TEST
			//header gap changed from 9 to 11
			for (j = 0 ; j < d64_gapheader; j++, wi=(wi+1) % trackByteLen)
				pGcr[wi]=0x55;

			if (sectorError!=3)
			{
				for (j = 0 ; j < d64_synclength; j++, wi=(wi+1) % trackByteLen)
					pGcr[wi] = 0xff;
			}
			else
			{
				for (j = 0 ; j < d64_synclength; j++, wi=(wi+1) % trackByteLen)
					pGcr[wi] = 0x00;
			}

			pb = &d64Binary[D64_info[tr2].file_offset + 256 * (unsigned long)sec];
			if (sectorError!=4)
				p[0] = 0x7;
			else
				p[0] = 0x0;
			sum=0;
			for (i=0 ; i < 256 ; i++)
			{
				p[i+1] = pb[i];
				sum = sum ^ pb[i];
			}
			if (sectorError!=5)
				p[257] = sum;
			else
				p[257] = ~sum;
			p[258] = 0;
			p[259] = 0;

			D64_Binary_to_GCR(p, &tempGcrBuffer[0], 260);
			for (j = 0 ; j < (260*5/4); j++, wi=(wi+1) % trackByteLen)
				pGcr[wi] = tempGcrBuffer[j];

			for (j = 0 ; j < d64_gapsector; j++, wi=(wi+1) % trackByteLen)
				pGcr[wi]=0x55;
			
		}
	}
	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
		ConvertGCRtoRAW(tr);
}

HRESULT GCRDISK::ReadFromFile(HANDLE hfile, TCHAR *filename, char *buffer, DWORD byteCount, DWORD *bytesRead)
{
DWORD bytes_read=0;
BOOL r;
HRESULT hr;
	r = ReadFile(hfile, buffer, byteCount, &bytes_read, NULL);
	if (r==0 || byteCount!=bytes_read)
	{
		if (filename != 0)
			hr = SetError(E_FAIL,TEXT("Could not read from %s."),filename);
		else
			hr = SetError(E_FAIL,TEXT("Could not read from file."));
	}
	else
		hr = S_OK;
	if (bytesRead)
		*bytesRead = bytes_read;
	return hr;
}

HRESULT GCRDISK::ReadFromFileQ(HANDLE hfile, char *buffer, DWORD byteCount, DWORD *bytesRead)
{
	return ReadFromFile(hfile, 0, buffer, byteCount, bytesRead);
}

HRESULT GCRDISK::LoadG64FromFileHandle(HANDLE hfile, TCHAR *filename, bool bConvertToRAW)
{
HRESULT hr;
DWORD r;
struct G64Header g64Header;
bit8 tr;
unsigned long trackOffset[G64_MAX_TRACKS];
unsigned long speedOffset[G64_MAX_TRACKS];
long i;
DWORD file_size,j;
bit32 data32;
bit8 speed;
bit8 data8;
WORD s;

	ClearError();
	m_d64TrackCount = G64_MAX_TRACKS/2;

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."),filename);
	}

	hr = ReadFromFile(hfile, filename, &g64Header.signature[0], sizeof(struct G64Header), 0);
	if (FAILED(hr))
		return hr;
	if (_memicmp(&g64Header.signature[0],"GCR-1541", 8)!=0)
	{
		return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
	}
	if (g64Header.version != 0)
		return SetError(E_FAIL,TEXT("%s is not a supported G64 file version. Version 2.x only is supported."), filename);

	if (g64Header.trackSize > G64_MAX_BYTES_PER_TRACK)
		return SetError(E_FAIL,TEXT("G64 track size is too long."));

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&data32, 4, 0);
		if (FAILED(hr))
			return hr;
		if (data32 + 2 >= file_size)
			return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
		trackOffset[tr] = data32;
	}
	
	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&data32, 4, 0);
		if (FAILED(hr))
			return hr;
		if (data32 > 3)
		{
			if (data32 + 2 >= file_size)
				return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
		}
		speedOffset[tr] = data32;
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		speed = GetD64SpeedZone(tr/2);
		ZeroMemory(trackData[tr], G64_MAX_BYTES_PER_TRACK);
		for (i = 0 ; i < G64_MAX_BYTES_PER_TRACK ; i++)
			SetSpeedZone(tr, (bit16)i, speed); 
		trackSize[tr] = D64_TRACK_SIZE_1_17 * 8;
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		if (trackOffset[tr]!=0)
		{
			r = SetFilePointer (hfile, trackOffset[tr], 0L, FILE_BEGIN);
			if (r == INVALID_SET_FILE_POINTER)
			{
				return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
			}
			hr = ReadFromFile(hfile, filename, (char *)&s, 2, 0);
			if (FAILED(hr))
				return hr;

			if ((bit16)s > g64Header.trackSize)
				return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);

			if (s==0)
				continue;

			trackSize[tr] = (DWORD)s * 8L;

			hr = ReadFromFile(hfile, filename, (char *)&trackData[tr][0], s, 0);
			if (FAILED(hr))
				return hr;

			data8 = 0;
			if (speedOffset[tr] > 3)
			{
				r = SetFilePointer (hfile, speedOffset[tr], 0L, FILE_BEGIN);
				if (r == INVALID_SET_FILE_POINTER)
				{
					return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
				}
				for (j=0 ; j < (trackSize[tr] / 8) ; j++)
				{
					switch (j & 3)
					{
					case 0:
						data8=0;
						hr = ReadFromFile(hfile, filename, (char *)&data8, 1, 0);
						if (FAILED(hr))
							return hr;
						SetSpeedZone(tr, (bit16)j, (bit8)((data8 >> 6) & 3));
						break;
					case 1:
						SetSpeedZone(tr, (bit16)j, (bit8)((data8 >> 4) & 3));
						break;
					case 2:
						SetSpeedZone(tr, (bit16)j, (bit8)((data8 >> 2) & 3));
						break;
					case 3:
						SetSpeedZone(tr, (bit16)j, (bit8)(data8 & 3));
						break;
					}
				}
			}
			else
			{
				data8 = (bit8)(speedOffset[tr] & 3);
				for (j=0 ; j < (trackSize[tr] / 8) ; j++)
				{
					SetSpeedZone(tr, (bit16)j, data8);
				}
			}
		}
	}

	if (bConvertToRAW)
	{
		for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
			ConvertGCRtoRAW(tr);
	}

	m_d64_protectOff=0;
	return S_OK;
}

HRESULT GCRDISK::LoadFDIFromFileHandle(HANDLE hfile, TCHAR *filename)
{
HRESULT hr;
DWORD r,dw,filePointer;
struct FDIHeader fdiHeader;
bit8 tr,ftr;
DWORD file_size;
//struct FDITrackDescription fdiTrackDescription[G64_MAX_TRACKS];
struct FDITrackDescription *fdiTrackDescription = &fdiHeader.trackDescription[0];

	ClearError();
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}

	hr = ReadFromFile(hfile, filename, &fdiHeader.signature[0], sizeof(struct FDIHeader), 0);
	if (FAILED(hr))
		return hr;
	if (_memicmp(fdiHeader.signature,"Formatted Disk Image file\r\n", 27)!=0)
	{
		return SetError(E_FAIL,TEXT("%s is not a valid FDI file."), filename);
	}
	fdiHeader.ltrack = wordswap(fdiHeader.ltrack);

	if (fdiHeader.type != 1)
	{//5.25 inch check
		return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);
	}

	if (fdiHeader.lhead!=0)
	{
		return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);
	}

	if (fdiHeader.tpi == 0)
	{

		if ((fdiHeader.tpi == 0) && ((fdiHeader.ltrack+1) * 2 > G64_MAX_TRACKS))
			fdiHeader.ltrack = G64_MAX_TRACKS/2 - 1;
		m_d64TrackCount = fdiHeader.ltrack+1;
	}
	else if (fdiHeader.tpi == 2)
	{
		if ((fdiHeader.tpi == 2) && ((fdiHeader.ltrack+1) > G64_MAX_TRACKS))
			fdiHeader.ltrack = G64_MAX_TRACKS - 1;
		m_d64TrackCount = (fdiHeader.ltrack+1) / 2;
	}
	else
		return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);

	r = SetFilePointer (hfile, 152, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}

	for (tr=0 ; tr<G64_MAX_TRACKS ; tr++)
		ZeroMemory(m_rawTrackData[tr], DISK_RAW_TRACK_SIZE);

	ftr=0;
	tr=0;
	while (ftr<=fdiHeader.ltrack)
	{
		if (IsEventQuitSignalled())
			return E_FAIL;
		hr = ReadFromFile(hfile, filename, (char *)&fdiTrackDescription[ftr], sizeof(struct FDITrackDescription), 0);
		if (FAILED(hr))
			return hr;
		ftr++;
	}

	ftr=0;
	tr=0;
	filePointer = 0x200;
	while (ftr<=fdiHeader.ltrack)
	{
		if (IsEventQuitSignalled())
			return E_FAIL;
		if (fdiTrackDescription[ftr].type == 0)
			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		else if (fdiTrackDescription[ftr].type >= 0x80 && fdiTrackDescription[ftr].type <= 0xbf)
		{//pulses-index streams
			hr = FDIReadTrackStream(hfile, filePointer, tr);
			if (FAILED(hr))
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			dw = fdiTrackDescription[ftr].size;
			dw = dw | ((DWORD)(fdiTrackDescription[ftr].type & 0x3f)<<8);
			dw *= 0x100;
		}
		else if ((fdiTrackDescription[ftr].type & 0xf0) == 0xd0)
		{
			hr = FDIReadRawGCR(hfile, filePointer, tr, fdiTrackDescription[ftr].type);
			if (FAILED(hr))
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		}
		else if ((fdiTrackDescription[ftr].type & 0xf0) == 0xc0)
		{
			hr = FDIReadDecodedGCR(hfile, filePointer, tr, fdiTrackDescription[ftr].type);
			if (FAILED(hr))
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		}
		else
			return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);

		filePointer += dw;
		if (fdiHeader.tpi == 0)
			tr+=2;
		else
			tr+=1;

		ftr++;
	}

	if (fdiHeader.flags & 1)
		m_d64_protectOff=0;
	else
		m_d64_protectOff=1;


	if (IsEventQuitSignalled())
		return E_FAIL;
	if (wordswap(fdiHeader.version) >= 0x0201)
		return FDICheckCRC(hfile, filename, file_size);
	return S_OK;
}

HRESULT GCRDISK::FDICheckCRC(HANDLE hfile, TCHAR *filename, DWORD file_size)
{

HRESULT hr;
DWORD r;
long i;
bit8 currentByte;
CRC32Alloc crc;
//#ifdef DEBUG
//TCHAR szBuff[502];
//#endif
bit32 vHeader=0;
bit32 vData=0;
struct FDIHeader fdiHeader;
TCHAR sTest[] = TEXT("123456789");
bit32 vTest=0;
bit32 vTestCheck=0xCBF43926;

	if (!crc.isOK)
		return E_OUTOFMEMORY;

	crc.pCRC32->Init(CRC32POLY,0xffffffff,0xffffffff, true);

	r = SetFilePointer (hfile, 0L, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	hr = ReadFromFile(hfile, filename, (char *)&fdiHeader, sizeof(struct FDIHeader), 0);
	if (FAILED(hr))
		return hr;

	r = SetFilePointer (hfile, 0x200, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	for (i=0x200 ; (DWORD)i < file_size ; i++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&currentByte, 1, 0);
		if (FAILED(hr))
			return hr;
		crc.pCRC32->ProcessByte((BYTE)currentByte);
	}
	vData =crc.pCRC32->Value();


	crc.pCRC32->Init();
	for (i=0 ; i < lstrlen(sTest) ; i++)
	{
		crc.pCRC32->ProcessByte((BYTE)sTest[i]);
	}
	vTest = crc.pCRC32->Value();

	crc.pCRC32->Init();
	for (i=0 ; i < 508 ; i++)
	{
		crc.pCRC32->ProcessByte((BYTE)  ((BYTE *)&fdiHeader.signature[0])[i]);
	}
	vHeader = crc.pCRC32->Value();

	//#ifdef DEBUG
	//_sntprintf(szBuff, sizeof(szBuff) / sizeof(TCHAR), "Header CRC-32:\t\t %X\r\nCalc Header CRC-32:\t %X\r\nTrack CRC-32:\t\t %X\r\nCalc Track CRC-32:\t\t %X\r\nTest Check:\t\t%X\r\nTest:\t\t\t%X",
	//	(int)dwordswap(fdiHeader.headerCRC),
	//	(int)vHeader, 
	//	(int)dwordswap(fdiHeader.dataCRC),
	//	(int)vData,
	//	(int)vTestCheck,
	//	(int)vTest
	//);
	//MessageBox(0L, szBuff, TEXT("CRC-32 Check"), MB_OK | MB_ICONEXCLAMATION);
	//#endif
	if (dwordswap(fdiHeader.headerCRC)!=vHeader || dwordswap(fdiHeader.dataCRC)!=vData)
	{
		return SetError(APPERR_BAD_CRC, TEXT("The FDI disk file has successfully loaded but the CRC-32 check has failed. This file may be corrupt."));
	}
	return S_OK;
}

bit32 FDI3To4(bit32 v)
{
bit8 *p,t;
bit32 x;
	
	x = v;
	p = (bit8 *)&x;
	t = p[2];
	p[3] = 0;
	p[2] = p[0];
	p[0] = t;
	return x;
}

bit8 GCRDISK::FDIDecodeBitRate(bit8 fdiTrackType)
{
	switch (fdiTrackType & 0xf)
	{
	case 2:
		return 0;
		break;
	case 9:
		return 1;
		break;
	case 10:
		return 2;
		break;
	case 11:
		return 3;
		break;
	default:
		return 0xff;
	}
}

void GCRDISK::G64SetTrackSpeedZone(bit8 trackNumber, bit8 speed)
{
bit8 speedG64;

	speed &= 3;
	speedG64 = speed;
	speedG64 |= (speed<<2);
	speedG64 |= (speed<<4);
	speedG64 |= (speed<<6);
	FillMemory(&speedZone[trackNumber][0], G64_MAX_BYTES_PER_TRACK/4, speedG64);
}


HRESULT GCRDISK::FDIReadRawGCR(HANDLE hfile, DWORD filePointer, bit8 trackNumber, bit8 fdiTrackType)
{
HRESULT hr;
bit32 trackBitLength;
bit32 trackByteLength;
bit32 indexPos;
bit8 speed;
DWORD r;

	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&trackBitLength, 4, 0);
	if (FAILED(hr))
		return hr;

	if (trackBitLength > G64_MAX_BYTES_PER_TRACK*8)
		return E_FAIL;

	hr = ReadFromFileQ(hfile, (char *)&indexPos, 4, 0);
	if (FAILED(hr))
		return hr;

	
	trackByteLength = (trackBitLength + 7)/8;

	if (trackByteLength > G64_MAX_BYTES_PER_TRACK && trackByteLength < 6000)
		return E_FAIL;

	
	hr = ReadFromFileQ(hfile, (char *)&trackData[trackNumber][0], trackByteLength, 0);
	if (FAILED(hr))
		return hr;
	trackSize[trackNumber] = trackBitLength;
	speed = FDIDecodeBitRate(fdiTrackType);
	if (speed == 0xff)
		return E_FAIL;
	G64SetTrackSpeedZone(trackNumber, speed);

	ConvertGCRtoRAW(trackNumber);

	return S_OK;
}

void GCRDISK::WriteGCRBit(bit8 trackNumber, bit32 index, bit8 v)
{
bit8 i,m;

	index = index % (G64_MAX_BYTES_PER_TRACK*8);
	i = trackData[trackNumber][index/8];
	m = 1 << (7-(index & 7));
	v = v << (7-(index & 7));
	trackData[trackNumber][index/8] = (i & ~m) | v;
}

void GCRDISK::WriteGCRByte(bit8 trackNumber, bit32 index, bit8 v)
{
int j;

	for (j=7; j>=0; j--)
	{
		WriteGCRBit(trackNumber, index+j, (v & 0x1));
		v>>=1;
	}
}

HRESULT GCRDISK::FDIReadDecodedGCR(HANDLE hfile, DWORD filePointer, bit8 trackNumber, bit8 fdiTrackType)
{
bit8 speed;
DWORD r;
bit8 encoding;
bit32 indexPos,c,i,size;
HRESULT hr;
bit8 token;
bit16 wordData;
bit8 byteData;
bit8 buffer4[4];
bit8 b1,b2,b3,b4,b5;

	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&encoding, 1, 0);
	if (FAILED(hr))
		return hr;
	if (encoding!=1)//Commodore GCR
		return hr;
	hr = ReadFromFileQ(hfile, (char *)&indexPos, 3, 0);
	if (FAILED(hr))
		return hr;

	c=0;
	i=0;
	bool done=false;
	while (!done)
	{
		if (i > G64_MAX_BYTES_PER_TRACK*8)
			return E_FAIL;

		hr = ReadFromFileQ(hfile, (char *)&token, 1, 0);
		if (FAILED(hr))
			return hr;
		switch(token)
		{
		case 0x2://CBM sync mark {word size in bits}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 2, 0);
			if (FAILED(hr))
				return hr;
			wordData = wordswap(wordData);
			for (c=0 ; c < wordData; c++)
			{
				WriteGCRBit(trackNumber, i, 1);
				i++;
			}
			break;
		case 0x8://RLE GCR encoded {byte size in bytes}, {1 data byte}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 2, 0);
			if (FAILED(hr))
				return hr;
			byteData = wordData >> 8;
			size = wordData & 0xff;
			for (c=0 ; c < size ; c++)
			{
				WriteGCRByte(trackNumber, i, byteData);
				i+=8;
			}

			break;
		case 0x9://CBM RLE GCR decoded {byte size in bytes}, {1 data byte}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 1, 0);
			if (FAILED(hr))
				return hr;
			byteData = wordData >> 8;
			size = wordData & 0xff;
			D64_Binary_to_GCR(&byteData, &buffer4[0], 1);
			b1 = (buffer4[1] >> 7) & 1;
			b2 = (buffer4[1] >> 6) & 1;
			for (c=0 ; c < size ; c++)
			{
				WriteGCRByte(trackNumber, i, buffer4[0]);
				i+=8;
				WriteGCRBit(trackNumber, i, b1);
				i++;
				WriteGCRBit(trackNumber, i, b2);
				i++;
			}

			break;
		case 0xa://GCR encoded {word size in bits}, {ceil(size in bits/8 data bytes)}
		case 0xb://GCR encoded {word size in bits-65536}, {ceil(size in bits/8) data bytes}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 1, 0);
			if (FAILED(hr))
				return hr;
			wordData = wordswap(wordData);
			size = wordData/8;
			if (token=0xb)
				size+=0x2000;
			for (c=0 ; c < size ; c++)
			{
				hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
				if (FAILED(hr))
					return hr;
				WriteGCRByte(trackNumber, i, byteData);
				i+=8;
			}
			if (wordData & 7)
			{
				hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
				if (FAILED(hr))
					return hr;
				WriteGCRByte(trackNumber, i, byteData);
				i+=(wordData & 7);
			}
			break;
		case 0xc://CBM GCR decoded {word size in nibbles}, {ceil(size in bits/8 data bytes)}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 1, 0);
			if (FAILED(hr))
				return hr;
			size = wordData;
			for (c=0 ; c < size ; c++)
			{
				if ((c & 1) == 0)
				{
					hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
					if (FAILED(hr))
						return hr;
					D64_Binary_to_GCR(&byteData, &buffer4[0], 1);
					b1 = (buffer4[1] >> 7) & 1;
					b2 = (buffer4[1] >> 6) & 1;
				}
				if (c+1 < size)
				{
					WriteGCRByte(trackNumber, i, buffer4[0]);
					i+=8;
					WriteGCRBit(trackNumber, i, b1);
					i++;
					WriteGCRBit(trackNumber, i, b2);
					i++;
					c++;
				}
				else
				{
					b1 = (buffer4[0] >> 7) & 1;
					b2 = (buffer4[0] >> 6) & 1;
					b3 = (buffer4[0] >> 5) & 1;
					b4 = (buffer4[0] >> 4) & 1;
					b5 = (buffer4[0] >> 3) & 1;
					WriteGCRBit(trackNumber, i, b1);
					i++;
					WriteGCRBit(trackNumber, i, b2);
					i++;
					WriteGCRBit(trackNumber, i, b3);
					i++;
					WriteGCRBit(trackNumber, i, b4);
					i++;
					WriteGCRBit(trackNumber, i, b5);
					i++;
				}
			}
			break;
		case 0xff://end of buffer
			done = true;
			break;
		}
	}
	
	speed = FDIDecodeBitRate(fdiTrackType);
	if (speed == 0xff)
		return E_FAIL;
	G64SetTrackSpeedZone(trackNumber, speed);

	ConvertGCRtoRAW(trackNumber);
	return S_OK;
}

HRESULT GCRDISK::FDIReadTrackStream(HANDLE hfile, DWORD filePointer, bit8 trackNumber)
{
HRESULT hr;
DWORD r,d,i;
FDIStreamsHeader fdiStreamsHeader;
 
	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&d, 4, 0);
	if (FAILED(hr))
		return hr;

	fdiStreamsHeader.numPulses = dwordswap(d);

	d = 0;
	hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
	if (FAILED(hr))
		return hr;
	d = FDI3To4(d);
	fdiStreamsHeader.aveCompression = (d >> 22) & 3;
	fdiStreamsHeader.aveSize = d & 0x3fffff;

	d = 0;
	hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
	if (FAILED(hr))
		return hr;
	d = FDI3To4(d);
	fdiStreamsHeader.minCompression = (d >> 22) & 3;
	fdiStreamsHeader.minSize = d & 0x3fffff;

	d = 0;
	hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
	if (FAILED(hr))
		return hr;
	d = FDI3To4(d);
	fdiStreamsHeader.maxCompression = (d >> 22) & 3;
	fdiStreamsHeader.maxSize = d & 0x3fffff;

	d = 0;
	hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
	if (FAILED(hr))
		return hr;
	d = FDI3To4(d);
	fdiStreamsHeader.idxCompression = (d >> 22) & 3;
	fdiStreamsHeader.idxSize = d & 0x3fffff;

	if (fdiStreamsHeader.aveSize==0)
		return E_FAIL;

	//Ave
	if (fdiStreamsHeader.aveCompression==0)
		hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.aveData);
	else if (fdiStreamsHeader.aveCompression==1)
		hr = FDIDecompress(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.aveData);
	else
		hr = E_FAIL;
	if (FAILED(hr))
		return hr;

	//Check the Minimum stream. Not used by Hoxs64.
	if (fdiStreamsHeader.minSize)
	{
		//Min
		i = filePointer + 16 + fdiStreamsHeader.aveSize;
		r = SetFilePointer (hfile, i, 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return E_FAIL;
		}
		if (fdiStreamsHeader.minCompression==0)
			hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.minData);
		else if (fdiStreamsHeader.minCompression==1)
			hr = FDIDecompress(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.minData);
		else
			hr = E_FAIL;
		if (FAILED(hr))
			return hr;

		//Check the Maximum stream. Not used by Hoxs64.
		if (fdiStreamsHeader.maxSize)
		{
			//Max
			i = filePointer + 16 + fdiStreamsHeader.aveSize + fdiStreamsHeader.minSize;
			r = SetFilePointer (hfile, i, 0L, FILE_BEGIN);
			if (r == INVALID_SET_FILE_POINTER)
			{
				return E_FAIL;
			}
			if (fdiStreamsHeader.maxCompression==0)
				hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.maxData);
			else if (fdiStreamsHeader.maxCompression==1)
				hr = FDIDecompress(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.maxData);
			else
				hr = E_FAIL;
			if (FAILED(hr))
				return hr;
		}
	}

	//Check the Index stream. We use the Index stream to determine if a pulse is weak or strong. Pulse times always index from the last strong pulse.
	unsigned int maxPulseStrength = 0;
	if (fdiStreamsHeader.idxSize)
	{
		//Index
		i = filePointer + 16 + fdiStreamsHeader.aveSize + fdiStreamsHeader.minSize + fdiStreamsHeader.maxSize;
		r = SetFilePointer (hfile, i, 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return E_FAIL;
		}
		if (fdiStreamsHeader.idxCompression==0)
			hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.idxData);
		else if (fdiStreamsHeader.idxCompression==1)
			hr = FDIDecompress(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.idxData);
		else
			hr = E_FAIL;
		if (FAILED(hr))
			return hr;

		//Find the index value that is used to indicate a string pulse;
		for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
		{
			unsigned int index1state = (fdiStreamsHeader.idxData[i] >> 8) & 0xff;
			unsigned int index0state = (fdiStreamsHeader.idxData[i]) & 0xff;
			unsigned int pulseStrength = index1state + index0state;
			if (pulseStrength > maxPulseStrength)
				maxPulseStrength = pulseStrength;
		}
	}

	unsigned int pulseStrength=0;
	bool bIsStrongPulse;
	bool bIsNoticablePulse;
	unsigned __int64 totalTime;//Total FDI track length.
	unsigned __int64 pulseTime;//FDI distance from last strong pulse.
	unsigned __int64 sumPulseTime;//FDI distance of current pulse from start of track.
	unsigned __int64 sumStrongPulseTime;//FDI distance of last strong pulse from start of track.
	double clockTime;
	double totalClockTime;
	bit8 delayIndex;

	totalTime=0;
	//Loop through the aveData to total up the track length.
	for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
	{
		if (IsEventQuitSignalled())
			return E_FAIL;
		//Determine if this pulse is strong.
		if (fdiStreamsHeader.idxSize)
		{
			//idxData is used describe the strength of a pulse.
			unsigned int index1state = (fdiStreamsHeader.idxData[i] >> 8) & 0xff;
			unsigned int index0state = (fdiStreamsHeader.idxData[i]) & 0xff;
			pulseStrength = index1state + index0state;
			bIsStrongPulse = (pulseStrength == maxPulseStrength);
			bIsNoticablePulse = (pulseStrength >= maxPulseStrength/2);
		}
		else
		{
			//Assume all pulses are strong if the idxSize is not present.
			bIsStrongPulse = true;
			bIsNoticablePulse = true;
		}
		//Only strong pulse times are used to sum the total track time.
		bool bIsLastPulse = (i == fdiStreamsHeader.numPulses - 1);
		if (bIsStrongPulse)
			totalTime += fdiStreamsHeader.aveData[i];
	}
	
	totalClockTime = (double)(DISK_RAW_TRACK_SIZE*16);

	pulseTime=0;
	sumPulseTime=0;
	sumStrongPulseTime = 0;
	bit32 trackIndex;
	//Loop through aveData a second time to write the pulses to the emulated disk via PutDisk16
	for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
	{
		if (IsEventQuitSignalled())
			return E_FAIL;

		//Determine if this pulse is strong.
		if (fdiStreamsHeader.idxSize)
		{
			//idxData is used describe the strength of a pulse.
			unsigned int index1state = (fdiStreamsHeader.idxData[i] >> 8) & 0xff;
			unsigned int index0state = (fdiStreamsHeader.idxData[i]) & 0xff;
			pulseStrength = index1state + index0state;
			bIsStrongPulse = (pulseStrength == maxPulseStrength);
			bIsNoticablePulse = (pulseStrength >= maxPulseStrength/4);
		}
		else
		{
			//Assume all pulses are strong if the idxSize is not present.
			bIsStrongPulse = true;
			bIsNoticablePulse = true;
		}

		//pulseTime should be greater than zero.
		pulseTime = (unsigned __int64)fdiStreamsHeader.aveData[i];
		//Skip zero pulse times.
		if (pulseTime == 0)
			continue;
		if (pulseTime < 0)
			return E_FAIL;

		if (bIsStrongPulse)
		{
			//Count strong pulse times as part of the total track time.
			sumStrongPulseTime += pulseTime;
			sumPulseTime = sumStrongPulseTime;
		}
		else
		{
			//The weak pulse is indexed from the last strong pulse.
			//We are not going to write out this weak pulse. We take note only for debug purposes.
			sumPulseTime = sumStrongPulseTime + pulseTime;
		}

		//Do not write out weak pulses to the emulated disk.
		if (!bIsStrongPulse)
			continue;

		//DISK_RAW_TRACK_SIZE is 200000. This is the number of cells or arc's of track in an emulated track.
		//Take the current fdi distance from start of track (sumPulseTime) and re-scale it into our emulated track at arc trackIndex [0-199999] with the pulse position with in the that arc to be stored in delayIndex [1-16]
		clockTime = (double)sumPulseTime / (double)totalTime * (double)(DISK_RAW_TRACK_SIZE * 16);
		trackIndex = ((bit32)floor(clockTime)) / 16L;
		trackIndex = trackIndex % DISK_RAW_TRACK_SIZE;
		delayIndex = (bit8)(((bit32)floor(clockTime)) & 0xf) + 1;

		assert(trackIndex < DISK_RAW_TRACK_SIZE);

		//Write the pulse to the emulated disk.
		PutDisk16(trackNumber, trackIndex, delayIndex);
	}

	return S_OK;
}

HRESULT GCRDISK::FDICopyTrackStream(HANDLE hfile, DWORD pulseCount, DWORD **data)
{
FDIStream fdiStream;
HRESULT hr;
DWORD i;
bit32 *p;

	fdiStream.data = (bit8 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, pulseCount * 4);
	if (!fdiStream.data)
		return E_FAIL;

	hr = ReadFromFileQ(hfile, (char *)&fdiStream.data, pulseCount * 4, 0);
	if (FAILED(hr))
		return hr;

	p = (bit32 *)fdiStream.data;
	for (i=0 ; i<pulseCount ; i++)
	{
		p[i] = dwordswap(p[i]);
		i++;
	}

	*data = p;
	fdiStream.data=0;
	return S_OK;
}


HRESULT GCRDISK::FDIDecompress(HANDLE hfile, DWORD pulseCount, DWORD **data)
{
HuffNode *rootNode;
bit8 subStreamHeader1;
bit8 subStreamHeader2;
FDIStream fdiStream;
char currentByte;
short currentWord;
bit8 frame;
HuffNode *currentNode,*hn;
HRESULT hr;
DWORD i,v;
DWORD mask;
CHuffNodeArray nodeArray;
HuffNodeHolder nodeHolder;
int cnt = 0;

	fdiStream.data = (bit8 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, pulseCount * 4);
	if (!fdiStream.data)
		return E_FAIL;

	do
	{
		cnt++;
		//
		nodeArray.Clear();
		hr = nodeArray.Resize(256);
		if (FAILED(hr))
			return hr;
		hr = nodeHolder.Init(0x20000);
		if (FAILED(hr))
			return hr;

		rootNode = nodeHolder.Create();
		if (rootNode==NULL)
			return E_OUTOFMEMORY;
		currentNode = rootNode;
		//


		hr = ReadFromFileQ(hfile, (char *)&subStreamHeader1, 1, 0);
		if (FAILED(hr))
			return hr;

		hr = ReadFromFileQ(hfile, (char *)&subStreamHeader2, 1, 0);
		if (FAILED(hr))
			return hr;

		fdiStream.highBitNumber = subStreamHeader2 & 0x7f;
		fdiStream.lowBitNumber = subStreamHeader1 & 0x7f;

		if (subStreamHeader1 & 0x80)
			fdiStream.bSignExtend = true;
		else
			fdiStream.bSignExtend = false;


		if (subStreamHeader2 & 0x80)
			fdiStream.bitSize = 16;
		else
			fdiStream.bitSize = 8;

		mask = ((DWORD)-1) >> (31 - fdiStream.highBitNumber);
		mask &=  ((DWORD)-1) << (fdiStream.lowBitNumber);

		//Build Huffman tree;
		frame = 0;
		i=0;
		while(1)
		{
			if (frame == 0)
			{
				frame = 8;
				hr = ReadFromFileQ(hfile, (char *)&currentByte, 1, 0);
				if (FAILED(hr))
					return hr;
			}
			if (currentByte >= 0)
			{
				//high bit is a 0
				//currentNode is a "node"
				currentNode->AddLeft(hn = nodeHolder.Create());
				if (hn == NULL)
					return E_OUTOFMEMORY;
				currentNode->AddRight(hn = nodeHolder.Create());
				if (hn == NULL)
					return E_OUTOFMEMORY;
				currentNode = currentNode->leftNode;
			}
			else
			{
				//high bit is a 1
				//currentNode is a "leaf"
				currentNode->isLeaf = true;
				hr = nodeArray.Append(currentNode);
				if (FAILED(hr))
					return hr;
				currentNode = currentNode->FindDeepestRightNode();
				if (currentNode==0)
					break;
			}
			i++;
			if (i>0x20000)//tree limit
				return E_FAIL;

			currentByte <<= 1;
			frame--;
		}

		//Read tree values
		for (i = 0 ; i < nodeArray.Count(); i++)
		{
			if (fdiStream.bitSize == 16)
			{
				hr = ReadFromFileQ(hfile, (char *)&currentWord, 2, 0);
				if (FAILED(hr))
					return hr;

				//Is this right?
				//Not really a correct decompression but it saves on another loop to
				//swap endianess of the decompressed data.
				currentWord = wordswap(currentWord);

				if (fdiStream.bSignExtend)
					nodeArray[i]->value = (unsigned long)(signed short)currentWord;
				else
					nodeArray[i]->value = (unsigned long)(unsigned short)currentWord;

			}
			else
			{
				hr = ReadFromFileQ(hfile, (char *)&currentByte, 1, 0);
				if (FAILED(hr))
					return hr;

				if (fdiStream.bSignExtend)
					nodeArray[i]->value = (unsigned long)(signed char)currentByte;
				else
					nodeArray[i]->value = (unsigned long)(unsigned char)currentByte;
			}
		}


		//decode stream
		i=0;
		frame=0;
		for (i = 0 ; i < pulseCount ; i++)
		{
			currentNode = rootNode;
			while (!currentNode->isLeaf)
			{
				if (frame == 0)
				{
					frame = 8;
					hr = ReadFromFileQ(hfile, (char *)&currentByte, 1, 0);
					if (FAILED(hr))
						return hr;
				}
				if (currentByte >= 0)
				{
					if (currentNode->leftNode==0)
						return E_FAIL;
					currentNode = currentNode->leftNode;
				}
				else
				{
					if (currentNode->rightNode==0)
						return E_FAIL;
					currentNode = currentNode->rightNode;
				}

				currentByte <<= 1;
				frame--;
			}

			v = currentNode->value;
			v = v << fdiStream.lowBitNumber;
			v = v & mask;
			bit32 x = ((bit32 *)fdiStream.data)[i];
			x = (x & ~mask) | v;
			((bit32 *)fdiStream.data)[i] = x;
		}
	} while (fdiStream.lowBitNumber != 0);

	*data = (DWORD *)fdiStream.data;
	fdiStream.data = 0;
	return S_OK;

}

HRESULT GCRDISK::LoadFDIFromFile(TCHAR *filename)
{
HANDLE hfile=0;
HRESULT hr;
//bit8 tr;

	hfile=0;
	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}

	hr= LoadFDIFromFileHandle(hfile, filename);
	if (FAILED(hr))
	{
		CloseHandle(hfile);
		return hr;
	}

	CloseHandle(hfile);

	return hr;
}

HRESULT GCRDISK::LoadG64FromFile(TCHAR *filename, bool bConvertToRAW)
{
HANDLE hfile=0;
HRESULT hr;

	ClearError();
	hfile=0;
	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}

	hr= LoadG64FromFileHandle(hfile, filename, bConvertToRAW);
	if (FAILED(hr))
	{
		CloseHandle(hfile);
		return hr;
	}

	CloseHandle(hfile);

	return S_OK;
}

HRESULT GCRDISK::LoadD64FromFile(TCHAR *filename, bool bConvertToRAW, bool bAlignD64Tracks)
{
HANDLE hfile=0;
BOOL r;
DWORD bytes_read,file_size;

	ClearError();

	hfile=0;
	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}

	switch (file_size)
	{
	case 174848://35 track, no errors
		m_d64TrackCount=35;
		d64Errors=0;
		break;
	case 175531://35 track, 683 error bytes
		m_d64TrackCount=35;
		d64Errors=683;
		break;
	case 196608://40 track, no errors
		m_d64TrackCount=40;
		d64Errors=0;
		break;
	case 197376://40 track, 768 error bytes
		m_d64TrackCount=40;
		d64Errors=768;
		break;
	default:
		CloseHandle(hfile);
		return SetError(E_FAIL,TEXT("Invalid D64 file length."));
	}

	r=ReadFile(hfile, m_pD64Binary, file_size, &bytes_read, NULL);
	CloseHandle(hfile);
	if (r==0)
	{
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
	}
	if (bytes_read!=file_size)
	{
		return SetError(E_FAIL,TEXT("Could not read from %s."),filename);
	}

	if (bConvertToRAW)
		MakeGCRImage(m_pD64Binary, m_d64TrackCount, d64Errors, bAlignD64Tracks);

	m_d64_protectOff=0;
	return S_OK;
}

#define SOFFSET(t, m)	(INT_PTR)(&(((t *)(0))->m))

HRESULT GCRDISK::SaveFDIToFile(TCHAR *filename)
{
BOOL rb;
FDIHeader fdiHeader;
HANDLE hfile;
DWORD bytes_written,file_size,r;
struct FDITrackDescription fdiTrackDescription[G64_MAX_TRACKS];
long i,j;
bit8 tr;
HRESULT hr;
struct FDIRawTrackHeader fdiRawTrackHeader;
bit32 pulseCount;
FDIData buffer;
bit32 *p,delay;
HuffWork hw;
bit32 nextTrackWrite;
bit32 compressedBufferSize;
bit32 writeSize;
long zeroPadSize;
bit8 currentByte;
CRC32Alloc crc;

	ClearError();
	if (sizeof(FDIHeader) != 0x200)
		return SetError(E_FAIL,TEXT("Internal error. FDIHeader size was found not to be 0x200."));

	if (!crc.isOK)
		return E_OUTOFMEMORY;
	crc.pCRC32->Init(CRC32POLY,0xffffffff,0xffffffff, true);

	hfile=CreateFile(filename, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL ,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not save %s."),filename);
	}
	hw.HuffSetFile(hfile);

	memset(&fdiHeader.signature[0], 0, sizeof(FDIHeader));
	memcpy(fdiHeader.signature, "Formatted Disk Image file\r\n", 27);
	memset(fdiHeader.creator, 0x20, sizeof(fdiHeader.creator));
	memcpy(fdiHeader.creator, "Hoxs64 1541 FDI save",20);
	fdiHeader.cr[0] = '\r';
	fdiHeader.cr[1] = '\n';
	memset(fdiHeader.comment, 0x1a, sizeof(fdiHeader.comment));
	fdiHeader.eof = 0x1a;
	fdiHeader.version = wordswap(0x0201);
	fdiHeader.ltrack = wordswap(G64_MAX_TRACKS-1);
	fdiHeader.lhead = 0;
	fdiHeader.type = 1;
	fdiHeader.rotspeed = 0xac;
	fdiHeader.flags = (m_d64_protectOff==0);
	fdiHeader.tpi = 2;
	fdiHeader.headwidth = 2;
	fdiHeader.reserved = 0;
	
	rb = WriteFile(hfile, &fdiHeader, sizeof(FDIHeader), &bytes_written, NULL);
	if (!rb)
		return SetError(E_FAIL,TEXT("Could not save %s."),filename);

	hr = hw.Init();
	if(FAILED(hr))
		return SetError(hr, TEXT("Could initialise Huffmann compression."));


	buffer.data =(bit8 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, DISK_RAW_TRACK_SIZE*sizeof(bit32));
	p = (bit32 *) buffer.data;
	if (buffer.data == NULL)
		return SetError(E_OUTOFMEMORY,TEXT("Out of memory."));

	ZeroMemory(fdiTrackDescription, sizeof(FDITrackDescription) * G64_MAX_TRACKS);
	nextTrackWrite = 0x200;
	for (tr=0; tr<G64_MAX_TRACKS; tr++)
	{
		delay=0;
		pulseCount=0;
		for (i = DISK_RAW_TRACK_SIZE-1 ; i>=0; i--)
		{
			if (m_rawTrackData[tr][i]!=0)
			{
				delay+= (16 - m_rawTrackData[tr][i]);
				break;
			}
			else
				delay+=16;
		}
		if (i<0)
			continue;

		for (i=0 ; i<DISK_RAW_TRACK_SIZE; i++)
		{
			if (m_rawTrackData[tr][i]!=0)
			{
				delay+=m_rawTrackData[tr][i];
				
				p[pulseCount] = delay;
				pulseCount++;
		
				delay = (16 - m_rawTrackData[tr][i]);
			}
			else
				delay+=16;
		}

		if (pulseCount>DISK_RAW_TRACK_SIZE)
			return SetError(E_FAIL,TEXT("Too many pulses found in track %d."),(int)tr);

		
		r = SetFilePointer (hfile, nextTrackWrite + sizeof(FDIRawTrackHeader), 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
		}

		hr = hw.HuffCompress(p, pulseCount, &compressedBufferSize);
		if (FAILED(hr))
		{
			if (hr == E_OUTOFMEMORY)
				SetError(hr, TEXT("Out of memory"));
			return SetError(hr, TEXT("Could not compress track %d"), (long)tr);
		}
		if (compressedBufferSize>0x3ffff)
			return SetError(E_FAIL,TEXT("Compression failed for track %d."),(int)tr);

		writeSize = (sizeof(FDIRawTrackHeader) + compressedBufferSize);
		if ((writeSize & 0xff)!=0)
		{
			j=0;
			zeroPadSize = 0x100 - (writeSize & 0xff);
			for (i=0; i < zeroPadSize; i++)
			{
				rb = WriteFile(hfile, &j, 1, &bytes_written, NULL);
				if (!rb)
					return SetError(E_FAIL, TEXT("Could not save %s."),filename);
			}
		}

		assert(compressedBufferSize<=0x3ffff);
		fdiRawTrackHeader.numPulses = dwordswap(pulseCount);
		fdiRawTrackHeader.aveSize[2] = (bit8)(compressedBufferSize);
		fdiRawTrackHeader.aveSize[1] = (bit8)(compressedBufferSize >> 8);
		fdiRawTrackHeader.aveSize[0] = (bit8)(compressedBufferSize >> 16);
		fdiRawTrackHeader.aveSize[0] = (fdiRawTrackHeader.aveSize[0] & 0x3f) | 0x40;

		fdiRawTrackHeader.minSize[2] = 0;
		fdiRawTrackHeader.minSize[1] = 0;
		fdiRawTrackHeader.minSize[0] = 0;
		
		fdiRawTrackHeader.maxSize[2] = 0;
		fdiRawTrackHeader.maxSize[1] = 0;
		fdiRawTrackHeader.maxSize[0] = 0;

		fdiRawTrackHeader.idxSize[2] = 0;
		fdiRawTrackHeader.idxSize[1] = 0;
		fdiRawTrackHeader.idxSize[0] = 0;

		r = SetFilePointer (hfile, nextTrackWrite, 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
		}
		rb = WriteFile(hfile, &fdiRawTrackHeader, sizeof(FDIRawTrackHeader), &bytes_written, NULL);
		if (!rb)
			return SetError(E_FAIL, TEXT("Could not save %s."),filename);


		writeSize = (writeSize + 0xff) / 0x100;

		assert(writeSize<=0x3fff);
		fdiTrackDescription[tr].type = ((bit8)(writeSize>>8) & 0x3f) | 0x80;
		fdiTrackDescription[tr].size = (bit8)writeSize & 0xff;

		nextTrackWrite+= (writeSize * 0x100);
	}

	r = SetFilePointer (hfile, SOFFSET(FDIHeader,trackDescription) , 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	rb = WriteFile(hfile, &fdiTrackDescription[0], sizeof(FDITrackDescription) * G64_MAX_TRACKS, &bytes_written, NULL);
	if (!rb)
		return SetError(E_FAIL, TEXT("Could not save %s."),filename);

	/*Calculate CRC-32 values*/
	/*Seek to begining*/
	r = SetFilePointer (hfile, 0L, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL,TEXT("GetFileSize failed for %s."),filename);
	}

	/*Read header*/
	hr = ReadFromFile(hfile, filename, (char *)&fdiHeader, sizeof(struct FDIHeader), 0);
	if (FAILED(hr))
		return hr;
	
	/*Read track data and calculate it's CRC-32*/
	for (i=sizeof(struct FDIHeader) ; (DWORD)i < file_size ; i++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&currentByte, 1, 0);
		if (FAILED(hr))
			return hr;
		crc.pCRC32->ProcessByte((BYTE)currentByte);
	}
	fdiHeader.dataCRC = dwordswap(crc.pCRC32->Value());

	
	/*calculate CRC-32 for the header*/
	crc.pCRC32->Init();
	for (i=0 ; i < 508 ; i++)
	{
		crc.pCRC32->ProcessByte((BYTE)  ((BYTE *)&fdiHeader)[i]);
	}
	fdiHeader.headerCRC = dwordswap(crc.pCRC32->Value());


	/*Seek to begining*/
	r = SetFilePointer (hfile, 0L , 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	/*rewrite the header*/
	rb = WriteFile(hfile, &fdiHeader, sizeof(FDIHeader), &bytes_written, NULL);
	if (!rb || bytes_written!=sizeof(FDIHeader))
		return SetError(E_FAIL, TEXT("Could not save %s."),filename);

	return S_OK;
}

//Not implemented
HRESULT GCRDISK::SaveG64ToFile(TCHAR *filename)
{
	return S_OK;
}

HRESULT GCRDISK::SaveD64ToFile(TCHAR *filename, int numberOfTracks)
{
HANDLE hfile=0;
long r;
BOOL rb;
DWORD bytes_written,file_size;
bit32 tracks;
bit8 tr;
HRESULT hr;

	ClearError();
	hr = S_OK;

	if (numberOfTracks == 40)
	{
		file_size = 196608;
		tracks = 40;
	}
	else if (numberOfTracks == 35)
	{
		file_size = 174848;
		tracks = 35;
	}
	else
	{
		file_size = 196608;
		tracks = 40;
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
		ConvertRAWtoGCR(tr);

	r = ConvertGCRToD64(tracks);
	if (FAILED(r))
	{
		hr = TRUE;
	}

	hfile=CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL ,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not save %s."),filename);
	}
	
	rb = WriteFile(hfile, m_pD64Binary, file_size, &bytes_written, NULL);
	CloseHandle(hfile);
	if (rb == FALSE)
	{
		return SetError(E_FAIL,TEXT("Could not write to %s."),filename);
	}
	if (bytes_written!=file_size)
	{
		return SetError(E_FAIL,TEXT("Could not write to %s."),filename);
	}
	return hr;
}

void GCRDISK::MakeNewD64Image(TCHAR *diskname, bit8 id1, bit8 id2)
{
char dname[16];
int i;
	memset(&dname[0], 160, 16);
	memset(m_pD64Binary, 0x0, MAX_D64_SIZE);
	CopyMemory(&m_pD64Binary[D64_info[17].file_offset], &D64_BAM[0], 256L);
	for (i=0 ; i<=15 ; i++)
	{
		if (diskname[i] !=0 && diskname[i] !=160)
			dname[i] = diskname[i] & 255;
		else
			break;
	}
	CopyMemory(&m_pD64Binary[D64_info[17].file_offset + 0x90], &dname[0], 16);
	m_pD64Binary[D64_info[17].file_offset + 0xA2] = id1;
	m_pD64Binary[D64_info[17].file_offset + 0xA3] = id2;
}

void GCRDISK::InsertNewDiskImage(TCHAR *diskname, bit8 id1, bit8 id2, bool bAlignD64Tracks, int numberOfTracks)
{
	m_d64_protectOff=0;
	m_d64TrackCount = numberOfTracks == 40 ? 40 : 35;
	MakeNewD64Image(diskname, id1, id2);
	MakeGCRImage(m_pD64Binary, m_d64TrackCount, 0, bAlignD64Tracks);
}


bit8 GCRDISK::GetByte(bit8 trackNumber,bit16 byteIndex,bit8 bitIndex)
{
long i,j;
char byte,v;
bit32 startBitPos;

	if (trackSize[trackNumber]==0)
		return 0;
	startBitPos = (byteIndex * 8 + bitIndex) % trackSize[trackNumber];

	byte=0;
	i = startBitPos;
	for (j=0 ; j < 8 ; j++)
	{
		byte<<=1;
		v = (trackData[trackNumber][i/8]) << (i & 7);
		if (v<0)
			byte|=1;
		i = (i + 1) % trackSize[trackNumber];
	}
	return byte;
}


bit8 GCRDISK::GetDisk16(bit8 trackNumber, bit32 headIndex)
{
	return m_rawTrackData[trackNumber][headIndex];
}

void GCRDISK::PutDisk16(bit8 trackNumber, bit32 headIndex, bit8 data)
{
		m_rawTrackData[trackNumber][headIndex] = data;
}

bool GCRDISK::IsEventQuitSignalled()
{
	if (mhevtQuit)
		return (WaitForSingleObject(mhevtQuit, 0) == WAIT_OBJECT_0);
	else
		return false;
}