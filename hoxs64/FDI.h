#ifndef __FDI_H__ 
#define __FDI_H__ 

struct FDITrackDescription
{
	bit8 type;
	bit8 size;
};

struct FDIHeader
{
	char signature[27];
	char creator[30];
	char cr[2];
	char comment[80];
	bit8 eof;
	bit16 version;
	bit16 ltrack;
	bit8 lhead;
	bit8 type;
	bit8 rotspeed;
	bit8 flags;
	bit8 tpi;
	bit8 headwidth;
	bit16 reserved;
	struct FDITrackDescription trackDescription[176];
	bit32 dataCRC;
	bit32 headerCRC;
};

struct FDIRawTrackHeader
{
	bit32 numPulses;
	bit8 aveSize[3];
	bit8 minSize[3];
	bit8 maxSize[3];
	bit8 idxSize[3];
};

# pragma pack ()

class FDIStreamsHeader
{
public:
	FDIStreamsHeader();
	~FDIStreamsHeader();
	bit32 numPulses;
	bit32 aveSize;
	bit32 aveCompression;
	bit32 minSize;
	bit32 minCompression;
	bit32 maxSize;
	bit32 maxCompression;
	bit32 idxSize;
	bit32 idxCompression;
	bit32 *aveData;
	bit32 *minData;
	bit32 *maxData;
	bit32 *idxData;
};

class FDIStream
{
public:
	FDIStream();
	~FDIStream();
	bit8 lowBitNumber;
	bit8 highBitNumber;
	bit8 *data;
	bool bSignExtend;
	bit8 bitSize;
};

class FDIData
{
public:
	FDIData();
	~FDIData();
	bit8 *data;
};


#endif