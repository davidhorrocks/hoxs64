#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "boost2005.h"
#include <winuser.h>
#include <commctrl.h>
#include "bits.h"
#include "carray.h"
#include "mlist.h"
#include "errormsg.h"
#include "huff.h"
#include "FDI.h"
#include "crc.h"
#include "iquit.h"
#include "p64config.h"
#include "p64.h"
#include "d64.h"
#include "CDPI.h"
#include "utils.h"

#define DISKHEADFILTERWIDTH (40)
#define SHOWSKEW 0
#define SHOWGCR 0
#define SHOWGAPS 0
#define SHOWTRACKNUMBER 46
#define SHOWPASSNO 1

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
	/* 0 */{21,0x00000,D64_TRACK_SIZE_1_17,3,21,0.268956},
	/* 1 */{21,0x01500,D64_TRACK_SIZE_1_17,3,42,0.724382},
	/* 2 */{21,0x02a00,D64_TRACK_SIZE_1_17,3,63,0.177191},
	/* 3 */{21,0x03f00,D64_TRACK_SIZE_1_17,3,84,0.632698},
	/* 4 */{21,0x05400,D64_TRACK_SIZE_1_17,3,105,0.088173},
	/* 5 */{21,0x06900,D64_TRACK_SIZE_1_17,3,126,0.543583},
	/* 6 */{21,0x07e00,D64_TRACK_SIZE_1_17,3,147,0.996409},
	/* 7 */{21,0x09300,D64_TRACK_SIZE_1_17,3,168,0.451883},
	/* 8 */{21,0x0a800,D64_TRACK_SIZE_1_17,3,189,0.907342},
	/* 9 */{21,0x0bd00,D64_TRACK_SIZE_1_17,3,210,0.362768},
	/* 10 */{21,0x0d200,D64_TRACK_SIZE_1_17,3,231,0.815512},
	/* 11 */{21,0x0e700,D64_TRACK_SIZE_1_17,3,252,0.268338},
	/* 12 */{21,0x0fc00,D64_TRACK_SIZE_1_17,3,273,0.723813},
	/* 13 */{21,0x11100,D64_TRACK_SIZE_1_17,3,294,0.179288},
	/* 14 */{21,0x12600,D64_TRACK_SIZE_1_17,3,315,0.634779},
	/* 15 */{21,0x13b00,D64_TRACK_SIZE_1_17,3,336,0.090253},
	/* 16 */{21,0x15000,D64_TRACK_SIZE_1_17,3,357,0.545712},
	/* 17 */{19,0x16500,D64_TRACK_SIZE_18_24,2,376,0.945418},
	/* 18 */{19,0x17800,D64_TRACK_SIZE_18_24,2,395,0.506081},
	/* 19 */{19,0x18b00,D64_TRACK_SIZE_18_24,2,414,0.066622},
	/* 20 */{19,0x19e00,D64_TRACK_SIZE_18_24,2,433,0.627303},
	/* 21 */{19,0x1b100,D64_TRACK_SIZE_18_24,2,452,0.187862},
	/* 22 */{19,0x1c400,D64_TRACK_SIZE_18_24,2,471,0.748403},
	/* 23 */{19,0x1d700,D64_TRACK_SIZE_18_24,2,490,0.308962},
	/* 24 */{18,0x1ea00,D64_TRACK_SIZE_24_30,1,508,0.116926},
	/* 25 */{18,0x1fc00,D64_TRACK_SIZE_24_30,1,526,0.788086},
	/* 26 */{18,0x20e00,D64_TRACK_SIZE_24_30,1,544,0.459190},
	/* 27 */{18,0x22000,D64_TRACK_SIZE_24_30,1,562,0.130238},
	/* 28 */{18,0x23200,D64_TRACK_SIZE_24_30,1,580,0.801286},
	/* 29 */{18,0x24400,D64_TRACK_SIZE_24_30,1,598,0.472353},
	/* 30 */{17,0x25600,D64_TRACK_SIZE_31_35,0,615,0.834120},
	/* 31 */{17,0x26700,D64_TRACK_SIZE_31_35,0,632,0.614880},
	/* 32 */{17,0x27800,D64_TRACK_SIZE_31_35,0,649,0.395480},
	/* 33 */{17,0x28900,D64_TRACK_SIZE_31_35,0,666,0.176140},
	/* 34 */{17,0x29a00,D64_TRACK_SIZE_31_35,0,683,0.956800},
	/* 35 */{17,0x2ab00,D64_TRACK_SIZE_31_35,0,700,0.300},
	/* 36 */{17,0x2bc00,D64_TRACK_SIZE_31_35,0,717,0.820},
	/* 37 */{17,0x2cd00,D64_TRACK_SIZE_31_35,0,734,0.420},
	/* 38 */{17,0x2de00,D64_TRACK_SIZE_31_35,0,751,0.940},
	/* 39 */{17,0x2ef00,D64_TRACK_SIZE_31_35,0,768,0.540},
	/* 40 */{17,0x30000,D64_TRACK_SIZE_31_35,0,785,0.130},
	/* 41 */{17,0x31100,D64_TRACK_SIZE_31_35,0,802,0.830}
};

#define D64_GAPHEADER 9
#define D64_SYNCLENGTH 5
#define D64_GAPSECTOR_1_17 8
#define D64_GAPSECTOR_18_24 17
#define D64_GAPSECTOR_25_30 12
#define D64_GAPSECTOR_31_40 9
#define D64_SECTOR_HEADER_SIZE 9
#define D64_SECTOR_DATABLOCK_SIZE 256
#define D64_SECTOR_SIZE 260

struct sector_header
{
	bit8 header_id;
	bit8 check_eor; 
	bit8 sector;
	bit8 track;
	bit8 disk_id2;
	bit8 disk_id1;
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
GCRDISK Class
***********************************************************************************************************************/
GCRDISK::GCRDISK()
{
int i;
	for (i=0 ; i < HOST_MAX_TRACKS ; i++)
	{
		trackSize[i] = 0;
		trackData[i] = 0;
		speedZone[i] = 0;
		trackAllocatedMemorySize[i] = 0;
	}

	P64ImageCreate(&this->m_P64Image);
	d64Errors=0;
	m_pD64Binary=0;
	m_d64TrackCount=0;
	m_d64_protectOff=0;
	mhevtQuit = 0;
	linePrintCount = 0;
}

GCRDISK::~GCRDISK()
{
	Clean();
}

void GCRDISK::JumpBits(unsigned int trackNumber,unsigned int &bitIndex, unsigned int bitCount)
{
unsigned int startBitPos;
	if (trackSize[trackNumber]==0)
	{
		return;
	}

	startBitPos = bitIndex;
	bitIndex = (startBitPos + bitCount) % trackSize[trackNumber];
}

HRESULT GCRDISK::SeekSync(unsigned int trackNumber, unsigned int bitIndex, unsigned int bitScanLimit, bit8 *p_headerByte, unsigned int *p_newBitIndex, unsigned int* p_jumpedBitCount, unsigned int *p_sync_count)
{
unsigned int bitsScannedCounter;
unsigned int sync_count;
bit8 data;
bool bSyncFound;
	
	*p_jumpedBitCount = 0;
	*p_headerByte = 0;
	*p_sync_count = 0;
	bSyncFound=false;
	bitsScannedCounter=0;
	sync_count=0;

	while (bitsScannedCounter < bitScanLimit)
	{
		if (IsEventQuitSignalled())
		{
			return E_FAIL;
		}

		data = GetByte(trackNumber, bitIndex);
		if ((signed char)data < 0)
		{
			sync_count++;
			if (sync_count >= 10)
			{
				bSyncFound = true;
			}
		}
		else
		{
			if (bSyncFound)
			{
				*p_headerByte = data;
				*p_newBitIndex = bitIndex;
				*p_sync_count = sync_count;
				return S_OK;
			}

			sync_count = 0;
			bSyncFound = false;
		}			
		JumpBits(trackNumber, bitIndex, 1);
		bitsScannedCounter++;
		(*p_jumpedBitCount)++;
	}
	//No Sync with specified header byte found. 
	return S_FALSE;		
}


void GCRDISK::CopyRawData(bit8 *buffer, unsigned int trackNumber, unsigned int bitIndex, unsigned int count)
{
unsigned int i;
	for (i=0 ; i < count ; i++)
	{
		buffer[i] = GetByte(trackNumber, bitIndex);
		JumpBits(trackNumber, bitIndex, 8);
	}
}

HRESULT GCRDISK::ConvertGCRToD64(unsigned int tracks)
{
unsigned int tr;
long r;
HRESULT st;
struct sector_header sec_header;
struct sector_data sec_data;
unsigned int bitIndex;
HRESULT hr;
const bit32 MAXBYTESTOSCAN = G64_MAX_BYTES_PER_TRACK * 3;
const bit32 MAXBITSTOSCAN = MAXBYTESTOSCAN * 8L;
unsigned int bitCounter;
unsigned int jumpedBitCount;
unsigned int sync_count;
bit8 buffer[325];
unsigned int g64TrackNumber;
unsigned int bytesToCopy;
bit8 maxSectorsOnThisTrack;
bit8 headerByte;

#if defined(DEBUG) && (SHOWGAPS == 1 || SHOWSKEW == 1)
TCHAR textbuffer[50];
int datalength = 0;
#endif

	if (tracks > D64_MAX_TRACKS)
	{
		tracks = D64_MAX_TRACKS;
	}

	ClearD64LoadStatus();
	st = S_OK;
	memset(m_pD64Binary, 0x0, MAX_D64_SIZE);
	for(tr = 0; tr < tracks; tr++)
	{
		g64TrackNumber = tr * 2;
retryhalftrack:
		bitIndex = 0;
		bitCounter = 0;
		maxSectorsOnThisTrack = (bit8) D64_info[tr].sector_count;
		while (bitCounter < MAXBITSTOSCAN)
		{
			if (IsEventQuitSignalled())
			{
				return E_FAIL;
			}
			unsigned int nextBitIndex;
			hr = SeekSync(g64TrackNumber, bitIndex, MAXBITSTOSCAN - bitCounter, &headerByte, &nextBitIndex, &jumpedBitCount, &sync_count);
			if (FAILED(hr))
			{
				return hr;
			}

#if defined(DEBUG) && SHOWGAPS == 1
			if (SUCCEEDED(hr))
			{
				if (datalength > 0)
				{
					_stprintf_s(textbuffer, _countof(textbuffer), TEXT("tr=%d,%d,%d,%d\n"), tr, datalength, (jumpedBitCount - sync_count + 4) / 8 - datalength, (sync_count + 4) / 8);
					OutputDebugString(textbuffer);
				}
				datalength = headerByte == 0x52 ? 10 : 325;
			}
#endif
			bitIndex = nextBitIndex;
			bitCounter += jumpedBitCount;
			if (hr != S_OK)
			{
				continue;
			}

			if (headerByte != 0x52)
			{
				continue;
			}

headerfound:
			bytesToCopy = 10;
			CopyRawData(buffer, g64TrackNumber, bitIndex, bytesToCopy);
			bitCounter = bitCounter + bytesToCopy * 8;
			r = D64_GCR_to_Binary(buffer, (bit8 *) &sec_header, bytesToCopy * 8);
			if (r >= 0)
			{
				continue;//invalid GCR
			}
			
			if (sec_header.sector > maxSectorsOnThisTrack)
			{
				continue;
			}

			if (sec_header.track != (tr + 1))
			{
				continue;
			}

			if (get_D64LoadStatus(tr, sec_header.sector) == GCRDISK::OK)
			{
				continue;
			}

			if (sec_header.sector < _countof(d64LoadStatus[tr].d64SectorLoadStatus))
			{
				d64LoadStatus[tr].d64SectorLoadStatus[sec_header.sector].bitIndex = bitIndex;
			}

			if (bitCounter >= MAXBITSTOSCAN)
			{
				break;
			}
			
			hr = SeekSync(g64TrackNumber, bitIndex, MAXBITSTOSCAN - bitCounter, &headerByte, &nextBitIndex, &jumpedBitCount, &sync_count);
			if (FAILED(hr))
			{
				return hr;
			}

#if defined(DEBUG) && SHOWGAPS == 1
			if (SUCCEEDED(hr))
			{
				if (datalength > 0)
				{
					_stprintf_s(textbuffer, _countof(textbuffer), TEXT("tr=%d,%d,%d,%d\n"), tr, datalength, (jumpedBitCount - sync_count + 4) / 8 - datalength, (sync_count + 4) / 8);
					OutputDebugString(textbuffer);
				}
				datalength = headerByte == 0x52 ? 10 : 325;
			}
#endif
			bitIndex = nextBitIndex;
			bitCounter += jumpedBitCount;
			if (hr != S_OK)
			{
				continue;
			}

			if (headerByte == 0x52)
			{
				goto headerfound;
			}

			if (headerByte != 0x55)
			{
				continue;
			}

			bytesToCopy = 325;
			CopyRawData(buffer, g64TrackNumber, bitIndex, bytesToCopy);
			bitCounter = bitCounter + bytesToCopy * 8;
			r = D64_GCR_to_Binary(buffer, (bit8 *)&sec_data, bytesToCopy*8);
			if (r >= 0)
			{
				set_D64LoadStatus(tr, sec_header.sector, GCRDISK::Bad);
			}
			else
			{
				set_D64LoadStatus(tr, sec_header.sector, GCRDISK::OK);
				bitCounter = 0;
			}

			CopyMemory(&m_pD64Binary[D64_info[tr].file_offset + (256L * sec_header.sector)], &sec_data.data[0], 256);
			if (IsD64LoadStatusOKForD64Track(tr))
			{
				break;
			}
		}

		if (IsEventQuitSignalled())
		{
			return E_FAIL;
		}

		if (!IsD64LoadStatusOKForD64Track(tr))
		{
			int countOfSectorsOK = CountOfLoadStatus(tr, GCRDISK::OK);
			if ((g64TrackNumber & 1) == 0 && countOfSectorsOK == 0)
			{
				g64TrackNumber++;
				if (g64TrackNumber < HOST_MAX_TRACKS)
				{
					goto retryhalftrack;
				}
			}
			if (countOfSectorsOK != 0)
			{
				st = E_FAIL;
			}
		}
	}

#if defined(DEBUG) && SHOWSKEW == 1
	for (unsigned int tr = 0; tr < 35; tr++)
	{
		for (unsigned int sector = 0; sector < D64_info[tr].sector_count; sector++)
		{
			if (d64LoadStatus[tr].trackSize > 0)
			{
				if (sector == 0)
				{
					_stprintf_s(textbuffer, _countof(textbuffer), TEXT("tr=%d,sec=%d, %f\n"), tr, sector, (double)d64LoadStatus[tr].d64SectorLoadStatus[sector].bitIndex / (double)d64LoadStatus[tr].trackSize);
					OutputDebugString(textbuffer);
				}
			}
		}
	}
#endif

	return st;
}

void GCRDISK::ClearD64LoadStatus()
{
	for(unsigned int track = 0; track < _countof(d64LoadStatus); track++)
	{
		unsigned int g64track = track * 2;
		if (g64track < _countof(trackSize))
		{
			d64LoadStatus[track].trackSize = trackSize[g64track];
		}
		else
		{
			d64LoadStatus[track].trackSize = 0;
		}

		for(unsigned int sector = 0; sector < _countof(d64LoadStatus[track].d64SectorLoadStatus); sector++)
		{
			d64LoadStatus[track].d64SectorLoadStatus[sector].loaded = GCRDISK::Missing;
			d64LoadStatus[track].d64SectorLoadStatus[sector].bitIndex = 0;
		}
	}
}

GCRDISK::LoadState GCRDISK::get_D64LoadStatus(unsigned int track, unsigned int sector)
{
	if (track >= D64_MAX_TRACKS || track >= _countof(d64LoadStatus))
	{
		return GCRDISK::Bad;
	}

	if (sector >= D64_info[track].sector_count || sector >= _countof(d64LoadStatus[track].d64SectorLoadStatus))
	{
		return GCRDISK::Bad;
	}

	return d64LoadStatus[track].d64SectorLoadStatus[sector].loaded;
}

void GCRDISK::set_D64LoadStatus(unsigned int track, unsigned int sector, GCRDISK::LoadState state)
{
	if (track >= D64_MAX_TRACKS || track >= _countof(d64LoadStatus))
	{
		return;
	}
	if (sector >= D64_info[track].sector_count || sector >= _countof(d64LoadStatus[track].d64SectorLoadStatus))
	{
		return;
	}
	d64LoadStatus[track].d64SectorLoadStatus[sector].loaded = state;
}

bool GCRDISK::IsD64LoadStatusOKForD64Track(unsigned int track)
{
	if (track >= D64_MAX_TRACKS || track >= _countof(d64LoadStatus))
	{
		return false;
	}

	int sectorCount = D64_info[track].sector_count;
	if (sectorCount > D64_MAX_SECTORS)
	{
		sectorCount = D64_MAX_SECTORS;
	}

	for(int i = 0; i < sectorCount; i++)
	{
		if (d64LoadStatus[track].d64SectorLoadStatus[i].loaded != GCRDISK::OK)
		{
			return false;
		}
	}
	return true;
}

bit8 GCRDISK::CountOfLoadStatus(unsigned int track, GCRDISK::LoadState state)
{
bit8 c = 0;

	if (track >= D64_MAX_TRACKS || track >= _countof(d64LoadStatus))
	{
		return 0;
	}

	int sectorCount = D64_info[track].sector_count;
	if (sectorCount > D64_MAX_SECTORS)
	{
		sectorCount = D64_MAX_SECTORS;
	}

	for(int i = 0; i < sectorCount && i < _countof(d64LoadStatus[track].d64SectorLoadStatus); i++)
	{
		if (d64LoadStatus[track].d64SectorLoadStatus[i].loaded == state)
		{
			c++;
		}
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

	for (i=0 ; i < HOST_MAX_TRACKS ; i++)
	{
		if (trackData[i])
		{
			GlobalFree(trackData[i]);
		}

		trackSize[i] = 0;
		trackData[i] = 0;
		trackAllocatedMemorySize[i] = 0;
		if (speedZone[i])
		{
			GlobalFree(speedZone[i]);
		}

		speedZone[i] = 0;
	}

	P64ImageDestroy(&m_P64Image);
}

HRESULT GCRDISK::Init()
{
unsigned int i;
HRESULT hr = E_FAIL;
	P64ImageCreate(&this->m_P64Image);
	m_pD64Binary = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, MAX_D64_SIZE);
	if (m_pD64Binary == 0)
	{
		goto fail;
	}

	for (i=0 ; i < HOST_MAX_TRACKS ; i++)
	{
		hr = InitTrackMemory(i, G64_MAX_ALLOCATED_BYTES_PER_TRACK, false);
		if (FAILED(hr))
		{
			goto fail;
		}
	}

	return S_OK;
fail:
	Clean();
	return SetError(hr ,TEXT("Out of memory."));
}

HRESULT GCRDISK::InitTrackMemory(unsigned int trackNumber, unsigned int byteLength, bool preserveData)
{
bit8 *pTrackData = NULL;
bit8 *pSpeed = NULL;
bit32 maxLen;

	if (byteLength == trackAllocatedMemorySize[trackNumber] && trackData[trackNumber] != NULL && speedZone[trackNumber] != NULL)
	{
		trackSize[trackNumber] = 0;
		return S_OK;
	}

	// Try to prevent lots of small memory reallocation increases.
	if (preserveData)
	{
		if (byteLength < G64_MAX_ALLOCATED_BYTES_PER_TRACK)
		{
			byteLength = G64_MAX_ALLOCATED_BYTES_PER_TRACK;
		}
		else if (byteLength > trackAllocatedMemorySize[trackNumber])
		{
			if (byteLength < (trackAllocatedMemorySize[trackNumber] * 2)) 
			{
				byteLength = trackAllocatedMemorySize[trackNumber] * 2;
			}
		}
	}

	bit32 oldSpeedLength = (trackAllocatedMemorySize[trackNumber] + 3) / 4;
	bit32 newSpeedLength = (byteLength + 3) / 4;
	pTrackData = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, byteLength);
	if (!pTrackData)
	{
		return E_OUTOFMEMORY;
	}

	pSpeed = (bit8 *) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, newSpeedLength);
	if (!pSpeed)
	{
		GlobalFree(pTrackData);
		return E_OUTOFMEMORY;
	}

	if (trackData[trackNumber])
	{
		if (preserveData && trackAllocatedMemorySize[trackNumber] != 0)
		{
			maxLen = trackAllocatedMemorySize[trackNumber];
			if (maxLen > byteLength)
			{
				maxLen = byteLength;
			}

			CopyMemory(pTrackData, trackData[trackNumber], maxLen);
		}

		GlobalFree(trackData[trackNumber]);
		trackData[trackNumber] = 0;
	}

	if (speedZone[trackNumber])
	{
		if (preserveData && oldSpeedLength != 0)
		{
			maxLen = oldSpeedLength;
			if (maxLen > newSpeedLength)
			{
				maxLen = newSpeedLength;
			}

			CopyMemory(pSpeed, speedZone[trackNumber], maxLen);
		}

		GlobalFree(speedZone[trackNumber]);
		speedZone[trackNumber] = 0;
	}

	trackData[trackNumber] = pTrackData;
	speedZone[trackNumber] = pSpeed;
	trackAllocatedMemorySize[trackNumber] = byteLength;	
	trackSize[trackNumber] = 0;
	return S_OK;
}

bit16 GCRDISK::GetD64TrackSize(bit8 track)
{
	if (track >= D64_MAX_TRACKS)
	{
		track = D64_MAX_TRACKS - 1;
	}

	return (bit16)(D64_info[track].gcr_byte_count);
}

bit8 GCRDISK::GetD64SpeedZone(bit8 track)
{
	if (track >= D64_MAX_TRACKS)
	{
		track = D64_MAX_TRACKS - 1;
	}

	return D64_info[track].defaultSpeed;
}

bit8 GCRDISK::GetSpeedZone(bit8 trackNumber, bit16 byteIndex)
{
bit8 t,s;
const unsigned int SmallestTrackBits = 8;
	if (trackSize[trackNumber] >= SmallestTrackBits)
	{
		if (trackNumber >= HOST_MAX_TRACKS)
		{
			trackNumber = HOST_MAX_TRACKS - 1;
		}

		byteIndex = byteIndex % (trackSize[trackNumber] / SmallestTrackBits);
		if (byteIndex < trackAllocatedMemorySize[trackNumber])
		{
			t = speedZone[trackNumber][byteIndex/4];
			s = (3 - (byteIndex & 3)) * 2;
			t = (t >> s) & 3;
		}
	}

	return t;
}

void GCRDISK::SetSpeedZone(bit8 trackNumber, bit16 byteIndex, bit8 speed)
{
bit8 t,s,mask;
const unsigned int SmallestTrackBits = 8;
	if (trackSize[trackNumber] >= SmallestTrackBits)
	{
		byteIndex = byteIndex % (trackSize[trackNumber] / SmallestTrackBits);
		if (byteIndex < trackAllocatedMemorySize[trackNumber])
		{
			if (trackNumber < HOST_MAX_TRACKS)
			{
				speed = speed & 3;
				t = speedZone[trackNumber][byteIndex/4];
				s = (3 - (byteIndex & 3)) * 2;
				mask = 3 << s;
				speed = speed << s;
				speedZone[trackNumber][byteIndex/4] =  (t & ~mask) | speed;
			}
		}
	}
}

void GCRDISK::ConvertGCRtoP64(bit8 trackNumber)
{
unsigned long i,j,len;
bit8 speed;
bit8 byte;
unsigned long sourceSize;
unsigned long sourceCounter;
unsigned long nextRawCounter;
unsigned long rawSize;
unsigned long bits;

	rawSize = P64PulseSamplesPerRotation;
	sourceSize = 0;
	len = (trackSize[trackNumber] + 7) / 8;
	bits = 8;
	for (i = 0; i < len; i++)
	{
		if (i >= trackSize[trackNumber] / 8)
		{
			bits = trackSize[trackNumber] & 7;
		}

		speed = GetSpeedZone(trackNumber, (unsigned short) i);

		// 1 bit is 4 ticks of 
		sourceSize = sourceSize + (16 - speed) * 4 * bits;
	}

	bits = 8;
	sourceCounter = 0;
	for (i=0 ; i < len ; i++)
	{
		if (i >= trackSize[trackNumber] / 8)
		{
			bits = trackSize[trackNumber] & 7;
		}

		speed = GetSpeedZone(trackNumber, (unsigned short)i);
		speed = (16 - speed) * 4;
		byte = trackData[trackNumber][i];
		for (j = 0 ; j < bits ; j++)
		{
			nextRawCounter = (unsigned long)(((double)sourceCounter / (double)sourceSize) * (double)rawSize);
			if (nextRawCounter >= rawSize) 
			{
				break;
			}

			if ((signed char)byte < 0)
			{
				P64PulseStreamAddPulse(&this->m_P64Image.PulseStreams[P64FirstHalfTrack + trackNumber], nextRawCounter, 0xffffffff);
			}

			byte <<= 1;
			sourceCounter += speed;
		}

		if (nextRawCounter >= rawSize) 
		{
			break;
		}
	}
}

HRESULT GCRDISK::ConvertP64toGCR()
{
bit8 tr;
HRESULT hrRet = S_OK;
HRESULT hr;
	for (tr=0 ; tr < HOST_MAX_TRACKS; tr++)
	{
		if (IsEventQuitSignalled())
		{
			return E_FAIL;
		}

		hr = ConvertP64toGCR(tr);
		if (FAILED(hr))
		{
			if (SUCCEEDED(hrRet))
			{
				hrRet = hr;
			}
		}
	}

	return hrRet;
}

#if defined(SHOWGCR)
int tracklist[] = { 1, 18, 25, 31 };
bool isfirstbitzonetrack(int trackNumber)
{
	for(int i = 0; i < _countof(tracklist); i++)
	{
		if ((tracklist[i] - 1) * 2 == trackNumber)
		{
			return false;
		}
	}
	return false;
}
#endif

HRESULT GCRDISK::ConvertP64toGCR(bit8 trackNumber)
{
unsigned int i;
bit8 dataByte;
HRESULT hr;
bool memoryAllocFailed = false;
bit32 requiredTrackBitSize;
bit32 requiredTrackByteSize;

	if (trackNumber >= HOST_MAX_TRACKS)
	{
		return E_FAIL;
	}

	int defaultSpeed = (int)GetD64SpeedZone(trackNumber / 2);
	trackSize[trackNumber] = 0;
	ZeroMemory(trackData[trackNumber], trackAllocatedMemorySize[trackNumber]);
	for (i = 0 ; i < trackAllocatedMemorySize[trackNumber] ; i++)
	{
		SetSpeedZone(trackNumber, (bit16)i, defaultSpeed); 
	}

	p64_uint32_t defaultSpeedGapClockCount = (16 - defaultSpeed) * 4;
	int p64TrackIndex;
	this->m_d64TrackCount = G64_MAX_TRACKS / 2;
	this->m_d64_protectOff = m_P64Image.WriteProtected == 0;
	p64TrackIndex = P64FirstHalfTrack + trackNumber;
	TP64PulseStream *track = &m_P64Image.PulseStreams[p64TrackIndex];
	p64_int32_t currentindex;
	TP64Pulse *ppulse;
	p64_int32_t firstStrongIndex = -1;
	p64_int32_t lastStrongIndex = -1;
	if (track->UsedFirst >= 0 && track->UsedLast >=0)
	{
		for (currentindex = track->UsedFirst; currentindex >= 0; currentindex = ppulse->Next)
		{
			ppulse = &track->Pulses[currentindex];
			if (ppulse->Strength >= 0x80000000)
			{
				firstStrongIndex = currentindex;
				break;
			}
		}

		for (currentindex = track->UsedLast; currentindex >= 0; currentindex = ppulse->Previous)
		{
			ppulse = &track->Pulses[currentindex];
			if (ppulse->Strength >= 0x80000000)
			{
				lastStrongIndex = currentindex;
				break;
			}
		}

		if (firstStrongIndex >= 0)
		{
			P64PulseStreamSeek(track, 0);
			bool done = false;
			unsigned int shifterReader_UD2 = 0;
			int bitOfByteIndex = 0;
			int clockDivider1_UE7 = defaultSpeed;
			int clockDivider2_UF4 = 0;
			const int pulseFilterWidth = 40;
			unsigned int lastPulseTime = 0;//pulseFilterWidth;
			bool pulseFlip = false;
			bool lastPulseFlip = false;
			unsigned int destByteIndex = 0;			
			p64_uint32_t sourceBitPosition = 0;
			p64_uint32_t lastPosition = 0;
			p64_uint32_t delayCounter = 0;
			bool isSync = false;
			this->linePrintCount = 0;
			currentindex = track->UsedFirst;
			bool byteInShifterReady = false;
			bool getNextPulse = true;
			for (int pass = 0; pass < 2; pass++)
			{
#if defined(DEBUG) && SHOWGCR == 1
				//if (pass == SHOWPASSNO && trackNumber == SHOWTRACKNUMBER)
				if (pass == SHOWPASSNO && isfirstbitzonetrack(trackNumber))
				{
					WriteLineToDebugger();
					WriteLineToDebugger();
					WriteByteGroupedToDebugger(trackNumber);
					WriteLineToDebugger();
				}
#endif
				destByteIndex = 0;
				this->linePrintCount = 0;
				for (sourceBitPosition = 0; !done && sourceBitPosition < P64PulseSamplesPerRotation; sourceBitPosition++, delayCounter--, lastPulseTime++)
				{
					if (getNextPulse)
					{
						getNextPulse = false;
						delayCounter = P64PulseSamplesPerRotation;
						while (currentindex >= 0)
						{
							if (sourceBitPosition == 0)
							{
								currentindex = track->UsedFirst;
							}
							else
							{
								currentindex = track->Pulses[currentindex].Next;
							}

							if (currentindex < 0)
							{
								currentindex = track->UsedFirst;
							}

							if (currentindex >= 0)
							{
								ppulse = &track->Pulses[currentindex];
								if (ppulse != 0 && ppulse->Position >= 0 && ppulse->Position < (p64_uint32_t)P64PulseSamplesPerRotation)
								{
									if (ppulse->Strength >= 0x80000000)
									{
										if (ppulse->Position >= sourceBitPosition)
										{
											delayCounter = ppulse->Position - sourceBitPosition;
										}
										else
										{
											delayCounter = P64PulseSamplesPerRotation - sourceBitPosition + ppulse->Position;
										}

										lastPosition = ppulse->Position;
										break;
									}
								}
							}
						}
					}

					clockDivider1_UE7 = (clockDivider1_UE7 + 1) & 0xf;
					if (lastPulseTime == pulseFilterWidth && pulseFlip != lastPulseFlip)
					{
						lastPulseFlip = pulseFlip;
						clockDivider1_UE7 = defaultSpeed;
						clockDivider2_UF4 = 0;
					}

					if (delayCounter == 0)
					{
						pulseFlip = !pulseFlip;
						lastPulseTime = 0;
						getNextPulse = true;
					}

					if (clockDivider1_UE7 == 0)
					{
						clockDivider1_UE7 = defaultSpeed;
						clockDivider2_UF4 = (clockDivider2_UF4 + 1) & 0xf;
						if ((clockDivider2_UF4 & 3) == 2)//QB rising
						{
							shifterReader_UD2 <<= 1;
							if ((clockDivider2_UF4 & 0xc) == 0)
							{
								shifterReader_UD2 |= 1;
							}

							bitOfByteIndex = (bitOfByteIndex + 1) & 7;
							byteInShifterReady = (bitOfByteIndex == 0);
							if ((shifterReader_UD2 & 0x3ff) == 0x3ff)
							{
								isSync = true;
							}
						}
						
						if (byteInShifterReady)
						{
							if ((clockDivider2_UF4 & 2) == 0)
							{
								byteInShifterReady = false;
								dataByte = (bit8)(shifterReader_UD2 & 0xff);
								requiredTrackByteSize = destByteIndex + 1;
								if (requiredTrackByteSize >= trackAllocatedMemorySize[trackNumber])
								{
									hr = this->InitTrackMemory(trackNumber, requiredTrackByteSize, true);
									if (FAILED(hr))
									{
										memoryAllocFailed = true;
									}
								}

								if (destByteIndex < trackAllocatedMemorySize[trackNumber])
								{
									trackData[trackNumber][destByteIndex] = dataByte;
									destByteIndex++;
								}
#if defined(DEBUG) && SHOWGCR == 1
								//if (pass == SHOWPASSNO && trackNumber == SHOWTRACKNUMBER)
								if (pass == SHOWPASSNO && isfirstbitzonetrack(trackNumber))
								{
									WriteByteGroupedToDebugger(dataByte);
								}
#endif
							}
						}
					}
				}

				unsigned int numtrailingbits = 0;
				if (bitOfByteIndex > 0 || byteInShifterReady)
				{
					unsigned int shift = 0;
					bitOfByteIndex = bitOfByteIndex & 7;
					numtrailingbits = (bitOfByteIndex > 0) ? bitOfByteIndex : 8;
					shift = (bitOfByteIndex > 0) ? (8 - numtrailingbits) : 0;
					unsigned int maskoff = (0xff << shift) & 0xff;
					unsigned int trailingbits = (shifterReader_UD2 << shift) & 0xff;
					requiredTrackBitSize = (destByteIndex + 1) * 8;
					requiredTrackByteSize = requiredTrackBitSize / 8;
					if (destByteIndex >= trackAllocatedMemorySize[trackNumber])
					{
						hr = this->InitTrackMemory(trackNumber, requiredTrackByteSize, true);
						if (FAILED(hr))
						{
							memoryAllocFailed = true;
						}
					}

					if (destByteIndex < trackAllocatedMemorySize[trackNumber])
					{
						trackData[trackNumber][destByteIndex] = trailingbits;
					}
#if defined(DEBUG) && SHOWGCR == 1
					//if (pass == SHOWPASSNO && trackNumber == SHOWTRACKNUMBER)
					if (pass == SHOWPASSNO && isfirstbitzonetrack(trackNumber))
					{
						WriteByteToDebugger(trailingbits);
						OutputDebugString(TEXT(" / "));
						WriteByteToDebugger(numtrailingbits);
						WriteLineToDebugger();
					}
#endif
				}

				requiredTrackByteSize = destByteIndex + 1;
				requiredTrackBitSize = requiredTrackByteSize * 8;
				if (requiredTrackByteSize > trackAllocatedMemorySize[trackNumber])
				{
					trackSize[trackNumber] = trackAllocatedMemorySize[trackNumber];
				}
				else
				{
					trackSize[trackNumber] = requiredTrackBitSize;
				}

				bitOfByteIndex = 0;
				byteInShifterReady = false;
			}
		}
	}

	if (memoryAllocFailed)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

void GCRDISK::WriteLineToDebugger()
{
	OutputDebugString(TEXT("\n"));
}

void GCRDISK::WriteByteGroupedToDebugger(bit8 dataByte)
{
	TCHAR sDebug[50];
	_stprintf_s(sDebug, _countof(sDebug), TEXT("%2X "), dataByte);		
	OutputDebugString(sDebug);
	linePrintCount++;
	if (linePrintCount >= 16)
	{
		linePrintCount = 0;
		OutputDebugString(TEXT("\n"));
	}
}

void GCRDISK::WriteByteToDebugger(bit8 dataByte)
{
	TCHAR sDebug[50];
	_stprintf_s(sDebug, _countof(sDebug), TEXT("%02X"), dataByte);
	OutputDebugString(sDebug);
}

p64_uint32_t GCRDISK::CountP64ImageMaxTrackPulses(const TP64Image& image)
{
	p64_uint32_t count = 0;
	p64_uint32_t maxcount = 0;
	for (int tr = 0; tr < G64_MAX_TRACKS; tr++)
	{
		count = CountP64ImageTrackPulses(image, tr);
		if (count > maxcount)
		{
			maxcount = count;
		}
	}
	return maxcount;
}

p64_uint32_t GCRDISK::CountP64ImageTrackPulses(const TP64Image& image, unsigned int trackNumber)
{
	p64_uint32_t count = 0;	
	if (trackNumber < G64_MAX_TRACKS)
	{
		count = CountP64TrackPulses(image.PulseStreams[P64FirstHalfTrack + trackNumber]);
	}
	return count;
}

p64_uint32_t GCRDISK::CountP64TrackPulses(const TP64PulseStream& track)
{
	p64_uint32_t count = 0;		
	p64_int32_t currentIndex = track.UsedFirst;
	while (currentIndex >= 0 && count < P64PulseSamplesPerRotation)
	{
		currentIndex = track.Pulses[currentIndex].Next;
		count++;
	}
	return count;
}

bit8 GCRDISK::GetSectorErrorCode(bit8 *d64Binary, bit16 errorBytes, bit8 trackNumber, bit8 sectorNumber)
{
bit32 i;
	if (errorBytes == 683)
	{
		i = D64_info[35].file_offset;
	}
	else if (errorBytes == 768)
	{
		i = D64_info[40].file_offset;
	}
	else
	{
		return 1;
	}

	if (trackNumber>=40)
	{
		return 1;
	}

	i += D64_info[trackNumber].sector_total - D64_info[trackNumber].sector_count + sectorNumber;
	return d64Binary[i];

}

HRESULT GCRDISK::MakeGCRImage(bit8 *d64Binary, bit32 tracks, bit16 errorBytes, bool bAlignD64Tracks)
{
bit32 i,j;
bit8 *pb,*pGcr,*p,sum,id1,id2,sec,tr,tr2;
bit8 d64_sector_binary[D64_SECTOR_SIZE];
bit8 tempGcrBuffer[260*5/4];
bit8 c;
bit8 sectorError;
bit8 id1x,id2x;
unsigned int wi = 0;
unsigned int trackByteLen;
unsigned int dostracknumber;
HRESULT hr;
bool memoryAllocFailed = false;

	for(tr = 0 ; tr < HOST_MAX_TRACKS ; tr++)
	{
		bit32 requiredTrackByteSize = GetD64TrackSize(tr/2);
		bit32 requiredTrackBitSize = requiredTrackByteSize * 8;
		if (requiredTrackByteSize > trackAllocatedMemorySize[tr])
		{
			hr = this->InitTrackMemory(tr, requiredTrackByteSize, false);
			if (FAILED(hr))
			{
				memoryAllocFailed = true;
			}
		}

		bit32 actualTrackByteSize = trackAllocatedMemorySize[tr];
		bit32 actualTrackBitSize = actualTrackByteSize * 8;
		if (requiredTrackBitSize < actualTrackBitSize)
		{
			actualTrackBitSize = requiredTrackBitSize;
			actualTrackByteSize = requiredTrackByteSize;
		}

		trackSize[tr] = actualTrackBitSize;
		memset(trackData[tr], 0x55, actualTrackByteSize);
		for (i = 0; i < requiredTrackByteSize; i++)
		{
			SetSpeedZone(tr, (bit16)i, GetD64SpeedZone(tr/2));
		}
	}

	id1x = d64Binary[D64_info[17].file_offset + 162];
	id2x = d64Binary[D64_info[17].file_offset + 163];
	for(tr = 0 ; tr < (tracks * 2) ; tr++)
	{
		tr2 = tr >> 1;
		dostracknumber = tr2 + 1;
		if ((tr & 1) != 0)
        {
			continue;
        }

		trackByteLen = trackSize[tr] / 8;
		if (trackByteLen == 0)
		{
			continue;
		}

		pGcr = &trackData[tr][0];
		if (bAlignD64Tracks)
        {
			wi = 0;
        }
		else
        {
			wi = ((int)(((double)(trackSize[tr]/8)) * (D64_info[tr/2].track_stagger))) % (trackByteLen);
        }
	
		for(c = 0; c < D64_info[tr2].sector_count; c++)
		{
			/*sector interleave*/
			sec=c;
			if (errorBytes!=0)
            {
				sectorError = GetSectorErrorCode(d64Binary, errorBytes, tr2, sec);
            }
			else
            {
				sectorError = 1;
            }

			if (sectorError!=3)
			{
				for (j = 0 ; j < D64_SYNCLENGTH; j++, wi=(wi+1) % trackByteLen)
                {
					pGcr[wi] = 0xff;
                }
			}
			else
			{
				for (j = 0 ; j < D64_SYNCLENGTH; j++, wi=(wi+1) % trackByteLen)
                {
					pGcr[wi] = 0x00;
                }
			}

			p= &d64_sector_binary[0];
			if (sectorError!=2)
            {
				p[0] = 0x8;
            }
			else
            {
				p[0] = 0x0;
            }

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
            {
				p[1] = sec ^ (dostracknumber) ^ id2 ^ id1;
            }
			else
            {
				p[1] = ~(sec ^ (dostracknumber) ^ id2 ^ id1);
            }

			p[2] = sec;
			p[3] = (dostracknumber);
			p[4] = id2;
			p[5] = id1;
			p[6] = 0xf;
			p[7] = 0xf;
			D64_Binary_to_GCR(p, &tempGcrBuffer[0], 8);
			for (j = 0 ; j < 10; j++, wi=(wi+1) % trackByteLen)
            {
				pGcr[wi] = tempGcrBuffer[j];
            }

			for (j = 0 ; j < D64_GAPHEADER; j++, wi=(wi+1) % trackByteLen)
            {
				pGcr[wi]=0x55;
            }

			if (sectorError!=3)
			{
				for (j = 0 ; j < D64_SYNCLENGTH; j++, wi=(wi+1) % trackByteLen)
                {
					pGcr[wi] = 0xff;
                }
			}
			else
			{
				for (j = 0 ; j < D64_SYNCLENGTH; j++, wi=(wi+1) % trackByteLen)
                {
					pGcr[wi] = 0x00;
                }
			}

			pb = &d64Binary[D64_info[tr2].file_offset + 256 * (unsigned long)sec];
			if (sectorError!=4)
            {
				p[0] = 0x7;
            }
			else
            {
				p[0] = 0x0;
            }

			sum=0;
			for (i=0 ; i < 256 ; i++)
			{
				p[i+1] = pb[i];
				sum = sum ^ pb[i];
			}

			if (sectorError!=5)
            {
				p[257] = sum;
            }
			else
            {
				p[257] = ~sum;
            }

			p[258] = 0;
			p[259] = 0;
			D64_Binary_to_GCR(p, &tempGcrBuffer[0], 260);
			for (j = 0 ; j < (260*5/4); j++, wi=(wi+1) % trackByteLen)
            {
				pGcr[wi] = tempGcrBuffer[j];
            }

			bit32 sectorgap;
			if (dostracknumber < 18)
			{
				sectorgap = D64_GAPSECTOR_1_17;
			}
			else if (dostracknumber < 25)
			{
				sectorgap = D64_GAPSECTOR_18_24;
			}
			else if (dostracknumber < 31)
			{
				sectorgap = D64_GAPSECTOR_25_30;
			}
			else
			{
				sectorgap = D64_GAPSECTOR_31_40;
			}

			for (j = 0 ; j < sectorgap; j++, wi=(wi+1) % trackByteLen)
            {
				pGcr[wi]=0x55;
            }			
		}
	}

	for (tr=0 ; tr < HOST_MAX_TRACKS ; tr++)
    {
		ConvertGCRtoP64(tr);
    }

	hr = S_OK;
	if (memoryAllocFailed)
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		return S_OK;
	}

	return hr;
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
        {
			hr = SetError(E_FAIL,TEXT("Could not read from %s."),filename);
        }
		else
        {
			hr = SetError(E_FAIL,TEXT("Could not read from file."));
        }
	}
	else
    {
		hr = S_OK;
    }
	if (bytesRead)
    {
		*bytesRead = bytes_read;
    }
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
unsigned long i;
DWORD file_size,j;
bit32 data32;
bit8 speed;
bit8 data8;
bit16 s;

	ClearError();
	P64ImageClear(&m_P64Image);
	m_d64TrackCount = G64_MAX_TRACKS / 2;

	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL, TEXT("Could not open %s."),filename);
	}

	hr = ReadFromFile(hfile, filename, &g64Header.signature[0], sizeof(struct G64Header), 0);
	if (FAILED(hr))
	{
		return hr;
	}

	if (_memicmp(&g64Header.signature[0],"GCR-1541", 8)!=0)
	{
		return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
	}

	if (g64Header.version != 0)
	{
		return SetError(E_FAIL,TEXT("%s is not a supported G64 file version. Version 2.x only is supported."), filename);
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&data32, 4, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		if (data32 + 2 >= file_size)
		{
			return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
		}

		trackOffset[tr] = data32;
	}
	
	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		hr = ReadFromFile(hfile, filename, (char *)&data32, 4, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		if (data32 > 3)
		{
			if (data32 + 2 >= file_size)
			{
				return SetError(E_FAIL,TEXT("%s is not a valid G64 file."), filename);
			}
		}

		speedOffset[tr] = data32;
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		speed = GetD64SpeedZone(tr/2);
		ZeroMemory(trackData[tr], trackAllocatedMemorySize[tr]);
		for (i = 0 ; i < trackAllocatedMemorySize[tr] ; i++)
		{
			SetSpeedZone(tr, (bit16)i, speed); 
		}

		trackSize[tr] = 0;
	}

	for (tr=0 ; tr < G64_MAX_TRACKS ; tr++)
	{
		if (trackOffset[tr] != 0)
		{
			r = SetFilePointer (hfile, trackOffset[tr], 0L, FILE_BEGIN);
			if (r == INVALID_SET_FILE_POINTER)
			{
				return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
			}

			hr = ReadFromFile(hfile, filename, (char *)&s, 2, 0);
			if (FAILED(hr))
			{
				return hr;
			}

			if (s > trackAllocatedMemorySize[tr])
			{
				hr = InitTrackMemory(tr, s, false);
				if (FAILED(hr))
				{
					return hr;
				}
			}

			if (s > g64Header.fileAllocatedTrackSize)
			{
				double track = ((double)(tr) / 2.0) + 1.0;
				return SetError(E_FAIL,TEXT("The track %.1f with length %d is longer than the G64 file header file wide track allocation of %d."), (double)track, (unsigned int)s, (unsigned int)g64Header.fileAllocatedTrackSize);
			}

			if (s==0)
			{
				trackSize[tr] = 0;
				continue;
			}

			trackSize[tr] =  (bit32)s * 8L;
			hr = ReadFromFile(hfile, filename, (char *)&trackData[tr][0], (DWORD)s, 0);
			if (FAILED(hr))
			{
				return hr;
			}

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
						data8 = 0;
						hr = ReadFromFile(hfile, filename, (char *)&data8, 1, 0);
						if (FAILED(hr))
						{
							return hr;
						}
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
		{
			ConvertGCRtoP64(tr);
		}
	}

	m_d64_protectOff=0;
	return S_OK;
}

HRESULT GCRDISK::LoadP64FromFileHandle(HANDLE hfile, TCHAR *filename)
{
TP64MemoryStream P64MemoryStreamInstance;
DWORD file_size;
DWORD bytes_read;
HRESULT hr = E_FAIL;

	ClearError();
	P64MemoryStreamCreate(&P64MemoryStreamInstance);
	P64ImageClear(&m_P64Image);
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE != file_size && file_size != 0)
	{		
		char *buffer = (char *)malloc(file_size);
		if (buffer != 0)
		{
			hr = this->ReadFromFileQ(hfile, buffer, file_size, &bytes_read);
			if (SUCCEEDED(hr))
			{
				P64MemoryStreamWrite(&P64MemoryStreamInstance, (p64_uint8_t *)buffer, file_size);
				P64MemoryStreamSeek(&P64MemoryStreamInstance, 0);
				if (P64ImageReadFromStream(&m_P64Image, &P64MemoryStreamInstance, this))
				{
					int hostTrackIndex;
					int p64TrackIndex;
					this->m_d64TrackCount = G64_MAX_TRACKS / 2;
					this->m_d64_protectOff = m_P64Image.WriteProtected == 0;
					for (p64TrackIndex = P64FirstHalfTrack, hostTrackIndex = 0; p64TrackIndex < _countof(m_P64Image.PulseStreams) && hostTrackIndex < G64_MAX_TRACKS; p64TrackIndex++, hostTrackIndex++)
					{
						TP64PulseStream *p64sourcetrack = &m_P64Image.PulseStreams[p64TrackIndex];
						P64PulseStreamSeek(p64sourcetrack, 0);
					}
				}
				else
				{
					hr = SetError(E_FAIL, TEXT("P64 read file structure failed."));
				}
			}
			else
			{
				SetError(hr, TEXT("P64 read failed."));
			}
			if (buffer != 0)
			{
				free(buffer);
				buffer = 0;
			}
		}
		else
		{
			hr = SetError(E_OUTOFMEMORY, TEXT("Out of memory."));
		}
	}
	else
	{
		hr = SetError(E_FAIL, TEXT("Could not open %s."), filename);
	}
	P64MemoryStreamDestroy(&P64MemoryStreamInstance);
	//P64ImageDestroy(&P64Image);
    return hr;
}

HRESULT GCRDISK::LoadFDIFromFileHandle(HANDLE hfile, TCHAR *filename)
{
HRESULT hr;
DWORD r,dw,filePointer;
struct FDIHeader fdiHeader;
bit8 tr,ftr;
DWORD file_size;
struct FDITrackDescription *fdiTrackDescription = &fdiHeader.trackDescription[0];

	ClearError();
	P64ImageClear(&m_P64Image);
	file_size = GetFileSize(hfile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."), filename);
	}

	hr = ReadFromFile(hfile, filename, &fdiHeader.signature[0], sizeof(struct FDIHeader), 0);
	if (FAILED(hr))
	{
		return hr;
	}

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
		if ((fdiHeader.tpi == 0) && ((fdiHeader.ltrack+1) * 2 > HOST_MAX_TRACKS))
		{
			fdiHeader.ltrack = HOST_MAX_TRACKS/2 - 1;
		}

		m_d64TrackCount = fdiHeader.ltrack+1;
	}
	else if (fdiHeader.tpi == 2)
	{
		if ((fdiHeader.tpi == 2) && ((fdiHeader.ltrack+1) > HOST_MAX_TRACKS))
		{
			fdiHeader.ltrack = HOST_MAX_TRACKS - 1;
		}

		m_d64TrackCount = (fdiHeader.ltrack+1) / 2;
	}
	else
	{
		return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);
	}

	r = SetFilePointer (hfile, 152, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}

	ftr=0;
	tr=0;
	while (ftr<=fdiHeader.ltrack)
	{
		if (IsEventQuitSignalled())
		{
			return E_FAIL;
		}

		hr = ReadFromFile(hfile, filename, (char *)&fdiTrackDescription[ftr], sizeof(struct FDITrackDescription), 0);
		if (FAILED(hr))
		{
			return hr;
		}

		ftr++;
	}

	ftr=0;
	tr=0;
	filePointer = 0x200;
	while (ftr <= fdiHeader.ltrack)
	{
		if (IsEventQuitSignalled())
		{
			return E_FAIL;
		}

		if (fdiTrackDescription[ftr].type == 0)
		{
			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		}
		else if (fdiTrackDescription[ftr].type >= 0x80 && fdiTrackDescription[ftr].type <= 0xbf)
		{//pulses-index streams
			hr = FDIReadTrackStream(hfile, filePointer, tr);
			if (FAILED(hr))
			{
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			}

			dw = fdiTrackDescription[ftr].size;
			dw = dw | ((DWORD)(fdiTrackDescription[ftr].type & 0x3f)<<8);
			dw *= 0x100;
		}
		else if ((fdiTrackDescription[ftr].type & 0xf0) == 0xd0)
		{
			hr = FDIReadGCR(hfile, filePointer, tr, fdiTrackDescription[ftr].type);
			if (FAILED(hr))
			{
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			}

			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		}
		else if ((fdiTrackDescription[ftr].type & 0xf0) == 0xc0)
		{
			hr = FDIReadDecodedGCR(hfile, filePointer, tr, fdiTrackDescription[ftr].type);
			if (FAILED(hr))
			{
				return SetError(hr, TEXT("%s is not a valid C64 FDI file."), filename);
			}
			dw = (DWORD)fdiTrackDescription[ftr].size * 256;
		}
		else
		{
			return SetError(E_FAIL,TEXT("%s is not a valid C64 FDI file."), filename);
		}

		filePointer += dw;
		if (fdiHeader.tpi == 0)
		{
			tr+=2;
		}
		else
		{
			tr+=1;
		}

		ftr++;
	}

	if (fdiHeader.flags & 1)
	{
		m_d64_protectOff = 0;
	}
	else
	{
		m_d64_protectOff = 1;
	}

	if (IsEventQuitSignalled())
	{
		return E_FAIL;
	}
	if (wordswap(fdiHeader.version) >= 0x0201)
	{
		return FDICheckCRC(hfile, filename, file_size);
	}

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
	bit32 speedLength = (trackAllocatedMemorySize[trackNumber] + 3) / 4;
	FillMemory(&speedZone[trackNumber][0], speedLength, speedG64);
}


HRESULT GCRDISK::FDIReadGCR(HANDLE hfile, DWORD filePointer, bit8 trackNumber, bit8 fdiTrackType)
{
HRESULT hr;
bit32 trackBitLength;
bit32 trackByteLength;
bit32 indexPos;
bit8 speed;
DWORD r;

	trackSize[trackNumber] = 0;
	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&trackBitLength, 4, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	if (trackBitLength > G64_MAX_BYTES_PER_TRACK * 10)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&indexPos, 4, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	trackByteLength = (trackBitLength + 7) / 8;
	if (trackByteLength > trackAllocatedMemorySize[trackNumber])
	{
		hr = InitTrackMemory(trackNumber, trackByteLength, false);
		if (SUCCEEDED(hr))
		{
			trackSize[trackNumber] = trackBitLength;
		}
		else
		{
			return hr;
		}
	}
	
	if (trackByteLength < 6000)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&trackData[trackNumber][0], trackByteLength, 0);
	if (FAILED(hr))
	{
		return hr;
	}
	
	speed = FDIDecodeBitRate(fdiTrackType);
	if (speed == 0xff)
	{
		speed = 3;
	}

	G64SetTrackSpeedZone(trackNumber, speed);
	ConvertGCRtoP64(trackNumber);
	return S_OK;
}

void GCRDISK::WriteGCRBit(bit8 trackNumber, bit32 index, bit8 v)
{
bit8 i,m;
	
	// this->trackSize is invalid during this call so we check this->trackAllocatedMemorySize
	bit32 byteIndex = index / 8;
	if (trackAllocatedMemorySize[trackNumber] > 0 && byteIndex < trackAllocatedMemorySize[trackNumber])
	{
		i = trackData[trackNumber][index/8];
		m = 1 << (7-(index & 7));
		v = v << (7-(index & 7));
		trackData[trackNumber][index/8] = (i & ~m) | v;
	}
}

void GCRDISK::WriteGCRByte(bit8 trackNumber, bit32 index, bit8 v)
{
int j;

	for (j=7; j >= 0; j--)
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

	trackSize[trackNumber] = 0;
	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&encoding, 1, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	if (encoding!=1)//Commodore GCR
	{
		return hr;
	}

	hr = ReadFromFileQ(hfile, (char *)&indexPos, 3, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	c=0;
	i=0;
	bool done=false;
	while (!done)
	{
		bit32 byteIndex = i / 8;
		if (byteIndex > trackAllocatedMemorySize[trackNumber] * 8)
		{
			hr = this->InitTrackMemory(trackNumber, byteIndex * 2, true);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		hr = ReadFromFileQ(hfile, (char *)&token, 1, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		switch(token)
		{
		case 0x2://CBM sync mark {word size in bits}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 2, 0);
			if (FAILED(hr))
			{
				return hr;
			}

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
			{
				return hr;
			}

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
			{
				return hr;
			}

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
			{
				return hr;
			}

			wordData = wordswap(wordData);
			size = wordData/8;
			if (token=0xb)
			{
				size+=0x2000;
			}

			for (c=0 ; c < size ; c++)
			{
				hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
				if (FAILED(hr))
				{
					return hr;
				}

				WriteGCRByte(trackNumber, i, byteData);
				i+=8;
			}

			if (wordData & 7)
			{
				hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
				if (FAILED(hr))
				{
					return hr;
				}

				WriteGCRByte(trackNumber, i, byteData);
				i+=(wordData & 7);
			}
			break;
		case 0xc://CBM GCR decoded {word size in nibbles}, {ceil(size in bits/8 data bytes)}
			hr = ReadFromFileQ(hfile, (char *)&wordData, 1, 0);
			if (FAILED(hr))
			{
				return hr;
			}

			size = wordData;
			for (c=0 ; c < size ; c++)
			{
				if ((c & 1) == 0)
				{
					hr = ReadFromFileQ(hfile, (char *)&byteData, 1, 0);
					if (FAILED(hr))
					{
						return hr;
					}

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
	
	trackSize[trackNumber] = i;	
	speed = FDIDecodeBitRate(fdiTrackType);
	if (speed == 0xff)
	{
		return E_FAIL;
	}

	G64SetTrackSpeedZone(trackNumber, speed);
	ConvertGCRtoP64(trackNumber);
	return S_OK;
}

HRESULT GCRDISK::FDIReadTrackStream(HANDLE hfile, DWORD filePointer, bit8 trackNumber)
{
HRESULT hr;
DWORD r,d,i;
FDIStreamsHeader fdiStreamsHeader;
HuffDecompression hd;

	hr = hd.SetFile(hfile, false);
	if (FAILED(hr))
	{
		return hr;
	}
	r = SetFilePointer (hfile, filePointer, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return E_FAIL;
	}

	hr = ReadFromFileQ(hfile, (char *)&d, 4, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	fdiStreamsHeader.numPulses = dwordswap(d);
	if (fdiStreamsHeader.numPulses > 0)
	{
		d = 0;
		hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
		if (FAILED(hr))
		{
			return hr;
		}
		d = FDI3To4(d);
		fdiStreamsHeader.aveCompression = (d >> 22) & 3;
		fdiStreamsHeader.aveSize = d & 0x3fffff;

		d = 0;
		hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		d = FDI3To4(d);
		fdiStreamsHeader.minCompression = (d >> 22) & 3;
		fdiStreamsHeader.minSize = d & 0x3fffff;

		d = 0;
		hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		d = FDI3To4(d);
		fdiStreamsHeader.maxCompression = (d >> 22) & 3;
		fdiStreamsHeader.maxSize = d & 0x3fffff;

		d = 0;
		hr = ReadFromFileQ(hfile, (char *)&d, 3, 0);
		if (FAILED(hr))
		{
			return hr;
		}

		d = FDI3To4(d);
		fdiStreamsHeader.idxCompression = (d >> 22) & 3;
		fdiStreamsHeader.idxSize = d & 0x3fffff;

		if (fdiStreamsHeader.aveSize != 0)
		{
			//Ave
			if (fdiStreamsHeader.aveCompression==0)
			{
				hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.aveData);
			}
			else if (fdiStreamsHeader.aveCompression==1)
			{
				hr = hd.Decompress(fdiStreamsHeader.numPulses, &fdiStreamsHeader.aveData);
			}
			else
			{
				hr = E_FAIL;
			}
			if (FAILED(hr))
			{
				return hr;
			}

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
				{
					hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.minData);
				}
				else if (fdiStreamsHeader.minCompression==1)
				{
					hr = hd.Decompress(fdiStreamsHeader.numPulses, &fdiStreamsHeader.minData);
				}
				else
				{
					hr = E_FAIL;
				}
				if (FAILED(hr))
				{
					return hr;
				}
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
					{
						hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.maxData);
					}
					else if (fdiStreamsHeader.maxCompression==1)
					{
						hr =  hd.Decompress(fdiStreamsHeader.numPulses, &fdiStreamsHeader.maxData);
					}
					else
					{
						hr = E_FAIL;
					}
					if (FAILED(hr))
					{
						return hr;
					}
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
				{
					hr = FDICopyTrackStream(hfile, fdiStreamsHeader.numPulses, &fdiStreamsHeader.idxData);
				}
				else if (fdiStreamsHeader.idxCompression==1)
				{
					hr =  hd.Decompress(fdiStreamsHeader.numPulses, &fdiStreamsHeader.idxData);
				}
				else
				{
					hr = E_FAIL;
				}
				if (FAILED(hr))
				{
					return hr;
				}

				//Find the index value that is used to indicate a string pulse;
				for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
				{
					unsigned int index1state = (fdiStreamsHeader.idxData[i] >> 8) & 0xff;
					unsigned int index0state = (fdiStreamsHeader.idxData[i]) & 0xff;
					unsigned int pulseStrength = index1state + index0state;
					if (pulseStrength > maxPulseStrength)
					{
						maxPulseStrength = pulseStrength;
					}
				}
			}

			if (fdiStreamsHeader.numPulses > 0)
			{
				unsigned int pulseStrength=0;
				bool bIsStrongPulse;
				bool bIsNoticablePulse;
				unsigned __int64 totalTime;//Total FDI track length.
				unsigned __int64 pulseTime;//FDI distance from last strong pulse.
				unsigned __int64 sumPulseTime;//FDI distance of current pulse from start of track.
				unsigned __int64 sumStrongPulseTime;//FDI distance of last strong pulse from start of track.
				double clockTime;
				totalTime=0;
				//Loop through the aveData to total up the track length.
				for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
				{
					if (IsEventQuitSignalled())
					{
						return E_FAIL;
					}
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
					{
						totalTime += fdiStreamsHeader.aveData[i];
					}
				}
	
				pulseTime=0;
				sumPulseTime=0;
				sumStrongPulseTime = 0;
				//Loop through aveData a second time to write the pulses to the emulated disk
				for (i = 0 ; i < fdiStreamsHeader.numPulses ; i++)
				{
					if (IsEventQuitSignalled())
					{
						return E_FAIL;
					}

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

					//pulseTime should be greater than zero.
					pulseTime = (unsigned __int64)fdiStreamsHeader.aveData[i];
					//Skip zero pulse times.
					if (pulseTime == 0)
					{
						continue;
					}
					if (pulseTime < 0)
					{
						return E_FAIL;
					}

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
					{
						continue;
					}

					clockTime = (double)(sumPulseTime - 1) / (double)totalTime * (double)(P64PulseSamplesPerRotation);
					p64_uint32_t pulsePosition = (p64_uint32_t)floor(clockTime);
					if (pulsePosition >= 0  && pulsePosition < P64PulseSamplesPerRotation)
					{
						//Write the pulse to the emulated disk.
						P64PulseStreamAddPulse(&this->m_P64Image.PulseStreams[P64FirstHalfTrack + trackNumber], pulsePosition, 0xffffffff);
					}
				}
			}
		}
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

HRESULT GCRDISK::LoadP64FromFile(TCHAR *filename)
{
HANDLE hfile=0;
HRESULT hr;

	hfile=0;
	hfile=CreateFile(filename,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not open %s."),filename);
	}
	hr= LoadP64FromFileHandle(hfile, filename);
	if (FAILED(hr))
	{
		CloseHandle(hfile);
		return hr;
	}
	CloseHandle(hfile);
	return hr;
}

HRESULT GCRDISK::LoadFDIFromFile(TCHAR *filename)
{
HANDLE hfile=0;
HRESULT hr;

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
HRESULT hr = S_OK;

	ClearError();
	P64ImageClear(&m_P64Image);
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
	{
		hr = MakeGCRImage(m_pD64Binary, m_d64TrackCount, d64Errors, bAlignD64Tracks);
	}

	m_d64_protectOff=0;
	return hr;
}

#define SOFFSET(t, m)	(INT_PTR)(&(((t *)(0))->m))

HRESULT GCRDISK::SaveFDIToFile(TCHAR *filename)
{
BOOL rb;
FDIHeader fdiHeader;
HANDLE hFile;
DWORD bytes_written,file_size,r;
struct FDITrackDescription fdiTrackDescription[HOST_MAX_TRACKS];
long i,j;
bit8 tr;
HRESULT hr;
struct FDIRawTrackHeader fdiRawTrackHeader;
bit32 pulseCountArray[HOST_MAX_TRACKS];
bit32 maxPulseCount;
bit32 currentPulseCount;
FDIData buffer;
bit32 *p;
HuffCompression hw;
bit32 nextTrackWrite;
bit32 compressedBufferSize;
bit32 writeSize;
long zeroPadSize;
bit8 currentByte;
CRC32Alloc crc;

	ClearError();
	if (sizeof(FDIHeader) != 0x200)
	{
		return SetError(E_FAIL,TEXT("Internal error. FDIHeader size was found not to be 0x200."));
	}

	if (!crc.isOK)
	{
		return E_OUTOFMEMORY;
	}

	crc.pCRC32->Init(CRC32POLY,0xffffffff,0xffffffff, true);
	hFile=CreateFile(filename, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
	if (hFile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not save %s."), filename);
	}
	hr = hw.SetFile(hFile, true);
	if (FAILED(hr))
	{
		CloseHandle(hFile);
		return SetError(hr, TEXT("Could not save %s."), filename);
	}

	const char Signature[] = "Formatted Disk Image file\r\n";
	const char Creator[] = "Hoxs64 1541 FDI save v1.0.1";
	memset(&fdiHeader, 0, sizeof(FDIHeader));
	memcpy(fdiHeader.signature, Signature, strlen(Signature));
	memset(fdiHeader.creator, 0x20, sizeof(fdiHeader.creator));
	memcpy(fdiHeader.creator, Creator, strlen(Creator));
	fdiHeader.cr[0] = '\r';
	fdiHeader.cr[1] = '\n';
	memset(fdiHeader.comment, 0x1a, sizeof(fdiHeader.comment));
	fdiHeader.eof = 0x1a;
	fdiHeader.version = wordswap(0x0201);
	fdiHeader.ltrack = wordswap(HOST_MAX_TRACKS - 1);
	fdiHeader.lhead = 0;
	fdiHeader.type = 1;
	fdiHeader.rotspeed = 0xac;
	fdiHeader.flags = (m_d64_protectOff==0);
	fdiHeader.tpi = 2;
	fdiHeader.headwidth = 2;
	fdiHeader.reserved = 0;
	
	rb = WriteFile(hFile, &fdiHeader, sizeof(FDIHeader), &bytes_written, NULL);
	if (!rb)
	{
		return SetError(E_FAIL,TEXT("Could not save %s."),filename);
	}

	hr = hw.Init();
	if(FAILED(hr))
	{
		return SetError(hr, TEXT("Could not initialise huffmann compression."));
	}

	//Find how many pulses are on the track with the most pulses.	
	ZeroMemory(pulseCountArray, sizeof(pulseCountArray));
	p64_int32_t currentIndex;
	PP64PulseStream track;
	p64_uint32_t currentPosition;
	p64_uint32_t previousPosition;
	bit32 destIndex = 0;
	maxPulseCount = 0;
	for (tr=0; tr < HOST_MAX_TRACKS; tr++)
	{
		pulseCountArray[tr] = 0;
		track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + tr];
		if (track->UsedFirst >= 0 && track->UsedLast > 0)
		{
			currentIndex = track->UsedFirst;
			destIndex = 0;
			currentPosition = 0;
			previousPosition = track->Pulses[track->UsedLast].Position;
			if (previousPosition < P64PulseSamplesPerRotation)
			{
				for (;currentIndex >= 0; destIndex++, previousPosition = currentPosition, currentIndex = track->Pulses[currentIndex].Next)
				{
					currentPosition = track->Pulses[currentIndex].Position;
					if ((currentPosition <= previousPosition && destIndex > 0) || currentPosition >= P64PulseSamplesPerRotation)
					{
						break;
					}
					pulseCountArray[tr]++;			
				}
			}
			if (pulseCountArray[tr] > maxPulseCount)
			{
				maxPulseCount = pulseCountArray[tr];
			}
		}
	}

	//Allocate one buffer that can hold the largest track which is the track with the most pulses.
	buffer.data =(bit8 *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, maxPulseCount * sizeof(bit32));
	p = (bit32 *) buffer.data;
	if (buffer.data == NULL)
	{
		return SetError(E_OUTOFMEMORY,TEXT("Out of memory."));
	}

	ZeroMemory(fdiTrackDescription, sizeof(FDITrackDescription) * HOST_MAX_TRACKS);
	nextTrackWrite = 0x200;
	for (tr=0; tr < HOST_MAX_TRACKS; tr++)
	{
		r = SetFilePointer (hFile, nextTrackWrite + sizeof(FDIRawTrackHeader), 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
		}

		PP64PulseStream track = &this->m_P64Image.PulseStreams[P64FirstHalfTrack + tr];
		compressedBufferSize = 0;
		currentPulseCount = 0;
		if (track->UsedFirst >= 0 && track->UsedLast >= 0)
		{			
			destIndex = 0;
			currentPosition = 0;
			previousPosition = track->Pulses[track->UsedLast].Position;
			currentIndex = track->UsedFirst;
			if (previousPosition < P64PulseSamplesPerRotation)
			{
				for (;currentIndex >= 0 && destIndex < pulseCountArray[tr]; destIndex++, previousPosition = currentPosition, currentIndex = track->Pulses[currentIndex].Next)
				{
					currentPosition = track->Pulses[currentIndex].Position;
					if ((currentPosition <= previousPosition && destIndex > 0) || currentPosition >= P64PulseSamplesPerRotation)
					{
						break;
					}
					if (currentPosition > previousPosition)
					{
						p[destIndex] = currentPosition - previousPosition;
					}
					else
					{
						p[destIndex] = P64PulseSamplesPerRotation + currentPosition - previousPosition;
					}
					currentPulseCount++;
				}
			}
			if (currentPulseCount > 0)
			{
				hr = hw.Compress(p, currentPulseCount, &compressedBufferSize);
				if (FAILED(hr))
				{
					if (hr == E_OUTOFMEMORY)
					{
						SetError(hr, TEXT("Out of memory."));
					}
					return SetError(hr, TEXT("Could not compress track %d."), (long)tr);
				}

				if (compressedBufferSize > 0x3ffff)
				{
					return SetError(E_FAIL,TEXT("Compression failed for track %d."),(int)tr);
				}
			}
		}
		fdiRawTrackHeader.numPulses = 0;
		fdiRawTrackHeader.aveSize[2] = 0;
		fdiRawTrackHeader.aveSize[1] = 0;
		fdiRawTrackHeader.aveSize[0] = 0;

		fdiRawTrackHeader.minSize[2] = 0;
		fdiRawTrackHeader.minSize[1] = 0;
		fdiRawTrackHeader.minSize[0] = 0;
		
		fdiRawTrackHeader.maxSize[2] = 0;
		fdiRawTrackHeader.maxSize[1] = 0;
		fdiRawTrackHeader.maxSize[0] = 0;

		fdiRawTrackHeader.idxSize[2] = 0;
		fdiRawTrackHeader.idxSize[1] = 0;
		fdiRawTrackHeader.idxSize[0] = 0;

		const int MAXALLOWEDCOMPRESSEDDATA = 0x3fff00;
		assert(compressedBufferSize <= MAXALLOWEDCOMPRESSEDDATA);
		if (compressedBufferSize > 0 && compressedBufferSize <= MAXALLOWEDCOMPRESSEDDATA && currentPulseCount > 0)
		{
			fdiRawTrackHeader.numPulses = dwordswap(currentPulseCount);
			fdiRawTrackHeader.aveSize[2] = (bit8)(compressedBufferSize);
			fdiRawTrackHeader.aveSize[1] = (bit8)(compressedBufferSize >> 8);
			fdiRawTrackHeader.aveSize[0] = (bit8)(compressedBufferSize >> 16);
			fdiRawTrackHeader.aveSize[0] = (fdiRawTrackHeader.aveSize[0] & 0x3f) | 0x40;
		}

		writeSize = (sizeof(FDIRawTrackHeader) + compressedBufferSize);
		if ((writeSize & 0xff) != 0)
		{
			j = 0;
			zeroPadSize = 0x100 - (writeSize & 0xff);
			for (i=0; i < zeroPadSize; i++)
			{
				rb = WriteFile(hFile, &j, 1, &bytes_written, NULL);
				if (!rb)
				{
					return SetError(E_FAIL, TEXT("Could not save %s."),filename);
				}
			}
		}

		r = SetFilePointer (hFile, nextTrackWrite, 0L, FILE_BEGIN);
		if (r == INVALID_SET_FILE_POINTER)
		{
			return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
		}
		rb = WriteFile(hFile, &fdiRawTrackHeader, sizeof(FDIRawTrackHeader), &bytes_written, NULL);
		if (!rb)
		{
			return SetError(E_FAIL, TEXT("Could not save %s."),filename);
		}

		writeSize = (writeSize + 0xff) >> 8;
		assert(writeSize <= MAXALLOWEDCOMPRESSEDDATA / 0x100);
		fdiTrackDescription[tr].type = ((bit8)(writeSize>>8) & 0x3f) | 0x80;
		fdiTrackDescription[tr].size = (bit8)writeSize & 0xff;
		nextTrackWrite+= (writeSize << 8);
	}

	r = SetFilePointer (hFile, SOFFSET(FDIHeader,trackDescription) , 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	rb = WriteFile(hFile, &fdiTrackDescription[0], sizeof(FDITrackDescription) * HOST_MAX_TRACKS, &bytes_written, NULL);
	if (!rb)
	{
		return SetError(E_FAIL, TEXT("Could not save %s."),filename);
	}
	/*Calculate CRC-32 values*/
	/*Seek to begining*/
	r = SetFilePointer (hFile, 0L, 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	file_size = GetFileSize(hFile, 0);
	if (INVALID_FILE_SIZE == file_size)
	{
		return SetError(E_FAIL,TEXT("GetFileSize failed for %s."),filename);
	}

	/*Read header*/
	hr = ReadFromFile(hFile, filename, (char *)&fdiHeader, sizeof(struct FDIHeader), 0);
	if (FAILED(hr))
	{
		return hr;
	}

	/*Read track data and calculate it's CRC-32*/
	for (i=sizeof(struct FDIHeader) ; (DWORD)i < file_size ; i++)
	{
		hr = ReadFromFile(hFile, filename, (char *)&currentByte, 1, 0);
		if (FAILED(hr))
		{
			return hr;
		}
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
	r = SetFilePointer (hFile, 0L , 0L, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER)
	{
		return SetError(E_FAIL,TEXT("Could not seek in file %s."),filename);
	}
	/*rewrite the header*/
	rb = WriteFile(hFile, &fdiHeader, sizeof(FDIHeader), &bytes_written, NULL);
	if (!rb || bytes_written!=sizeof(FDIHeader))
	{
		return SetError(E_FAIL, TEXT("Could not save %s."),filename);
	}

	return S_OK;
}

//Not implemented
HRESULT GCRDISK::SaveG64ToFile(TCHAR *filename)
{
	return S_OK;
}

HRESULT GCRDISK::SaveP64ToFile(TCHAR *filename)
{
TP64MemoryStream P64MemoryStreamInstance;
HANDLE hFile = NULL;
TCHAR *errorbuffer = NULL;
int bufferSize = 1024;
HRESULT hr = S_OK;
DWORD bytesWritten;

	do
	{
		P64MemoryStreamCreate(&P64MemoryStreamInstance);
		if (P64ImageWriteToStream(&this->m_P64Image, &P64MemoryStreamInstance))
		{
			hFile = CreateFile(filename, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
			if (hFile == INVALID_HANDLE_VALUE)
			{
				errorbuffer = G::MallocFormattedString(TEXT("Could not open %s."), filename);
				hr = this->SetErrorFromGetLastError(::GetLastError(), errorbuffer);
				break;
			}
			if (!WriteFile(hFile, P64MemoryStreamInstance.Data, P64MemoryStreamInstance.Size, &bytesWritten, NULL))
			{
				errorbuffer = G::MallocFormattedString(TEXT("Could not write to %s."), filename);
				hr = this->SetErrorFromGetLastError(::GetLastError(), errorbuffer);
				break;
			}
		}
		else
		{
			hr = this->SetError(E_FAIL, TEXT("Could generate the P64 stream"));
		}
	} while(false);
	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
		hFile = NULL;
	}
	if (errorbuffer != NULL)
	{
		free(errorbuffer);
		errorbuffer = NULL;
	}
	return hr;
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

	bool memoryAllocFailed = false;
	for (tr=0 ; tr < HOST_MAX_TRACKS ; tr++)
	{
		hr = ConvertP64toGCR(tr);
		if (FAILED(hr))
		{
			memoryAllocFailed = true;			
		}
	}

	r = ConvertGCRToD64(tracks);
	if (FAILED(r))
	{
		hr = TRUE;
	}

	hfile=CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL ,NULL); 
	if (hfile==INVALID_HANDLE_VALUE)
	{
		return SetError(E_FAIL,TEXT("Could not save %s"), filename);
	}
	
	rb = WriteFile(hfile, m_pD64Binary, file_size, &bytes_written, NULL);
	CloseHandle(hfile);
	if (rb == FALSE)
	{
		return SetError(E_FAIL,TEXT("Could not write to %s"), filename);
	}

	if (bytes_written!=file_size)
	{
		return SetError(E_FAIL,TEXT("Could not write to %s"),filename);
	}

	if (memoryAllocFailed)
	{
		return SetError(E_OUTOFMEMORY, TEXT("Out of memory while saving %s"), filename);
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


bit8 GCRDISK::GetByte(unsigned int trackNumber, unsigned int bitIndex)
{
long i,j;
bit8 byte;
char v;
bit32 startBitPos;

	if (trackNumber >= HOST_MAX_TRACKS || trackSize[trackNumber] == 0)
	{
		return 0;
	}

	startBitPos = bitIndex % trackSize[trackNumber];
	byte = 0;
	i = startBitPos;
	int shift = i & 7;
	for (j=0 ; j < 8 ; j++)
	{
		byte <<= 1;
		v = (signed char)(((trackData[trackNumber][i / 8]) << shift) & 0xff);
		if (v < 0)
		{
			byte |= 1;
		}

		i++;
		if ((bit32)i >= trackSize[trackNumber])
		{
			i = 0;
			shift = 0;
		}
		else
		{
			shift++;
			if (shift >= 8)
			{
				shift = 0;
			}
		}
	}
	return byte;
}

bool GCRDISK::IsEventQuitSignalled()
{
	if (mhevtQuit)
	{
		return (WaitForSingleObject(mhevtQuit, 0) == WAIT_OBJECT_0);
	}
	else
	{
		return false;
	}
}

bool GCRDISK::IsQuit()
{
	return this->IsEventQuitSignalled();
}

void GCRDISK::Quit()
{
}
